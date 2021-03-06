// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#include "Pomdog/Experimental/Graphics/SpriteBatch.hpp"
#include "../../Utility/AlignedNew.hpp"
#include "Pomdog/Content/AssetBuilders/PipelineStateBuilder.hpp"
#include "Pomdog/Content/AssetBuilders/ShaderBuilder.hpp"
#include "Pomdog/Content/AssetManager.hpp"
#include "Pomdog/Experimental/Graphics/Texture2DView.hpp"
#include "Pomdog/Experimental/TexturePacker/TextureRegion.hpp"
#include "Pomdog/Graphics/BlendDescription.hpp"
#include "Pomdog/Graphics/BufferUsage.hpp"
#include "Pomdog/Graphics/ConstantBuffer.hpp"
#include "Pomdog/Graphics/DepthStencilDescription.hpp"
#include "Pomdog/Graphics/GraphicsCommandList.hpp"
#include "Pomdog/Graphics/GraphicsDevice.hpp"
#include "Pomdog/Graphics/IndexBuffer.hpp"
#include "Pomdog/Graphics/IndexElementSize.hpp"
#include "Pomdog/Graphics/InputLayoutHelper.hpp"
#include "Pomdog/Graphics/PipelineState.hpp"
#include "Pomdog/Graphics/PresentationParameters.hpp"
#include "Pomdog/Graphics/PrimitiveTopology.hpp"
#include "Pomdog/Graphics/RasterizerDescription.hpp"
#include "Pomdog/Graphics/RenderTarget2D.hpp"
#include "Pomdog/Graphics/SamplerDescription.hpp"
#include "Pomdog/Graphics/SamplerState.hpp"
#include "Pomdog/Graphics/Shader.hpp"
#include "Pomdog/Graphics/ShaderLanguage.hpp"
#include "Pomdog/Graphics/SurfaceFormat.hpp"
#include "Pomdog/Graphics/Texture2D.hpp"
#include "Pomdog/Graphics/VertexBuffer.hpp"
#include "Pomdog/Graphics/Viewport.hpp"
#include "Pomdog/Math/Color.hpp"
#include "Pomdog/Math/Matrix4x4.hpp"
#include "Pomdog/Math/Radian.hpp"
#include "Pomdog/Math/Rectangle.hpp"
#include "Pomdog/Math/Vector2.hpp"
#include "Pomdog/Math/Vector3.hpp"
#include "Pomdog/Math/Vector4.hpp"
#include <algorithm>
#include <cstring>
#include <tuple>
#include <vector>

using Pomdog::Detail::AlignedNew;

namespace Pomdog {
namespace {

// Built-in shaders
#include "Shaders/GLSL.Embedded/SpriteBatchDistanceField_PS.inc.hpp"
#include "Shaders/GLSL.Embedded/SpriteBatch_PS.inc.hpp"
#include "Shaders/GLSL.Embedded/SpriteBatch_VS.inc.hpp"
#include "Shaders/HLSL.Embedded/SpriteBatchDistanceField_PS.inc.hpp"
#include "Shaders/HLSL.Embedded/SpriteBatch_PS.inc.hpp"
#include "Shaders/HLSL.Embedded/SpriteBatch_VS.inc.hpp"
#include "Shaders/Metal.Embedded/SpriteBatch.inc.hpp"

Vector2 ComputeSpriteOffset(const TextureRegion& region, const Vector2& originPivot) noexcept
{
    if ((region.Subrect.Width <= 0) || (region.Subrect.Height <= 0)) {
        return Vector2::Zero;
    }

    POMDOG_ASSERT(region.Subrect.Width > 0);
    POMDOG_ASSERT(region.Subrect.Height > 0);

    const auto regionSize = Vector2{
        static_cast<float>(region.Width),
        static_cast<float>(region.Height)};

    const auto baseOffset = regionSize * originPivot;

    const auto w = static_cast<float>(region.Subrect.Width);
    const auto h = static_cast<float>(region.Subrect.Height);

    auto offset = Vector2{
        static_cast<float>(region.XOffset),
        regionSize.Y - (static_cast<float>(region.YOffset) + h)};
    offset = (baseOffset - offset) / Vector2{w, h};
    return offset;
}

struct alignas(16) SpriteBatchConstantBuffer final {
    Matrix4x4 ViewProjection;

    // {x___} = Smoothing
    // {_y__} = Weight
    // {__zw} = unused
    Vector4 DistanceFieldParameters;
};

} // unnamed namespace

// MARK: - SpriteBatch::Impl

class SpriteBatch::Impl final {
private:
    static constexpr std::size_t MaxBatchSize = 2048;
    static constexpr std::size_t MinBatchSize = 128;
    static constexpr std::size_t MaxDrawCallCount = 16;

    static_assert(MaxBatchSize >= MinBatchSize, "");

    struct alignas(16) SpriteInfo final : public AlignedNew<SpriteInfo> {
        // {xy__} = position.xy
        // {__zw} = scale.xy
        Vector4 Translation;

        // {xy__} = xy
        // {__zw} = {width, height}
        Vector4 SourceRect;

        // {xy__} = originPivot.xy
        // {__z_} = rotation
        // {___w} = layerDepth
        Vector4 OriginRotationLayerDepth;

        // {rgb_} = color.rgb
        // {___a} = color.a
        Vector4 Color;

        // {xy__} = {1.0f / textureWidth, 1.0f / textureHeight}
        // {__z_} = RGBA channel flags (8-bits)
        // {___w} = unused
        Vector4 InverseTextureSize;
    };

private:
    std::vector<SpriteInfo> spriteQueue;

    std::shared_ptr<GraphicsCommandList> commandList;
    Texture2DView currentTexture;

    std::shared_ptr<VertexBuffer> planeVertices;
    std::shared_ptr<IndexBuffer> planeIndices;
    std::shared_ptr<VertexBuffer> instanceVertices;

    std::shared_ptr<PipelineState> pipelineState;
    std::shared_ptr<ConstantBuffer> constantBuffer;
    std::shared_ptr<SamplerState> sampler;

    Vector2 inverseTextureSize;
    std::size_t startInstanceLocation;

public:
    int drawCallCount;

public:
    Impl(
        const std::shared_ptr<GraphicsDevice>& graphicsDevice,
        std::optional<BlendDescription>&& blendState,
        std::optional<RasterizerDescription>&& rasterizerDesc,
        std::optional<SamplerDescription>&& samplerState,
        std::optional<SurfaceFormat>&& renderTargetViewFormat,
        std::optional<DepthFormat>&& depthStencilViewFormat,
        SpriteBatchPixelShaderMode pixelShaderMode,
        AssetManager& assets);

    void Begin(
        const std::shared_ptr<GraphicsCommandList>& commandListIn,
        const Matrix4x4& transformMatrix,
        std::optional<SpriteBatchDistanceFieldParameters>&& distanceFieldParameters);

    void Draw(
        const Texture2DView& texture,
        const Vector2& position,
        const Rectangle& sourceRect,
        const Color& color,
        const Radian<float>& rotation,
        const Vector2& originPivot,
        const Vector2& scale,
        float layerDepth);

    void FlushBatch();

    void End();

private:
    void RenderBatch(
        const Texture2DView& texture,
        const std::vector<SpriteInfo>& sprites);

    void CompareTexture(const Texture2DView& texture);
};

SpriteBatch::Impl::Impl(
    const std::shared_ptr<GraphicsDevice>& graphicsDevice,
    std::optional<BlendDescription>&& blendDesc,
    std::optional<RasterizerDescription>&& rasterizerDesc,
    std::optional<SamplerDescription>&& samplerDesc,
    std::optional<SurfaceFormat>&& renderTargetViewFormat,
    std::optional<DepthFormat>&& depthStencilViewFormat,
    SpriteBatchPixelShaderMode pixelShaderMode,
    AssetManager& assets)
    : startInstanceLocation(0)
    , drawCallCount(0)
{
    auto presentationParameters = graphicsDevice->GetPresentationParameters();

    if (!blendDesc) {
        blendDesc = BlendDescription::CreateNonPremultiplied();
    }
    if (!rasterizerDesc) {
        rasterizerDesc = RasterizerDescription::CreateCullNone();
    }
    if (!samplerDesc) {
        samplerDesc = SamplerDescription::CreateLinearWrap();
    }
    if (!renderTargetViewFormat) {
        renderTargetViewFormat = presentationParameters.BackBufferFormat;
    }
    if (!depthStencilViewFormat) {
        depthStencilViewFormat = presentationParameters.DepthStencilFormat;
    }

    POMDOG_ASSERT(blendDesc);
    POMDOG_ASSERT(rasterizerDesc);
    POMDOG_ASSERT(samplerDesc);
    POMDOG_ASSERT(renderTargetViewFormat);
    POMDOG_ASSERT(depthStencilViewFormat);

    {
        using PositionTextureCoord = Vector4;

        std::array<PositionTextureCoord, 4> const verticesCombo = {{
            Vector4{0.0f, 0.0f, 0.0f, 1.0f},
            Vector4{0.0f, 1.0f, 0.0f, 0.0f},
            Vector4{1.0f, 1.0f, 1.0f, 0.0f},
            Vector4{1.0f, 0.0f, 1.0f, 1.0f},
        }};
        planeVertices = std::make_shared<VertexBuffer>(
            graphicsDevice,
            verticesCombo.data(),
            verticesCombo.size(),
            sizeof(PositionTextureCoord),
            BufferUsage::Immutable);
    }
    {
        std::array<std::uint16_t, 6> const indices = {{
            0, 1, 2,
            2, 3, 0
        }};

        // Create index buffer
        planeIndices = std::make_shared<IndexBuffer>(
            graphicsDevice,
            IndexElementSize::SixteenBits,
            indices.data(),
            indices.size(),
            BufferUsage::Immutable);
    }
    {
        const auto maxBatchSize = MaxBatchSize;
        instanceVertices = std::make_shared<VertexBuffer>(
            graphicsDevice,
            maxBatchSize,
            sizeof(SpriteInfo),
            BufferUsage::Dynamic);
    }
    {
        constantBuffer = std::make_shared<ConstantBuffer>(
            graphicsDevice,
            sizeof(SpriteBatchConstantBuffer),
            BufferUsage::Dynamic);

        sampler = std::make_shared<SamplerState>(
            graphicsDevice,
            *samplerDesc);
    }
    {
        auto inputLayout = InputLayoutHelper{}
            .AddInputSlot()
            .Float4()
            .AddInputSlot(InputClassification::InputPerInstance, 1)
            .Float4().Float4().Float4().Float4().Float4();

        auto vertexShader = assets.CreateBuilder<Shader>(ShaderPipelineStage::VertexShader)
            .SetGLSL(Builtin_GLSL_SpriteBatch_VS, std::strlen(Builtin_GLSL_SpriteBatch_VS))
            .SetHLSLPrecompiled(BuiltinHLSL_SpriteBatch_VS, sizeof(BuiltinHLSL_SpriteBatch_VS))
            .SetMetal(Builtin_Metal_SpriteBatch, sizeof(Builtin_Metal_SpriteBatch), "SpriteBatchVS");

        auto pixelShader = assets.CreateBuilder<Shader>(ShaderPipelineStage::PixelShader);

        switch (pixelShaderMode) {
        case SpriteBatchPixelShaderMode::Default:
            pixelShader.SetGLSL(Builtin_GLSL_SpriteBatch_PS, std::strlen(Builtin_GLSL_SpriteBatch_PS));
            pixelShader.SetHLSLPrecompiled(BuiltinHLSL_SpriteBatch_PS, sizeof(BuiltinHLSL_SpriteBatch_PS));
            pixelShader.SetMetal(Builtin_Metal_SpriteBatch, sizeof(Builtin_Metal_SpriteBatch), "SpriteBatchPS");
            break;
        case SpriteBatchPixelShaderMode::DistanceField:
            pixelShader.SetGLSL(Builtin_GLSL_SpriteBatchDistanceField_PS, std::strlen(Builtin_GLSL_SpriteBatchDistanceField_PS));
            pixelShader.SetHLSLPrecompiled(BuiltinHLSL_SpriteBatchDistanceField_PS, sizeof(BuiltinHLSL_SpriteBatchDistanceField_PS));
            pixelShader.SetMetal(Builtin_Metal_SpriteBatch, sizeof(Builtin_Metal_SpriteBatch), "SpriteBatchDistanceFieldPS");
            break;
        }

        pipelineState = assets.CreateBuilder<PipelineState>()
            .SetRenderTargetViewFormat(*renderTargetViewFormat)
            .SetDepthStencilViewFormat(*depthStencilViewFormat)
            .SetVertexShader(vertexShader.Build())
            .SetPixelShader(pixelShader.Build())
            .SetInputLayout(inputLayout.CreateInputLayout())
            .SetPrimitiveTopology(PrimitiveTopology::TriangleList)
            .SetBlendState(*blendDesc)
            .SetDepthStencilState(DepthStencilDescription::CreateNone())
            .SetRasterizerState(*rasterizerDesc)
            .SetConstantBufferBindSlot("SpriteBatchConstants", 0)
            .Build();
    }

    spriteQueue.reserve(MinBatchSize);
}

void SpriteBatch::Impl::Begin(
    const std::shared_ptr<GraphicsCommandList>& commandListIn,
    const Matrix4x4& transformMatrix,
    std::optional<SpriteBatchDistanceFieldParameters>&& distanceFieldParameters)
{
    POMDOG_ASSERT(commandListIn);
    this->commandList = commandListIn;

    POMDOG_ASSERT(constantBuffer);

    SpriteBatchConstantBuffer constants;
    constants.ViewProjection = Matrix4x4::Transpose(transformMatrix);

    if (distanceFieldParameters != std::nullopt) {
        constants.DistanceFieldParameters.X = distanceFieldParameters->Smoothing;
        constants.DistanceFieldParameters.Y = distanceFieldParameters->Weight;
    }
    else {
        constants.DistanceFieldParameters.X = 0.25f;
        constants.DistanceFieldParameters.Y = 0.65f;
    }

    constantBuffer->SetValue(constants);

    startInstanceLocation = 0;
    drawCallCount = 0;
}

void SpriteBatch::Impl::End()
{
    FlushBatch();

    if (drawCallCount > 0) {
        commandList->SetTexture(0);
    }
    commandList.reset();
}

void SpriteBatch::Impl::FlushBatch()
{
    if (spriteQueue.empty()) {
        return;
    }

    POMDOG_ASSERT(currentTexture != nullptr);
    POMDOG_ASSERT(!spriteQueue.empty());
    POMDOG_ASSERT((startInstanceLocation + spriteQueue.size()) <= MaxBatchSize);

    RenderBatch(currentTexture, spriteQueue);

    currentTexture = nullptr;
    spriteQueue.clear();
}

void SpriteBatch::Impl::RenderBatch(
    const Texture2DView& texture,
    const std::vector<SpriteInfo>& sprites)
{
    POMDOG_ASSERT(commandList);
    POMDOG_ASSERT(texture);
    POMDOG_ASSERT(!sprites.empty());
    POMDOG_ASSERT((startInstanceLocation + sprites.size()) <= MaxBatchSize);

    POMDOG_ASSERT(drawCallCount >= 0);

    POMDOG_ASSERT(sprites.size() <= MaxBatchSize);
    const auto instanceOffsetBytes = sizeof(SpriteInfo) * startInstanceLocation;
    instanceVertices->SetData(
        instanceOffsetBytes,
        sprites.data(),
        sprites.size(),
        sizeof(SpriteInfo));

    if (texture.GetIndex() == Texture2DViewIndex::Texture2D) {
        commandList->SetTexture(0, texture.AsTexture2D());
    }
    else if (texture.GetIndex() == Texture2DViewIndex::RenderTarget2D) {
        commandList->SetTexture(0, texture.AsRenderTarget2D());
    }
    commandList->SetSamplerState(0, sampler);

    commandList->SetPipelineState(pipelineState);
    commandList->SetConstantBuffer(0, constantBuffer);
    commandList->SetVertexBuffer(0, planeVertices);
    commandList->SetVertexBuffer(1, instanceVertices);

    commandList->DrawIndexedInstanced(
        planeIndices,
        planeIndices->GetIndexCount(),
        sprites.size(),
        0,
        startInstanceLocation);

    startInstanceLocation += sprites.size();
    POMDOG_ASSERT(startInstanceLocation <= MaxBatchSize);

    ++drawCallCount;
}

void SpriteBatch::Impl::CompareTexture(const Texture2DView& texture)
{
    POMDOG_ASSERT(texture != nullptr);

    if (texture != currentTexture) {
        if (currentTexture != nullptr) {
            FlushBatch();
        }
        POMDOG_ASSERT(spriteQueue.empty());
        POMDOG_ASSERT(currentTexture == nullptr);

        currentTexture = texture;

        POMDOG_ASSERT(texture->GetWidth() > 0);
        POMDOG_ASSERT(texture->GetHeight() > 0);

        const float w = static_cast<float>(texture->GetWidth());
        const float h = static_cast<float>(texture->GetHeight());

        inverseTextureSize.X = (w > 0.0f) ? (1.0f / w) : 0.0f;
        inverseTextureSize.Y = (h > 0.0f) ? (1.0f / h) : 0.0f;
    }
}

void SpriteBatch::Impl::Draw(
    const Texture2DView& texture,
    const Vector2& position,
    const Rectangle& sourceRect,
    const Color& color,
    const Radian<float>& rotation,
    const Vector2& originPivot,
    const Vector2& scale,
    float layerDepth)
{
    POMDOG_ASSERT(texture);
    POMDOG_ASSERT(texture->GetWidth() > 0);
    POMDOG_ASSERT(texture->GetHeight() > 0);
    POMDOG_ASSERT(sourceRect.Width >= 0);
    POMDOG_ASSERT(sourceRect.Height >= 0);

    if (sourceRect.Width == 0 || sourceRect.Height == 0) {
        return;
    }

    if (scale.X == 0.0f || scale.Y == 0.0f) {
        return;
    }

    if ((startInstanceLocation + spriteQueue.size()) >= MaxBatchSize) {
        FlushBatch();
        POMDOG_ASSERT(spriteQueue.empty());

        // TODO: Not implemented
        // GrowSpriteQueue();
        return;
    }

    bool sourceRGBEnabled = true;
    bool sourceAlphaEnabled = true;
    bool compensationRGB = false;
    bool compensationAlpha = false;

    switch (texture->GetFormat()) {
    case SurfaceFormat::R8_UNorm:
    case SurfaceFormat::R8G8_UNorm:
    case SurfaceFormat::R16G16_Float:
    case SurfaceFormat::R11G11B10_Float:
    case SurfaceFormat::R32_Float:
        sourceAlphaEnabled = false;
        compensationAlpha = true;
        break;
    case SurfaceFormat::A8_UNorm:
        sourceRGBEnabled = false;
        compensationRGB = true;
        break;
    case SurfaceFormat::R8G8B8A8_UNorm:
    case SurfaceFormat::R10G10B10A2_UNorm:
    case SurfaceFormat::B8G8R8A8_UNorm:
    case SurfaceFormat::R16G16B16A16_Float:
    case SurfaceFormat::R32G32B32A32_Float:
    case SurfaceFormat::BlockComp1_UNorm:
    case SurfaceFormat::BlockComp2_UNorm:
    case SurfaceFormat::BlockComp3_UNorm:
        break;
    }

    const int colorModeFlags = (sourceRGBEnabled ? 1 : 0)
        | (sourceAlphaEnabled ? 2 : 0)
        | (compensationRGB ? 4 : 0)
        | (compensationAlpha ? 8 : 0);

    POMDOG_ASSERT((startInstanceLocation + spriteQueue.size()) < MaxBatchSize);
    POMDOG_ASSERT(sourceRect.Width > 0);
    POMDOG_ASSERT(sourceRect.Height > 0);

    CompareTexture(texture);

    SpriteInfo info;
    info.Translation = Vector4{
        position.X,
        position.Y,
        scale.X,
        scale.Y
    };
    info.SourceRect = Vector4{
        static_cast<float>(sourceRect.X),
        static_cast<float>(sourceRect.Y),
        static_cast<float>(sourceRect.Width),
        static_cast<float>(sourceRect.Height)
    };
    info.OriginRotationLayerDepth = Vector4{
        originPivot.X,
        originPivot.Y,
        rotation.value,
        layerDepth
    };
    info.Color = color.ToVector4();
    info.InverseTextureSize = Vector4{
        inverseTextureSize.X,
        inverseTextureSize.Y,
        static_cast<float>(colorModeFlags),
        0.0f,
    };

    spriteQueue.push_back(std::move(info));
    POMDOG_ASSERT((startInstanceLocation + spriteQueue.size()) <= MaxBatchSize);
}

// MARK: - SpriteBatch

SpriteBatch::SpriteBatch(
    const std::shared_ptr<GraphicsDevice>& graphicsDevice,
    AssetManager& assets)
    : SpriteBatch(
        graphicsDevice,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        std::nullopt,
        SpriteBatchPixelShaderMode::Default,
        assets)
{
}

SpriteBatch::SpriteBatch(
    const std::shared_ptr<GraphicsDevice>& graphicsDevice,
    std::optional<BlendDescription>&& blendDesc,
    std::optional<RasterizerDescription>&& rasterizerDesc,
    std::optional<SamplerDescription>&& samplerDesc,
    std::optional<SurfaceFormat>&& renderTargetViewFormat,
    std::optional<DepthFormat>&& depthStencilViewFormat,
    SpriteBatchPixelShaderMode pixelShaderMode,
    AssetManager& assets)
    : impl(std::make_unique<Impl>(
        graphicsDevice,
        std::move(blendDesc),
        std::move(rasterizerDesc),
        std::move(samplerDesc),
        std::move(renderTargetViewFormat),
        std::move(depthStencilViewFormat),
        pixelShaderMode,
        assets))
{
}

SpriteBatch::~SpriteBatch() = default;

void SpriteBatch::Begin(
    const std::shared_ptr<GraphicsCommandList>& commandList,
    const Matrix4x4& transformMatrixIn)
{
    POMDOG_ASSERT(impl);
    impl->Begin(commandList, transformMatrixIn, std::nullopt);
}

void SpriteBatch::Begin(
    const std::shared_ptr<GraphicsCommandList>& commandList,
    const Matrix4x4& transformMatrixIn,
    const SpriteBatchDistanceFieldParameters& distanceFieldParameters)
{
    POMDOG_ASSERT(impl);
    impl->Begin(commandList, transformMatrixIn, distanceFieldParameters);
}

void SpriteBatch::Flush()
{
    POMDOG_ASSERT(impl);
    impl->FlushBatch();
}

void SpriteBatch::End()
{
    POMDOG_ASSERT(impl);
    impl->End();
}

void SpriteBatch::Draw(
    const std::shared_ptr<Texture2D>& texture,
    const Rectangle& sourceRect,
    const Color& color)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, {0, 0}, sourceRect, color, 0, {0.5f, 0.5f}, {1.0f, 1.0f}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<Texture2D>& texture,
    const Vector2& position,
    const Color& color)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    const Rectangle sourceRect = {0, 0, texture->GetWidth(), texture->GetHeight()};
    impl->Draw(texture, position, sourceRect, color, 0, {0.5f, 0.5f}, {1.0f, 1.0f}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<Texture2D>& texture,
    const Vector2& position,
    const Rectangle& sourceRect,
    const Color& color)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, sourceRect, color, 0, {0.5f, 0.5f}, {1.0f, 1.0f}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<Texture2D>& texture,
    const Vector2& position,
    const Rectangle& sourceRect,
    const Color& color,
    const Radian<float>& rotation,
    const Vector2& originPivot,
    float scale)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, sourceRect, color, rotation, originPivot, {scale, scale}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<Texture2D>& texture,
    const Vector2& position,
    const Rectangle& sourceRect,
    const Color& color,
    const Radian<float>& rotation,
    const Vector2& originPivot,
    const Vector2& scale)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, sourceRect, color, rotation, originPivot, scale, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<Texture2D>& texture,
    const Vector2& position,
    const TextureRegion& textureRegion,
    const Color& color,
    const Radian<float>& rotation,
    const Vector2& originPivot,
    float scale)
{
    POMDOG_ASSERT(impl);
    auto offset = ComputeSpriteOffset(textureRegion, originPivot);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, textureRegion.Subrect, color, rotation, offset, {scale, scale}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<Texture2D>& texture,
    const Vector2& position,
    const TextureRegion& textureRegion,
    const Color& color,
    const Radian<float>& rotation,
    const Vector2& originPivot,
    const Vector2& scale)
{
    POMDOG_ASSERT(impl);
    auto offset = ComputeSpriteOffset(textureRegion, originPivot);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, textureRegion.Subrect, color, rotation, offset, scale, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<RenderTarget2D>& texture,
    const Rectangle& sourceRect,
    const Color& color)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, {0, 0}, sourceRect, color, 0, {0.5f, 0.5f}, {1.0f, 1.0f}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<RenderTarget2D>& texture,
    const Vector2& position,
    const Color& color)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    const Rectangle sourceRect = {0, 0, texture->GetWidth(), texture->GetHeight()};
    impl->Draw(texture, position, sourceRect, color, 0, {0.5f, 0.5f}, {1.0f, 1.0f}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<RenderTarget2D>& texture,
    const Vector2& position,
    const Rectangle& sourceRect,
    const Color& color)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, sourceRect, color, 0, {0.5f, 0.5f}, {1.0f, 1.0f}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<RenderTarget2D>& texture,
    const Vector2& position,
    const Rectangle& sourceRect,
    const Color& color,
    const Radian<float>& rotation,
    const Vector2& originPivot,
    float scale)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, sourceRect, color, rotation, originPivot, {scale, scale}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<RenderTarget2D>& texture,
    const Vector2& position,
    const Rectangle& sourceRect,
    const Color& color,
    const Radian<float>& rotation,
    const Vector2& originPivot,
    const Vector2& scale)
{
    POMDOG_ASSERT(impl);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, sourceRect, color, rotation, originPivot, scale, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<RenderTarget2D>& texture,
    const Vector2& position,
    const TextureRegion& textureRegion,
    const Color& color,
    const Radian<float>& rotation,
    const Vector2& originPivot,
    float scale)
{
    POMDOG_ASSERT(impl);
    auto offset = ComputeSpriteOffset(textureRegion, originPivot);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, textureRegion.Subrect, color, rotation, offset, {scale, scale}, layerDepth);
}

void SpriteBatch::Draw(
    const std::shared_ptr<RenderTarget2D>& texture,
    const Vector2& position,
    const TextureRegion& textureRegion,
    const Color& color,
    const Radian<float>& rotation,
    const Vector2& originPivot,
    const Vector2& scale)
{
    POMDOG_ASSERT(impl);
    auto offset = ComputeSpriteOffset(textureRegion, originPivot);
    constexpr float layerDepth = 0.0f;
    impl->Draw(texture, position, textureRegion.Subrect, color, rotation, offset, scale, layerDepth);
}

int SpriteBatch::GetDrawCallCount() const noexcept
{
    POMDOG_ASSERT(impl);
    return impl->drawCallCount;
}

} // namespace Pomdog
