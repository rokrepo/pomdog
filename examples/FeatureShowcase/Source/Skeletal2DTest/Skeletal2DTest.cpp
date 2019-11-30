#include "Skeletal2DTest.hpp"
#include <Pomdog/Experimental/Graphics/BasicEffect.hpp>
#include <Pomdog/Experimental/Skeletal2D/AnimationClip.hpp>
#include <Pomdog/Experimental/Skeletal2D/SkeletonHelper.hpp>
#include <Pomdog/Experimental/Spine/AnimationLoader.hpp>
#include <Pomdog/Experimental/Spine/SkeletonDescLoader.hpp>
#include <Pomdog/Experimental/Spine/SkeletonLoader.hpp>
#include <Pomdog/Experimental/Spine/SkinLoader.hpp>
#include <Pomdog/Experimental/TexturePacker/TextureAtlasLoader.hpp>
#include <random>

namespace FeatureShowcase {

Skeletal2DTest::Skeletal2DTest(const std::shared_ptr<GameHost>& gameHostIn)
    : gameHost(gameHostIn)
    , graphicsDevice(gameHostIn->GetGraphicsDevice())
    , commandQueue(gameHostIn->GetGraphicsCommandQueue())
{
}

void Skeletal2DTest::Initialize()
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

    auto texturePath = PathHelper::Join(assets->GetContentDirectory(), "Skeletal2D/MaidChan/skeleton.png");
    auto textureAtlasPath = PathHelper::Join(assets->GetContentDirectory(), "Skeletal2D/MaidChan/skeleton.atlas");
    auto skeletonJSONPath = PathHelper::Join(assets->GetContentDirectory(), "Skeletal2D/MaidChan/skeleton.json");

    // NOTE: Load texture file for skeletal animation model
    if (auto [res, err] = assets->Load<Texture2D>(texturePath); err != nullptr) {
        Log::Verbose("failed to load texture: " + err->ToString());
    }
    else {
        texture = std::move(res);
    }

    // NOTE: Load texture atlas file for skeletal animation model
    TexturePacker::TextureAtlas textureAtlas;
    if (auto [atlas, err] = TexturePacker::TextureAtlasLoader::Load(textureAtlasPath); err != nullptr) {
        Log::Verbose("failed to load texture atlas: " + err->ToString());
    }
    else {
        textureAtlas = std::move(atlas);
    }

    // NOTE: Load skeletal animation data
    if (auto [desc, descErr] = Spine::SkeletonDescLoader::Load(skeletonJSONPath); descErr != nullptr) {
        Log::Verbose("failed to load skeleton JSON file: " + descErr->ToString());
    }
    else {
        skeleton = std::make_shared<Skeletal2D::Skeleton>(Spine::CreateSkeleton(desc.Bones));

        // NOTE: Create bind pose
        skeletonPose = std::make_shared<Skeletal2D::SkeletonPose>(Skeletal2D::SkeletonPose::CreateBindPose(*skeleton));

        // NOTE: Create animation clip and animation state
        auto [animationClip, clipErr] = Spine::CreateAnimationClip(desc, textureAtlas, "Walk");
        if (clipErr != nullptr) {
            Log::Verbose("failed to create animation clip: " + clipErr->ToString());
        }
        animationState = std::make_shared<Skeletal2D::AnimationState>(animationClip, 1.0f, true);

        // NOTE: Initialize global pose
        globalPose = Skeletal2D::SkeletonHelper::ToGlobalPose(*skeleton, *skeletonPose);

        // NOTE: Create skin
        skin = Spine::CreateSkin(desc, textureAtlas, "default");

        // NOTE: Add new skeleton animation to animation system
        animationSystem.Add(animationState, skeleton, skeletonPose, skin);
    }

    {
        using VertexCombined = BasicEffect::VertexPositionTexture;

        // NOTE: Create vertex buffer
        std::array<VertexCombined, 4> verticesCombo = {{
            {Vector3{0.0f, 0.0f, 0.0f}, Vector2{0.0f, 1.0f}},
            {Vector3{0.0f, 1.0f, 0.0f}, Vector2{0.0f, 0.0f}},
            {Vector3{1.0f, 1.0f, 0.0f}, Vector2{1.0f, 0.0f}},
            {Vector3{1.0f, 0.0f, 0.0f}, Vector2{1.0f, 1.0f}},
        }};

        vertexBuffer = std::make_shared<VertexBuffer>(
            graphicsDevice,
            verticesCombo.data(),
            4 * skin->GetSlots().size(),
            sizeof(VertexCombined),
            BufferUsage::Dynamic);
    }
    {
        // NOTE: Create index buffer
        std::array<std::uint16_t, 6> quadIndices = {{0, 1, 2, 2, 3, 0}};

        std::vector<std::uint16_t> indices;
        indices.reserve(6 * skin->GetSlots().size());

        for (std::size_t k = 0; k < skin->GetSlots().size(); k++) {
            for (auto i : quadIndices) {
                indices.push_back(i + (k * 4));
            }
        }

        indexBuffer = std::make_shared<IndexBuffer>(
            graphicsDevice,
            IndexElementSize::SixteenBits,
            indices.data(),
            indices.size(),
            BufferUsage::Immutable);
    }
    {
        // NOTE: Create constant buffer
        modelConstantBuffer = std::make_shared<ConstantBuffer>(
            graphicsDevice,
            sizeof(BasicEffect::ModelConstantBuffer),
            BufferUsage::Dynamic);

        worldConstantBuffer = std::make_shared<ConstantBuffer>(
            graphicsDevice,
            sizeof(BasicEffect::WorldConstantBuffer),
            BufferUsage::Dynamic);
    }
    {
        auto presentationParameters = graphicsDevice->GetPresentationParameters();

        BasicEffect::BasicEffectDescription effectDesc;
        effectDesc.LightingEnabled = false;
        effectDesc.TextureEnabled = true;
        effectDesc.VertexColorEnabled = false;

        // NOTE: Create pipeline state
        pipelineState = BasicEffect::CreateBasicEffect(*assets, effectDesc)
            .SetRenderTargetViewFormat(presentationParameters.BackBufferFormat)
            .SetDepthStencilViewFormat(presentationParameters.DepthStencilFormat)
            .SetDepthStencilState(DepthStencilDescription::CreateDefault())
            .SetBlendState(BlendDescription::CreateNonPremultiplied())
            .SetRasterizerState(RasterizerDescription::CreateDefault())
            .Build();

        // NOTE: Create pipeline state for wireframe debug rendering
        auto rasterizerDesc = RasterizerDescription::CreateCullNone();
        rasterizerDesc.FillMode = FillMode::WireFrame;

        // NOTE: Create pipeline state
        pipelineStateWireframe = BasicEffect::CreateBasicEffect(*assets, effectDesc)
            .SetRenderTargetViewFormat(presentationParameters.BackBufferFormat)
            .SetDepthStencilViewFormat(presentationParameters.DepthStencilFormat)
            .SetDepthStencilState(DepthStencilDescription::CreateDefault())
            .SetBlendState(BlendDescription::CreateOpaque())
            .SetRasterizerState(rasterizerDesc)
            .Build();
    }
    {
        sampler = std::make_shared<SamplerState>(graphicsDevice, SamplerDescription::CreateLinearWrap());
    }
}

void Skeletal2DTest::Update()
{
    auto clock = gameHost->GetClock();
    animationSystem.Update(*clock);

    // NOTE: Global pose generation
    Skeletal2D::SkeletonHelper::ToGlobalPose(*skeleton, *skeletonPose, globalPose);

    auto presentationParameters = graphicsDevice->GetPresentationParameters();

    auto projectionMatrix = Matrix4x4::CreateOrthographicLH(
        static_cast<float>(presentationParameters.BackBufferWidth),
        static_cast<float>(presentationParameters.BackBufferHeight),
        -1.0f,
        1000.0f);

    auto viewMatrix = Matrix4x4::Identity;

    // NOTE: Update constant buffer for world
    BasicEffect::WorldConstantBuffer worldConstants;
    worldConstants.ViewProjection = viewMatrix * projectionMatrix;
    worldConstants.InverseView = Matrix4x4::Invert(viewMatrix);
    worldConstants.LightDirection = Vector4{Vector3::UnitZ, 0.0f};
    worldConstantBuffer->SetValue(worldConstants);

    constexpr float metalness = 0.1f;

    // NOTE: Update constant buffer for model
    BasicEffect::ModelConstantBuffer modelConstants;
    modelConstants.Model = Matrix4x4::CreateTranslation(Vector3{0.0f, -180.0f, 0.0f});
    modelConstants.Material = Vector4{metalness, 0.0f, 0.0f, 0.0f};
    modelConstants.Color = Vector4{1.0f, 1.0f, 1.0f, 1.0f};
    modelConstantBuffer->SetValue(modelConstants);

    std::vector<BasicEffect::VertexPositionTexture> vertices;
    for (const auto& slot : skin->GetSlots()) {
        const auto slotCount = static_cast<int>(skin->GetSlots().size());
        auto layerDepth = static_cast<float>(slotCount - slot.DrawOrder) / slotCount;
        auto poseMatrix = globalPose[*slot.JointIndex];

        using Vertex = BasicEffect::VertexPositionTexture;

        std::array<Vertex, 4> quadVertices;
        if (slot.TextureRotate) {
            quadVertices = {{
                {Vector3{1.0f, 0.0f, 0.0f}, Vector2{1.0f, 1.0f}},
                {Vector3{0.0f, 0.0f, 0.0f}, Vector2{0.0f, 1.0f}},
                {Vector3{0.0f, 1.0f, 0.0f}, Vector2{0.0f, 0.0f}},
                {Vector3{1.0f, 1.0f, 0.0f}, Vector2{1.0f, 0.0f}},
            }};
        }
        else {
            quadVertices = {{
                {Vector3{0.0f, 0.0f, 0.0f}, Vector2{0.0f, 1.0f}},
                {Vector3{0.0f, 1.0f, 0.0f}, Vector2{0.0f, 0.0f}},
                {Vector3{1.0f, 1.0f, 0.0f}, Vector2{1.0f, 0.0f}},
                {Vector3{1.0f, 0.0f, 0.0f}, Vector2{1.0f, 1.0f}},
            }};
        }

        auto inverseTextureSize = Vector2{1.0f / static_cast<float>(texture->GetWidth()), 1.0f / static_cast<float>(texture->GetHeight())};
        auto subrectPos = Vector2{static_cast<float>(slot.Subrect.X), static_cast<float>(slot.Subrect.Y)};
        auto subrectSize = Vector2{static_cast<float>(slot.Subrect.Width), static_cast<float>(slot.Subrect.Height)};
        auto scaling = Matrix3x2::CreateScale(subrectSize);

        auto translate = Matrix3x2::CreateTranslation(slot.Translate);
        auto rotate = Matrix3x2::CreateRotation(slot.Rotation);

        if (slot.TextureRotate) {
            rotate = rotate * Matrix3x2::CreateRotation(-Math::PiOver2<float>);
        }

        auto transformMatrix = scaling * rotate * translate * poseMatrix;

        for (const auto& v : quadVertices) {
            auto pos = Vector2{v.Position.X, v.Position.Y};
            pos = Vector2::Transform(pos - slot.Origin, transformMatrix);

            auto vertex = v;
            vertex.Position = Vector3{pos, layerDepth};
            vertex.TextureCoordinate = ((v.TextureCoordinate * subrectSize) + subrectPos) * inverseTextureSize;
            vertices.push_back(vertex);
        }
    }

    vertexBuffer->SetData(vertices.data(), vertices.size());
}

void Skeletal2DTest::Draw()
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

    commandList->SetConstantBuffer(0, modelConstantBuffer);
    commandList->SetConstantBuffer(1, worldConstantBuffer);
    commandList->SetSamplerState(0, sampler);
    commandList->SetTexture(0, texture);
    commandList->SetVertexBuffer(0, vertexBuffer);
    commandList->SetPipelineState(pipelineState);
    commandList->SetPrimitiveTopology(PrimitiveTopology::TriangleList);
    commandList->DrawIndexed(indexBuffer, indexBuffer->GetIndexCount(), 0);

    auto mouse = gameHost->GetMouse()->GetState();
    if (mouse.RightButton == ButtonState::Pressed) {
        commandList->SetPipelineState(pipelineStateWireframe);
        commandList->DrawIndexed(indexBuffer, indexBuffer->GetIndexCount(), 0);
    }

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
