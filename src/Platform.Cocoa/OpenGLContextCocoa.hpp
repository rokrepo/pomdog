// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include "../RenderSystem.GL4/OpenGLContext.hpp"
#include "Pomdog/Graphics/detail/ForwardDeclarations.hpp"
#import <Cocoa/Cocoa.h>

@class NSOpenGLContext, NSOpenGLPixelFormat;

namespace Pomdog::Detail::Cocoa {

struct CocoaOpenGLHelper {
    static NSOpenGLPixelFormat* CreatePixelFormat(
        const PresentationParameters& presentationParameters);
};

class OpenGLContextCocoa final : public GL4::OpenGLContext {
public:
    OpenGLContextCocoa() = delete;
    OpenGLContextCocoa(const OpenGLContextCocoa&) = delete;
    OpenGLContextCocoa& operator=(const OpenGLContextCocoa&) = delete;

    explicit OpenGLContextCocoa(NSOpenGLPixelFormat* pixelFormat);

    ~OpenGLContextCocoa();

    void MakeCurrent() override;

    void ClearCurrent() override;

    void SwapBuffers() override;

    void Lock();

    void Unlock();

    void SetView(NSView* view);

    void SetView();

    NSOpenGLContext* GetNativeOpenGLContext();

private:
    __strong NSOpenGLContext* openGLContext;
};

} // namespace Pomdog::Detail::Cocoa
