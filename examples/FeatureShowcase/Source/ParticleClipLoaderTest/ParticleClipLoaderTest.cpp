#include "ParticleClipLoaderTest.hpp"
#include <Pomdog/Experimental/Particles/ParticleClipLoader.hpp>
#include <random>

namespace FeatureShowcase {

ParticleClipLoaderTest::ParticleClipLoaderTest(const std::shared_ptr<GameHost>& gameHostIn)
    : gameHost(gameHostIn)
    , graphicsDevice(gameHostIn->GetGraphicsDevice())
    , commandQueue(gameHostIn->GetGraphicsCommandQueue())
{
}

void ParticleClipLoaderTest::Initialize()
{
    auto assets = gameHost->GetAssetManager();
    auto clock = gameHost->GetClock();
    commandList = std::make_shared<GraphicsCommandList>(*graphicsDevice);
    primitiveBatch = std::make_shared<PrimitiveBatch>(graphicsDevice, *assets);
    spriteBatch = std::make_shared<SpriteBatch>(
        graphicsDevice,
        BlendDescription::CreateAlphaBlend(),
        std::nullopt,
        SamplerDescription::CreatePointWrap(),
        std::nullopt,
        std::nullopt,
        SpriteBatchPixelShaderMode::Default,
        *assets);

    std::shared_ptr<Error> err;
    std::tie(texture, err) = assets->Load<Texture2D>("Textures/particle_smoke.png");
    if (err != nullptr) {
        Log::Verbose("failed to load texture: " + err->ToString());
    }

    timer = std::make_shared<Timer>(clock);
    timer->SetInterval(std::chrono::seconds(1));
    timer->SetScale(0.2);

    {
        // NOTE: Load particle clip from .json file
        auto [particleClip, clipErr] = assets->Load<ParticleClip>("Particles/Fire2D.json");
        if (clipErr != nullptr) {
            Log::Verbose("failed to load particle json: " + clipErr->ToString());
        }

        particleSystem = std::make_unique<ParticleSystem>(particleClip);
        particleSystem->Play();
    }

    emitterPosition = Vector2::Zero;

    auto mouse = gameHost->GetMouse();
    auto onMoved = [this](const Point2D& mousePos) {
        auto mouse = gameHost->GetMouse();
        auto mouseState = mouse->GetState();
        if (mouseState.LeftButton != ButtonState::Pressed) {
            return;
        }
        auto window = gameHost->GetWindow();
        auto pos = mousePos;
        pos.X = pos.X - (window->GetClientBounds().Width / 2);
        pos.Y = -pos.Y + (window->GetClientBounds().Height / 2);
        emitterPosition = MathHelper::ToVector2(pos);
    };
    auto onClipChanged = [this] {
        std::array<std::string, 2> filenames = {
            "Particles/Fire2D.json",
            "Particles/Water2D.json",
        };

        currentClipIndex++;
        if (currentClipIndex >= filenames.size()) {
            currentClipIndex = 0;
        }

        // NOTE: Load particle clip from .json file
        auto assets = gameHost->GetAssetManager();
        auto [particleClip, clipErr] = assets->Load<ParticleClip>(filenames[currentClipIndex]);
        if (clipErr != nullptr) {
            Log::Verbose("failed to load particle json: " + clipErr->ToString());
        }
        else {
            particleSystem = std::make_unique<ParticleSystem>(particleClip);
        }
    };
    connect(mouse->ButtonDown, [this, onMoved, onClipChanged]([[maybe_unused]] MouseButtons mouseButton) {
        auto mouse = gameHost->GetMouse();
        auto mouseState = mouse->GetState();
        if (mouseState.LeftButton == ButtonState::Pressed) {
            onMoved(mouseState.Position);
        }
        if (mouseState.RightButton == ButtonState::Pressed) {
            onClipChanged();
        }
    });
    connect(mouse->Moved, onMoved);
}

void ParticleClipLoaderTest::Update()
{
    auto clock = gameHost->GetClock();
    auto frameDuration = clock->GetFrameDuration();
    particleSystem->Simulate(emitterPosition, MathHelper::ToRadians(90.0f), frameDuration);
}

void ParticleClipLoaderTest::Draw()
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
        static_cast<float>(presentationParameters.BackBufferWidth),
        static_cast<float>(presentationParameters.BackBufferHeight),
        0.0f,
        100.0f);

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

    for (const auto& particle : particleSystem->GetParticles()) {
        spriteBatch->Draw(
            texture,
            Vector2{particle.Position.X, particle.Position.Y},
            Rectangle{0, 0, 64, 64},
            particle.Color,
            particle.Rotation,
            Vector2{0.5f, 0.5f},
            particle.Size);
    }

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
