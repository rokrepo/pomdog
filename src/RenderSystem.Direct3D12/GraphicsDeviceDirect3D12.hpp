// Copyright (c) 2013-2016 mogemimi. Distributed under the MIT license.

#pragma once

#include "../RenderSystem/NativeGraphicsDevice.hpp"
#include "Pomdog/Platform/Win32/PrerequisitesWin32.hpp"
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include <vector>

namespace Pomdog {
namespace Detail {
namespace Direct3D12 {

class GraphicsDeviceDirect3D12 final : public NativeGraphicsDevice {
public:
    GraphicsDeviceDirect3D12();

    ~GraphicsDeviceDirect3D12();

    ShaderLanguage GetSupportedLanguage() const override;

    std::unique_ptr<Shader>
    CreateShader(ShaderBytecode const& shaderBytecode,
        ShaderCompileOptions const& compileOptions) override;

    std::unique_ptr<NativeBuffer>
    CreateBuffer(std::size_t sizeInBytes,
        BufferUsage bufferUsage, BufferBindMode bindMode) override;

    std::unique_ptr<NativeBuffer>
    CreateBuffer(void const* sourceData, std::size_t sizeInBytes,
        BufferUsage bufferUsage, BufferBindMode bindMode) override;

    std::unique_ptr<NativeSamplerState>
    CreateSamplerState(SamplerDescription const& description) override;

    std::unique_ptr<NativePipelineState>
    CreatePipelineState(PipelineStateDescription const& description) override;

    std::unique_ptr<NativeEffectReflection>
    CreateEffectReflection(PipelineStateDescription const& description,
        NativePipelineState & pipelineState) override;

    std::unique_ptr<NativeTexture2D>
    CreateTexture2D(std::int32_t width, std::int32_t height,
        std::int32_t mipmapLevels, SurfaceFormat format) override;

    std::unique_ptr<NativeRenderTarget2D>
    CreateRenderTarget2D(std::int32_t width, std::int32_t height,
        std::int32_t mipmapLevels,
        SurfaceFormat format,
        DepthFormat depthStencilFormat,
        std::int32_t multiSampleCount) override;

    Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() const;

private:
    std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter1>> adapters;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> activeAdapter;
    Microsoft::WRL::ComPtr<ID3D12Device> device;
};

} // namespace Direct3D12
} // namespace Detail
} // namespace Pomdog
