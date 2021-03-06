// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include "Pomdog/Basic/Export.hpp"
#include "Pomdog/Graphics/DepthFormat.hpp"
#include "Pomdog/Graphics/SurfaceFormat.hpp"
#include <functional>
#include <memory>

#import <AppKit/NSWindow.h>

@class PomdogOpenGLView;
@class PomdogMetalViewController;

namespace Pomdog {
class Game;
class GameHost;
} // namespace Pomdog

namespace Pomdog::Detail::Cocoa {
class GameHostCocoa;
class GameHostMetal;
} // namespace Pomdog::Detail::Cocoa

namespace Pomdog::Cocoa {

class POMDOG_EXPORT Bootstrap final {
public:
    /// Sets the window where the video is drawn.
    void SetWindow(NSWindow* window);

    /// Sets to true if OpenGL is enabled, false if Metal is enabled. false by default.
    void SetOpenGLEnabled(bool enabled);

    /// Sets the format of the back buffer when using the OpenGL renderer.
    void SetOpenGLSurfaceFormat(SurfaceFormat surfaceFormat);

    /// Sets the format of the depth stencil buffer when using the OpenGL renderer.
    void SetOpenGLDepthFormat(DepthFormat depthFormat);

    /// Sets an error event handler to a log stream.
    void OnError(std::function<void(const std::exception&)>&& onError);

    /// Sets an completion event handler to a log stream.
    void OnCompleted(std::function<void()>&& onCompleted);

    /// Begins running a game loop.
    void Run(std::function<std::shared_ptr<Game>(const std::shared_ptr<GameHost>&)>&& createGame);

private:
    std::function<void()> onCompleted;
    std::function<void(const std::exception&)> onError;
    std::shared_ptr<Pomdog::Detail::Cocoa::GameHostCocoa> gameHostCocoa;
    std::shared_ptr<Pomdog::Detail::Cocoa::GameHostMetal> gameHostMetal;
    std::shared_ptr<Game> game;
    __weak NSWindow* nativeWindow = nil;
    PomdogMetalViewController* viewController = nil;
    SurfaceFormat surfaceFormat = SurfaceFormat::R8G8B8A8_UNorm;
    DepthFormat depthFormat = DepthFormat::Depth24Stencil8;
    bool openGLEnabled = false;
};

} // namespace Pomdog::Cocoa
