// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include "OpenGLPrerequisites.hpp"
#include "Texture2DGL4.hpp"
#include "../RenderSystem/NativeRenderTarget2D.hpp"
#include "../Utility/Tagged.hpp"
#include "Pomdog/Graphics/detail/ForwardDeclarations.hpp"
#include <optional>

namespace Pomdog::Detail::GL4 {

using RenderBuffer2DGL4 = Tagged<GLuint, RenderTarget2D>;

class RenderTarget2DGL4 final : public NativeRenderTarget2D {
public:
    RenderTarget2DGL4(
        std::int32_t pixelWidth,
        std::int32_t pixelHeight,
        std::int32_t levelCount,
        SurfaceFormat format,
        DepthFormat depthStencilFormat,
        std::int32_t multiSampleCount);

    ~RenderTarget2DGL4();

    void GetData(
        void* result,
        std::size_t offsetInBytes,
        std::size_t sizeInBytes,
        std::int32_t pixelWidth,
        std::int32_t pixelHeight,
        std::int32_t levelCount,
        SurfaceFormat format) const override;

    void BindToFramebuffer(GLenum attachmentPoint);
    void UnbindFromFramebuffer(GLenum attachmentPoint);

    void BindDepthStencilBuffer();

    const Texture2DObjectGL4& GetTextureHandle() const;

private:
    Texture2DGL4 texture;
    std::optional<RenderBuffer2DGL4> renderBuffer;
    bool generateMipmap;
    bool multiSampleEnabled;
};

} // namespace Pomdog::Detail::GL4
