#include "SpriteBatchTest.hpp"
#include <Pomdog/Experimental/Tween/EasingHelper.hpp>
#include <random>

namespace FeatureShowcase {

SpriteBatchTest::SpriteBatchTest(const std::shared_ptr<GameHost>& gameHostIn)
    : gameHost(gameHostIn)
    , graphicsDevice(gameHostIn->GetGraphicsDevice())
    , commandQueue(gameHostIn->GetGraphicsCommandQueue())
{
}

void SpriteBatchTest::Initialize()
{
    auto assets = gameHost->GetAssetManager();
    auto clock = gameHost->GetClock();
    commandList = std::make_shared<GraphicsCommandList>(*graphicsDevice);
    primitiveBatch = std::make_shared<PrimitiveBatch>(graphicsDevice, *assets);
    spriteBatch = std::make_shared<SpriteBatch>(
        graphicsDevice,
        BlendDescription::CreateNonPremultiplied(),
        std::nullopt,
        SamplerDescription::CreatePointWrap(),
        std::nullopt,
        std::nullopt,
        SpriteBatchPixelShaderMode::Default,
        *assets);

    if (auto [res, err] = assets->Load<Texture2D>("Textures/pomdog.png"); err != nullptr) {
        Log::Verbose("failed to load texture: " + err->ToString());
    }
    else {
        texture = std::move(res);
    }

    timer = std::make_shared<Timer>(clock);
    timer->SetInterval(std::chrono::seconds(1));
    timer->SetScale(0.2);

    auto mouse = gameHost->GetMouse();
    connect(mouse->ButtonDown, [this](MouseButtons mouseButton) {
        if (mouseButton != MouseButtons::Left) {
            return;
        }

        auto window = gameHost->GetWindow();
        auto mouse = gameHost->GetMouse();
        auto mouseState = mouse->GetState();
        auto pos = mouseState.Position;
        pos.X = pos.X - (window->GetClientBounds().Width / 2);
        pos.Y = -pos.Y + (window->GetClientBounds().Height / 2);

        std::mt19937 random(std::random_device{}());
        std::uniform_real_distribution<float> scaleDist(1.0f, 2.0f);
        std::uniform_int_distribution<uint8_t> colorDist(160, 255);

        const auto scale = scaleDist(random);

        SpriteInstance sprite;
        sprite.Position = MathHelper::ToVector2(pos);
        sprite.Scale.X = scale;
        sprite.Scale.Y = scale;
        sprite.Color.R = colorDist(random);
        sprite.Color.G = colorDist(random);
        sprite.Color.B = colorDist(random);
        sprite.Color.A = 255;
        sprites.push_back(std::move(sprite));
    });
}

void SpriteBatchTest::Update()
{
}

void SpriteBatchTest::Draw()
{
    auto presentationParameters = graphicsDevice->GetPresentationParameters();

    Viewport viewport = {0, 0, presentationParameters.BackBufferWidth, presentationParameters.BackBufferHeight};
    RenderPass pass;
    pass.RenderTargets[0] = {nullptr, Color::CornflowerBlue.ToVector4()};
    pass.ClearDepth = 1.0f;
    pass.ClearStencil = 0;
    pass.Viewport = viewport;
    pass.ScissorRect = viewport.GetBounds();

    commandList->Reset();
    commandList->SetRenderPass(std::move(pass));

    auto projectionMatrix = Matrix4x4::CreateOrthographicLH(
        presentationParameters.BackBufferWidth,
        presentationParameters.BackBufferHeight,
        0.0f,
        100.0f);

    const auto t = static_cast<float>(timer->GetTotalTime().count());

    // Drawing line
    const auto w = static_cast<float>(presentationParameters.BackBufferWidth);
    const auto h = static_cast<float>(presentationParameters.BackBufferHeight);
    primitiveBatch->Begin(commandList, projectionMatrix);
    primitiveBatch->DrawLine(Vector2{-w * 0.5f, 0.0f}, Vector2{w * 0.5f, 0.0f}, Color{221, 220, 218, 160}, 1.0f);
    primitiveBatch->DrawLine(Vector2{0.0f, -h * 0.5f}, Vector2{0.0f, h * 0.5f}, Color{221, 220, 218, 160}, 1.0f);
    primitiveBatch->DrawLine(Vector2{-w * 0.5f, h * 0.25f}, Vector2{w * 0.5f, h * 0.25f}, Color{221, 220, 218, 60}, 1.0f);
    primitiveBatch->DrawLine(Vector2{-w * 0.5f, -h * 0.25f}, Vector2{w * 0.5f, -h * 0.25f}, Color{221, 220, 218, 60}, 1.0f);
    primitiveBatch->DrawLine(Vector2{-w * 0.25f, -h * 0.5f}, Vector2{-w * 0.25f, h * 0.5f}, Color{221, 220, 218, 60}, 1.0f);
    primitiveBatch->DrawLine(Vector2{w * 0.25f, -h * 0.5f}, Vector2{w * 0.25f, h * 0.5f}, Color{221, 220, 218, 60}, 1.0f);
    primitiveBatch->End();

    spriteBatch->Begin(commandList, projectionMatrix);

    for (const auto& sprite : sprites) {
        spriteBatch->Draw(
            texture,
            sprite.Position,
            Rectangle{0, 0, texture->GetWidth(), texture->GetHeight()},
            sprite.Color,
            Math::TwoPi<float> * Easings::EaseSine::InOut(t),
            Vector2{0.5f, 0.5f},
            sprite.Scale);
    }

    spriteBatch->Draw(
        texture,
        Vector2{64.0f, 64.0f},
        Rectangle{0, 0, texture->GetWidth(), texture->GetHeight()},
        Color::White,
        Math::TwoPi<float> * Easings::EaseSine::InOut(t),
        Vector2{1.0f, 1.0f},
        2.0f);

    spriteBatch->End();

    commandList->Close();

    constexpr bool isStandalone = false;
    if constexpr (isStandalone) {
        commandQueue->Reset();
        commandQueue->PushbackCommandList(commandList);
        commandQueue->ExecuteCommandLists();
        commandQueue->Present();
    }
    else {
        commandQueue->PushbackCommandList(commandList);
    }
}

} // namespace FeatureShowcase
