#pragma once

#include <Pomdog/Experimental/Graphics/PolylineBatch.hpp>
#include <Pomdog/Pomdog.hpp>

namespace FeatureShowcase {

using namespace Pomdog;

class PolylineDrawingTest final : public Game {
public:
    explicit PolylineDrawingTest(const std::shared_ptr<GameHost>& gameHost);

    void Initialize() override;

    void Update() override;

    void Draw() override;

private:
    std::shared_ptr<GameHost> gameHost;
    std::shared_ptr<GraphicsDevice> graphicsDevice;
    std::shared_ptr<GraphicsCommandQueue> commandQueue;
    std::shared_ptr<GraphicsCommandList> commandList;
    ConnectionList connect;

    std::shared_ptr<PolylineBatch> lineBatch;
    std::vector<Vector2> path;

    float lineWidth = 4.0f;
    bool polylineClosed = false;
};

} // namespace FeatureShowcase
