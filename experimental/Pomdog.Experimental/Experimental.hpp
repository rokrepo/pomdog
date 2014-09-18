﻿//
//  Copyright (C) 2013-2015 mogemimi.
//  Distributed under the MIT License. See LICENSE.md or
//  http://enginetrouble.net/pomdog/license for details.
//

#ifndef POMDOG_EXPERIMENTAL_113A86BE_D2CB_4B4D_8927_74889ACD23F2_HPP
#define POMDOG_EXPERIMENTAL_113A86BE_D2CB_4B4D_8927_74889ACD23F2_HPP

#if (_MSC_VER > 1000)
#pragma once
#endif

#include "Actions/Action.hpp"
#include "Actions/MoveToAction.hpp"
#include "Actions/RemoveActorAction.hpp"
#include "Actions/ScaleToAction.hpp"
#include "Actions/SequenceAction.hpp"
#include "Actions/TintToAction.hpp"

#include "Compositing/RenderLayer.hpp"
#include "Compositing/RenderLayerCompositor.hpp"

#include "Gameplay2D/Animator.hpp"
#include "Gameplay2D/BeamRenderable.hpp"
#include "Gameplay2D/Behavior.hpp"
#include "Gameplay2D/Camera2D.hpp"
#include "Gameplay2D/GameLevel.hpp"
#include "Gameplay2D/ParticleRenderable.hpp"
#include "Gameplay2D/Renderable.hpp"
#include "Gameplay2D/RectangleRenderable.hpp"
#include "Gameplay2D/ScriptBehavior.hpp"
#include "Gameplay2D/SkinnedMeshRenderable.hpp"
#include "Gameplay2D/SpriteRenderable.hpp"
#include "Gameplay2D/Transform2D.hpp"

#include "Graphics/LineBatch.hpp"
#include "Graphics/PolygonBatch.hpp"
#include "Graphics/ScreenQuad.hpp"
#include "Graphics/SkinnedEffect.hpp"
#include "Graphics/SpriteBatch.hpp"
#include "Graphics/SpriteFont.hpp"
#include "Graphics/SpriteFontLoader.hpp"
#include "Graphics/SpriteLine.hpp"
#include "Graphics/SpriteSortMode.hpp"
#include "Graphics/SpriteRenderer.hpp"
#include "Graphics/TextureRegion.hpp"

#include "ImageEffects/ChromaticAberration.hpp"
#include "ImageEffects/FishEyeEffect.hpp"
#include "ImageEffects/FXAA.hpp"
#include "ImageEffects/GrayscaleEffect.hpp"
#include "ImageEffects/SepiaToneEffect.hpp"
#include "ImageEffects/VignetteEffect.hpp"

#include "InGameEditor/InGameEditor.hpp"

#include "MagicaVoxel/VoxModel.hpp"
#include "MagicaVoxel/VoxModelLoader.hpp"

#include "Particle2D/Beam.hpp"
#include "Particle2D/BeamEmitter.hpp"
#include "Particle2D/BeamSystem.hpp"
#include "Particle2D/Particle.hpp"
#include "Particle2D/ParticleClip.hpp"
#include "Particle2D/ParticleEmitter.hpp"
#include "Particle2D/ParticleLoader.hpp"
#include "Particle2D/ParticleSystem.hpp"

#include "Skeletal2D/AnimationBlendInput.hpp"
#include "Skeletal2D/AnimationBlendInputType.hpp"
#include "Skeletal2D/AnimationClip.hpp"
#include "Skeletal2D/AnimationGraph.hpp"
#include "Skeletal2D/AnimationNode.hpp"
#include "Skeletal2D/AnimationState.hpp"
#include "Skeletal2D/AnimationSystem.hpp"
#include "Skeletal2D/AnimationTimeInterval.hpp"
#include "Skeletal2D/AnimationTrack.hpp"
#include "Skeletal2D/CompressedFloat.hpp"
#include "Skeletal2D/Joint.hpp"
#include "Skeletal2D/JointIndex.hpp"
#include "Skeletal2D/JointPose.hpp"
#include "Skeletal2D/RigidSlot.hpp"
#include "Skeletal2D/RotationTrack.hpp"
#include "Skeletal2D/ScaleTrack.hpp"
#include "Skeletal2D/Skeleton.hpp"
#include "Skeletal2D/SkeletonHelper.hpp"
#include "Skeletal2D/SkeletonPose.hpp"
#include "Skeletal2D/SkeletonTransform.hpp"
#include "Skeletal2D/Skin.hpp"
#include "Skeletal2D/SkinnedMesh.hpp"
#include "Skeletal2D/SkinnedMeshPart.hpp"
#include "Skeletal2D/SkinnedVertex.hpp"
#include "Skeletal2D/SpriteAnimationTrack.hpp"
#include "Skeletal2D/TranslationTrack.hpp"

#include "Spine/AnimationGraphBuilder.hpp"
#include "Spine/AnimationLoader.hpp"
#include "Spine/SkeletonDesc.hpp"
#include "Spine/SkeletonDescLoader.hpp"
#include "Spine/SkeletonLoader.hpp"
#include "Spine/SkinLoader.hpp"
#include "Spine/SkinnedMeshLoader.hpp"
#include "Spine/SpriteAnimationLoader.hpp"

#include "TexturePacker/TextureAtlas.hpp"
#include "TexturePacker/TextureAtlasLoader.hpp"

#include "Tween/EasingHelper.hpp"

#include "Rendering/RenderCommand.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/RenderQueue.hpp"

#include "UI/Control.hpp"
#include "UI/DrawingContext.hpp"
#include "UI/DebugNavigator.hpp"
#include "UI/HorizontalAlignment.hpp"
#include "UI/Panel.hpp"
#include "UI/PointerEventType.hpp"
#include "UI/PointerPoint.hpp"
#include "UI/ScenePanel.hpp"
#include "UI/Slider.hpp"
#include "UI/StackPanel.hpp"
#include "UI/TextBlock.hpp"
#include "UI/Thickness.hpp"
#include "UI/ToggleSwitch.hpp"
#include "UI/UIElement.hpp"
#include "UI/UIElementHierarchy.hpp"
#include "UI/UIEventDispatcher.hpp"
#include "UI/UIEventListener.hpp"
#include "UI/UIHelper.hpp"
#include "UI/UIView.hpp"
#include "UI/VerticalAlignment.hpp"

#endif // !defined(POMDOG_EXPERIMENTAL_113A86BE_D2CB_4B4D_8927_74889ACD23F2_HPP)
