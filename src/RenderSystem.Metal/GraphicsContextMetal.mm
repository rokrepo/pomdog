// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#include "GraphicsContextMetal.hpp"
#include "../Basic/Unreachable.hpp"
#include "../RenderSystem.Metal/BufferMetal.hpp"
#include "../RenderSystem.Metal/ConstantsMetal.hpp"
#include "../RenderSystem.Metal/PipelineStateMetal.hpp"
#include "../RenderSystem.Metal/RenderTarget2DMetal.hpp"
#include "../RenderSystem.Metal/SamplerStateMetal.hpp"
#include "../RenderSystem.Metal/Texture2DMetal.hpp"
#include "../RenderSystem/GraphicsCapabilities.hpp"
#include "../RenderSystem/GraphicsCommandListImmediate.hpp"
#include "Pomdog/Graphics/IndexBuffer.hpp"
#include "Pomdog/Graphics/PresentationParameters.hpp"
#include "Pomdog/Graphics/RenderPass.hpp"
#include "Pomdog/Graphics/RenderTarget2D.hpp"
#include "Pomdog/Graphics/Texture2D.hpp"
#include "Pomdog/Graphics/VertexBuffer.hpp"
#include "Pomdog/Graphics/Viewport.hpp"
#include "Pomdog/Math/Rectangle.hpp"
#include "Pomdog/Math/Vector4.hpp"
#include "Pomdog/Utility/Assert.hpp"
#include "Pomdog/Utility/Exception.hpp"

namespace Pomdog::Detail::Metal {
namespace {

MTLIndexType ToIndexType(IndexElementSize elementSize) noexcept
{
    switch (elementSize) {
    case IndexElementSize::SixteenBits: return MTLIndexTypeUInt16;
    case IndexElementSize::ThirtyTwoBits: return MTLIndexTypeUInt32;
    }
    POMDOG_UNREACHABLE("Unsupported index element size");
}

std::size_t ToIndexByteSize(MTLIndexType elementSize) noexcept
{
    switch (elementSize) {
    case MTLIndexTypeUInt16: return 2;
    case MTLIndexTypeUInt32: return 4;
    }
    POMDOG_UNREACHABLE("Unsupported index element size");
}

MTLClearColor ToClearColor(const Vector4& color) noexcept
{
    return MTLClearColorMake(color.X, color.Y, color.Z, color.W);
}

void SetViewport(
    id<MTLRenderCommandEncoder> commandEncoder, const Viewport& viewportIn)
{
    POMDOG_ASSERT(viewportIn.Width > 0);
    POMDOG_ASSERT(viewportIn.Height > 0);
    POMDOG_ASSERT(commandEncoder != nil);

    // NOTE: The MinDepth and MaxDepth must be between 0.0 and 1.0, respectively.
    // Please see https://developer.apple.com/documentation/metal/mtlrendercommandencoder/1515527-setviewport
    POMDOG_ASSERT((0.0f <= viewportIn.MinDepth) && (viewportIn.MinDepth <= 1.0f));
    POMDOG_ASSERT((0.0f <= viewportIn.MaxDepth) && (viewportIn.MaxDepth <= 1.0f));

    MTLViewport viewport;
    viewport.originX = viewportIn.TopLeftX;
    viewport.originY = viewportIn.TopLeftY;
    viewport.width = viewportIn.Width;
    viewport.height = viewportIn.Height;
    viewport.znear = viewportIn.MinDepth;
    viewport.zfar = viewportIn.MaxDepth;
    [commandEncoder setViewport:viewport];
}

void SetScissorRectangle(
    id<MTLRenderCommandEncoder> commandEncoder, const Rectangle& rectangle)
{
    POMDOG_ASSERT(rectangle.Width >= 0);
    POMDOG_ASSERT(rectangle.Height >= 0);
    POMDOG_ASSERT(commandEncoder != nil);

    MTLScissorRect rect;
    rect.x = rectangle.X;
    rect.y = rectangle.Y;
    rect.width = rectangle.Width;
    rect.height = rectangle.Height;
    [commandEncoder setScissorRect:rect];
}

#if defined(DEBUG) && !defined(NDEBUG)
void CheckUnbindingRenderTargetsError(
    const std::vector<std::weak_ptr<RenderTarget2D>>& renderTargets,
    const std::vector<std::weak_ptr<Texture>>& textures)
{
    for (auto& renderTarget: renderTargets) {
        for (auto& texture: textures) {
            if (!renderTarget.expired() && !texture.expired()) {
                POMDOG_ASSERT(renderTarget.lock() != texture.lock());
            }
        }
    }
}
#endif

} // unnamed namespace

GraphicsContextMetal::GraphicsContextMetal(
    id<MTLDevice> nativeDevice)
    : commandQueue(nil)
    , commandBuffer(nil)
    , commandEncoder(nil)
    , indexBuffer(nil)
{
    POMDOG_ASSERT(nativeDevice != nil);

    // The max number of command buffers in flight
    constexpr NSUInteger kMaxInflightBuffers = 1;

    // Create semaphore for Metal
    inflightSemaphore = dispatch_semaphore_create(kMaxInflightBuffers);

    // NOTE: Create a new command queue
    commandQueue = [nativeDevice newCommandQueue];

#if defined(DEBUG) && !defined(NDEBUG)
    auto graphicsCapbilities = this->GetCapabilities();

    POMDOG_ASSERT(graphicsCapbilities.SamplerSlotCount > 0);
    weakTextures.resize(graphicsCapbilities.SamplerSlotCount);
#endif
}

GraphicsContextMetal::~GraphicsContextMetal() = default;

GraphicsCapabilities GraphicsContextMetal::GetCapabilities() const
{
    // NOTE: For more information, please see:
    // https://developer.apple.com/library/ios/documentation/Miscellaneous/Conceptual/MetalProgrammingGuide/MetalFeatureSetTables/MetalFeatureSetTables.html
    GraphicsCapabilities caps;
    caps.ConstantBufferSlotCount = 31;
    caps.SamplerSlotCount = 16;
    return caps;
}

void GraphicsContextMetal::DispatchSemaphoreWait()
{
    if (!isDrawing) {
        // NOTE: Skip waiting
        return;
    }

    dispatch_semaphore_wait(inflightSemaphore, DISPATCH_TIME_FOREVER);
    isDrawing = false;
}

void GraphicsContextMetal::SetMTKView(MTKView* view)
{
    POMDOG_ASSERT(view != nil);
    targetView = view;
}

void GraphicsContextMetal::ExecuteCommandLists(
    const std::vector<std::shared_ptr<GraphicsCommandListImmediate>>& commandLists)
{
    POMDOG_ASSERT(commandQueue != nil);
    POMDOG_ASSERT(targetView != nil);
    POMDOG_ASSERT(commandBuffer == nil);
    POMDOG_ASSERT(commandEncoder == nil);

    // Create a new command buffer for each renderpass to the current drawable
    commandBuffer = [commandQueue commandBuffer];
    commandBuffer.label = @"PomdogCommand";

    // Call the view's completion handler which is required by the view
    // since it will signal its semaphore and set up the next buffer
    __block dispatch_semaphore_t blockSema = inflightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(blockSema);
    }];
    isDrawing = true;

    POMDOG_ASSERT(commandBuffer != nil);

    // NOTE: Skip rendering when the graphics device is lost.
    const bool skipRender = (targetView.currentDrawable.texture.pixelFormat == MTLPixelFormatInvalid);

    if (!skipRender) {
        for (auto& commandList : commandLists) {
            POMDOG_ASSERT(commandList);
            commandList->ExecuteImmediate(*this);
        }
    }

    if (commandEncoder != nil) {
        // We're done encoding commands
        [commandEncoder popDebugGroup];
        [commandEncoder endEncoding];
        commandEncoder = nil;
    }

    POMDOG_ASSERT(commandBuffer != nil);

    // Schedule a present once the framebuffer is complete using the current drawable
    [commandBuffer presentDrawable:targetView.currentDrawable];

    // Finalize rendering here & push the command buffer to the GPU
    [commandBuffer commit];
    commandBuffer = nil;
}

void GraphicsContextMetal::Present()
{
    // NOTE: commandBuffer::commit() has already been called at GraphicsContextMetal::ExecuteCommandLists().
}

void GraphicsContextMetal::Draw(
    std::size_t vertexCount,
    std::size_t startVertexLocation)
{
    POMDOG_ASSERT(commandEncoder != nil);
    POMDOG_ASSERT(vertexCount > 0);

#if defined(DEBUG) && !defined(NDEBUG)
    CheckUnbindingRenderTargetsError(weakRenderTargets, weakTextures);
#endif

    [commandEncoder drawPrimitives:primitiveType
        vertexStart:startVertexLocation
        vertexCount:vertexCount];
}

void GraphicsContextMetal::DrawIndexed(
    std::size_t indexCount,
    std::size_t startIndexLocation)
{
    POMDOG_ASSERT(commandEncoder != nil);
    POMDOG_ASSERT(indexCount > 0);

#if defined(DEBUG) && !defined(NDEBUG)
    CheckUnbindingRenderTargetsError(weakRenderTargets, weakTextures);
#endif
    const auto indexBufferOffset = startIndexLocation * ToIndexByteSize(indexType);

    [commandEncoder drawIndexedPrimitives:primitiveType
        indexCount:indexCount
        indexType:indexType
        indexBuffer:indexBuffer
        indexBufferOffset:indexBufferOffset];
}

void GraphicsContextMetal::DrawInstanced(
    std::size_t vertexCountPerInstance,
    std::size_t instanceCount,
    std::size_t startVertexLocation,
    std::size_t startInstanceLocation)
{
    POMDOG_ASSERT(commandEncoder != nil);
    POMDOG_ASSERT(vertexCountPerInstance > 0);
    POMDOG_ASSERT(instanceCount > 0);

#if defined(DEBUG) && !defined(NDEBUG)
    CheckUnbindingRenderTargetsError(weakRenderTargets, weakTextures);
#endif

    [commandEncoder drawPrimitives:primitiveType
        vertexStart:startVertexLocation
        vertexCount:vertexCountPerInstance
        instanceCount:instanceCount
        baseInstance:startInstanceLocation];
}

void GraphicsContextMetal::DrawIndexedInstanced(
    std::size_t indexCountPerInstance,
    std::size_t instanceCount,
    std::size_t startIndexLocation,
    std::size_t startInstanceLocation)
{
    POMDOG_ASSERT(commandEncoder != nil);
    POMDOG_ASSERT(indexCountPerInstance > 0);
    POMDOG_ASSERT(instanceCount > 0);

#if defined(DEBUG) && !defined(NDEBUG)
    CheckUnbindingRenderTargetsError(weakRenderTargets, weakTextures);
#endif
    const auto indexBufferOffset = startIndexLocation * ToIndexByteSize(indexType);

    [commandEncoder drawIndexedPrimitives:primitiveType
        indexCount:indexCountPerInstance
        indexType:indexType
        indexBuffer:indexBuffer
        indexBufferOffset:indexBufferOffset
        instanceCount:instanceCount
        baseVertex:0
        baseInstance:startInstanceLocation];
}

void GraphicsContextMetal::SetViewport(const Viewport& viewport)
{
    POMDOG_ASSERT(commandEncoder != nil);
    Metal::SetViewport(commandEncoder, viewport);
}

void GraphicsContextMetal::SetScissorRect(const Rectangle& scissorRect)
{
    POMDOG_ASSERT(commandEncoder != nil);
    SetScissorRectangle(commandEncoder, scissorRect);
}

void GraphicsContextMetal::SetBlendFactor(const Vector4& blendFactor)
{
    POMDOG_ASSERT(commandEncoder != nil);
    [commandEncoder setBlendColorRed:blendFactor.X green:blendFactor.Y blue:blendFactor.Z alpha:blendFactor.W];
}

void GraphicsContextMetal::SetVertexBuffer(
    int index,
    const std::shared_ptr<VertexBuffer>& vertexBuffer,
    std::size_t offset)
{
    POMDOG_ASSERT(index >= 0);
    POMDOG_ASSERT(vertexBuffer != nullptr);
    POMDOG_ASSERT(vertexBuffer->GetNativeVertexBuffer() != nullptr);
    POMDOG_ASSERT((offset % 256) == 0);

    auto nativeVertexBuffer = static_cast<BufferMetal*>(
        vertexBuffer->GetNativeVertexBuffer());

    POMDOG_ASSERT(nativeVertexBuffer != nullptr);
    POMDOG_ASSERT(nativeVertexBuffer == dynamic_cast<BufferMetal*>(vertexBuffer->GetNativeVertexBuffer()));
    POMDOG_ASSERT(nativeVertexBuffer->GetBuffer() != nil);

    const auto slotIndex = index + VertexBufferSlotOffset;
    POMDOG_ASSERT(slotIndex < MaxVertexBufferSlotCount);

    [commandEncoder setVertexBuffer:nativeVertexBuffer->GetBuffer()
        offset:offset
        atIndex:slotIndex];
}

void GraphicsContextMetal::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBufferIn)
{
    POMDOG_ASSERT(indexBufferIn != nullptr);

    auto nativeIndexBuffer = static_cast<BufferMetal*>(
        indexBufferIn->GetNativeIndexBuffer());

    POMDOG_ASSERT(nativeIndexBuffer != nullptr);
    POMDOG_ASSERT(nativeIndexBuffer == dynamic_cast<BufferMetal*>(
        indexBufferIn->GetNativeIndexBuffer()));

    this->indexType = ToIndexType(indexBufferIn->GetElementSize());
    this->indexBuffer = nativeIndexBuffer->GetBuffer();
}

void GraphicsContextMetal::SetPipelineState(const std::shared_ptr<NativePipelineState>& pipelineState)
{
    POMDOG_ASSERT(pipelineState != nullptr);

    auto nativePipelineState = static_cast<PipelineStateMetal*>(pipelineState.get());

    POMDOG_ASSERT(nativePipelineState != nullptr);
    POMDOG_ASSERT(nativePipelineState == dynamic_cast<PipelineStateMetal*>(pipelineState.get()));

    this->primitiveType = nativePipelineState->GetPrimitiveType();

    POMDOG_ASSERT(commandEncoder != nil);
    nativePipelineState->Apply(commandEncoder);
}

void GraphicsContextMetal::SetConstantBuffer(
    int index,
    const std::shared_ptr<NativeBuffer>& constantBufferIn,
    std::size_t offset,
    [[maybe_unused]] std::size_t sizeInBytes)
{
    POMDOG_ASSERT(index >= 0);
    POMDOG_ASSERT(constantBufferIn);
    POMDOG_ASSERT(offset >= 0);
    POMDOG_ASSERT(sizeInBytes > 0);

#if defined(DEBUG) && !defined(NDEBUG)
    static const auto capabilities = GetCapabilities();
    POMDOG_ASSERT(index < static_cast<int>(capabilities.ConstantBufferSlotCount));
#endif

    auto constantBuffer = std::static_pointer_cast<BufferMetal>(constantBufferIn);

    POMDOG_ASSERT(constantBuffer);
    POMDOG_ASSERT(constantBuffer == std::dynamic_pointer_cast<BufferMetal>(constantBufferIn));

    POMDOG_ASSERT(constantBuffer->GetBuffer() != nil);
    [commandEncoder setVertexBuffer:constantBuffer->GetBuffer()
        offset:offset
        atIndex:index];
    [commandEncoder setFragmentBuffer:constantBuffer->GetBuffer()
        offset:offset
        atIndex:index];
}

void GraphicsContextMetal::SetSampler(int index, const std::shared_ptr<NativeSamplerState>& sampler)
{
    POMDOG_ASSERT(sampler != nullptr);
    POMDOG_ASSERT(index >= 0);

    auto samplerStateMetal = std::static_pointer_cast<SamplerStateMetal>(sampler);

    POMDOG_ASSERT(samplerStateMetal != nullptr);
    POMDOG_ASSERT(samplerStateMetal == std::dynamic_pointer_cast<SamplerStateMetal>(sampler));
    POMDOG_ASSERT(samplerStateMetal->GetSamplerState() != nil);

    POMDOG_ASSERT(commandEncoder != nil);
    [commandEncoder setVertexSamplerState:samplerStateMetal->GetSamplerState() atIndex:index];
    [commandEncoder setFragmentSamplerState:samplerStateMetal->GetSamplerState() atIndex:index];
}

void GraphicsContextMetal::SetTexture(int index)
{
    POMDOG_ASSERT(index >= 0);
    POMDOG_ASSERT(commandEncoder != nil);

#if defined(DEBUG) && !defined(NDEBUG)
    POMDOG_ASSERT(!weakTextures.empty());
    POMDOG_ASSERT(index < static_cast<int>(weakTextures.size()));
    weakTextures[index].reset();
#endif

    [commandEncoder setVertexTexture:nil atIndex:index];
    [commandEncoder setFragmentTexture:nil atIndex:index];
}

void GraphicsContextMetal::SetTexture(int index, const std::shared_ptr<Texture2D>& textureIn)
{
    POMDOG_ASSERT(index >= 0);
    POMDOG_ASSERT(textureIn);

#if defined(DEBUG) && !defined(NDEBUG)
    POMDOG_ASSERT(!weakTextures.empty());
    POMDOG_ASSERT(index < static_cast<int>(weakTextures.size()));
    weakTextures[index] = textureIn;
#endif

    auto textureMetal = static_cast<Texture2DMetal*>(textureIn->GetNativeTexture2D());

    POMDOG_ASSERT(textureMetal != nullptr);
    POMDOG_ASSERT(textureMetal == dynamic_cast<Texture2DMetal*>(textureIn->GetNativeTexture2D()));
    POMDOG_ASSERT(textureMetal->GetTexture() != nil);

    POMDOG_ASSERT(commandEncoder != nil);
    [commandEncoder setVertexTexture:textureMetal->GetTexture() atIndex:index];
    [commandEncoder setFragmentTexture:textureMetal->GetTexture() atIndex:index];
}

void GraphicsContextMetal::SetTexture(int index, const std::shared_ptr<RenderTarget2D>& textureIn)
{
    POMDOG_ASSERT(index >= 0);
    POMDOG_ASSERT(textureIn);

#if defined(DEBUG) && !defined(NDEBUG)
    POMDOG_ASSERT(!weakTextures.empty());
    POMDOG_ASSERT(index < static_cast<int>(weakTextures.size()));
    weakTextures[index] = textureIn;
#endif

    auto renderTargetMetal = static_cast<RenderTarget2DMetal*>(textureIn->GetNativeRenderTarget2D());

    POMDOG_ASSERT(renderTargetMetal != nullptr);
    POMDOG_ASSERT(renderTargetMetal == dynamic_cast<RenderTarget2DMetal*>(textureIn->GetNativeRenderTarget2D()));
    POMDOG_ASSERT(renderTargetMetal->GetTexture() != nil);

    POMDOG_ASSERT(commandEncoder != nil);
    [commandEncoder setVertexTexture:renderTargetMetal->GetTexture() atIndex:index];
    [commandEncoder setFragmentTexture:renderTargetMetal->GetTexture() atIndex:index];
}

void GraphicsContextMetal::SetRenderPass(const RenderPass& renderPass)
{
    POMDOG_ASSERT(!renderPass.RenderTargets.empty());
    POMDOG_ASSERT(renderPass.RenderTargets.size() <= 8);

#if defined(DEBUG) && !defined(NDEBUG)
    weakRenderTargets.clear();
    for (auto& renderTarget : renderPass.RenderTargets) {
        weakRenderTargets.push_back(std::get<0>(renderTarget));
    }
#endif

    MTLRenderPassDescriptor* renderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
    POMDOG_ASSERT(renderPassDescriptor != nil);

    const bool useBackBuffer = (std::get<0>(renderPass.RenderTargets.front()) == nullptr);

    const auto setClearColor = [&](int index, const std::optional<Vector4>& clearColor) {
        if (clearColor) {
            renderPassDescriptor.colorAttachments[index].loadAction = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[index].clearColor = ToClearColor(*clearColor);
        }
        else {
            renderPassDescriptor.colorAttachments[index].loadAction = MTLLoadActionDontCare;
        }
        renderPassDescriptor.colorAttachments[index].storeAction = MTLStoreActionStore;
    };

    if (useBackBuffer) {
        auto& renderTargetView = renderPass.RenderTargets.front();
        auto& clearColor = std::get<1>(renderTargetView);
        POMDOG_ASSERT(std::get<0>(renderTargetView) == nullptr);

        constexpr int renderTargetIndex = 0;
        renderPassDescriptor.colorAttachments[renderTargetIndex].texture = targetView.currentDrawable.texture;
        setClearColor(renderTargetIndex, clearColor);
    }
    else {
        int renderTargetIndex = 0;
        for (const auto& renderTargetView: renderPass.RenderTargets) {
            auto& renderTarget = std::get<0>(renderTargetView);
            auto& clearColor = std::get<1>(renderTargetView);

            if (renderTarget == nullptr) {
                break;
            }
            auto nativeRenderTarget = static_cast<RenderTarget2DMetal*>(renderTarget->GetNativeRenderTarget2D());
            POMDOG_ASSERT(nativeRenderTarget == dynamic_cast<RenderTarget2DMetal*>(renderTarget->GetNativeRenderTarget2D()));
            POMDOG_ASSERT(nativeRenderTarget != nullptr);

            renderPassDescriptor.colorAttachments[renderTargetIndex].texture = nativeRenderTarget->GetTexture();

            setClearColor(renderTargetIndex, clearColor);
            ++renderTargetIndex;
        }
    }

    {
        auto renderTargetView = renderPass.RenderTargets[0];
        auto& renderTarget = std::get<0>(renderTargetView);

        if (!renderTarget) {
            renderPassDescriptor.depthAttachment.texture = targetView.currentRenderPassDescriptor.depthAttachment.texture;
            renderPassDescriptor.stencilAttachment.texture = targetView.currentRenderPassDescriptor.stencilAttachment.texture;
        }
        else {
            auto nativeRenderTarget = static_cast<RenderTarget2DMetal*>(renderTarget->GetNativeRenderTarget2D());
            POMDOG_ASSERT(nativeRenderTarget == dynamic_cast<RenderTarget2DMetal*>(renderTarget->GetNativeRenderTarget2D()));
            POMDOG_ASSERT(nativeRenderTarget != nullptr);

            renderPassDescriptor.depthAttachment.texture = nativeRenderTarget->GetDepthStencilTexture();

            bool isStencilRenderable = [&]() -> bool {
                switch ([nativeRenderTarget->GetDepthStencilTexture() pixelFormat]) {
                case MTLPixelFormatStencil8:
                case MTLPixelFormatX24_Stencil8:
                case MTLPixelFormatX32_Stencil8:
                case MTLPixelFormatDepth24Unorm_Stencil8:
                case MTLPixelFormatDepth32Float_Stencil8:
                    return true;
                default:
                    break;
                }
                return false;
            }();

            if (isStencilRenderable) {
                renderPassDescriptor.stencilAttachment.texture = nativeRenderTarget->GetDepthStencilTexture();
            }
            else {
                renderPassDescriptor.stencilAttachment.texture = nil;
            }
        }
    }

    if (renderPassDescriptor.depthAttachment.texture != nil) {
        renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
        renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
    }
    if (renderPassDescriptor.stencilAttachment.texture != nil) {
        renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionLoad;
        renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionStore;
    }

    if (renderPass.ClearDepth) {
        renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        renderPassDescriptor.depthAttachment.clearDepth = *renderPass.ClearDepth;
    }
    if (renderPass.ClearStencil) {
        renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionClear;
        renderPassDescriptor.stencilAttachment.clearStencil = *renderPass.ClearStencil;
    }

    POMDOG_ASSERT(commandBuffer != nil);

    if (commandEncoder != nil) {
        // We're done encoding commands
        [commandEncoder popDebugGroup];
        [commandEncoder endEncoding];
        commandEncoder = nil;
    }

    // Create a render command encoder so we can render into something
    commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    commandEncoder.label = @"PomdogRenderEncoder";
    [commandEncoder pushDebugGroup:@"PomdogDraw"];

    [commandEncoder setFrontFacingWinding:MTLWindingClockwise];

    if (renderPass.Viewport) {
        Metal::SetViewport(commandEncoder, *renderPass.Viewport);
    }
    if (renderPass.ScissorRect) {
        SetScissorRectangle(commandEncoder, *renderPass.ScissorRect);
    }
}

} // namespace Pomdog::Detail::Metal
