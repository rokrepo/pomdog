﻿//
//  Copyright (C) 2013-2014 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#include "AnimationGraphBuilder.hpp"
#include <fstream>
#include <vector>
#include <rapidjson/document.h>
#include "../Spine/AnimationLoader.hpp"
#include "../Skeletal2D/AnimationClip.hpp"
#include "../Skeletal2D/AnimationClipNode.hpp"
#include "../Skeletal2D/AnimationLerpNode.hpp"

namespace Pomdog {
namespace Details {
namespace Skeletal2D {
namespace {

static std::vector<char> ReadBinaryFile(std::string const& filename)
{
	std::ifstream stream(filename, std::ios::in | std::ios::binary);
	
	stream.seekg(0, stream.end);
	auto const length = stream.tellg();
	stream.seekg(0, stream.beg);
	
	std::vector<char> result(length, 0);
	stream.read(result.data(), result.size());
	return std::move(result);
}

enum class AnimationNodeType: std::uint8_t {
	Clip,
	Lerp,
};

struct AnimationNodeDesc {
	Optional<std::string> ClipName;
	Optional<std::string> Parameter;
	std::array<Optional<std::uint16_t>, 4> Inputs;
	Optional<float> DefaultParameter;
	std::uint16_t Id;
	AnimationNodeType Type;
};

static std::unique_ptr<AnimationNode> CreateAnimationNode(
	AnimationNodeDesc const& desc, std::vector<AnimationBlendInput> & inputs,
		std::vector<AnimationNodeDesc> const& nodes, Details::Skeletal2D::SkeletonDesc const& skeletonDesc)
{
	switch (desc.Type) {
	case AnimationNodeType::Clip: {
		POMDOG_ASSERT(desc.ClipName);
		auto animationClip = std::make_shared<AnimationClip>(Details::Skeletal2D::CreateAnimationClip(skeletonDesc, desc.ClipName->c_str()));
		return std::make_unique<AnimationClipNode>(animationClip);
		break;
	}
	case AnimationNodeType::Lerp: {
		POMDOG_ASSERT(desc.Inputs[0]);
		POMDOG_ASSERT(desc.Inputs[1]);
	
		auto iter1 = std::find_if(std::begin(nodes),std::end(nodes),
			[&](AnimationNodeDesc const& nodeDesc){ return nodeDesc.Id == *desc.Inputs[0]; });
		POMDOG_ASSERT(iter1 != std::end(nodes));
		
		auto iter2 = std::find_if(std::begin(nodes),std::end(nodes),
			[&](AnimationNodeDesc const& nodeDesc){ return nodeDesc.Id == *desc.Inputs[1]; });
		POMDOG_ASSERT(iter2 != std::end(nodes));
	
		auto node1 = CreateAnimationNode(*iter1, inputs, nodes, skeletonDesc);
		auto node2 = CreateAnimationNode(*iter2, inputs, nodes, skeletonDesc);
		
		if (desc.Parameter) {
			inputs.push_back({*desc.Parameter, AnimationBlendInputType::Float});
		}
		else {
			inputs.push_back({"__unknown", AnimationBlendInputType::Float});
		}
		std::uint16_t parameterIndex = inputs.size() - 1;
		
		return std::make_unique<AnimationLerpNode>(std::move(node1), std::move(node2), parameterIndex);
		break;
	}
	}
}

}// unnamed namespace

std::shared_ptr<AnimationGraph> LoadAnimationGraph(Details::Skeletal2D::SkeletonDesc const& skeletonDesc,
	AssetManager const& assets, std::string const& assetPath)
{
	auto filename = assets.RootDirectory() + "/" + assetPath;
	auto json = ReadBinaryFile(filename);
	
	rapidjson::Document doc;
	doc.Parse<0, rapidjson::UTF8<>>(json.data());
	
	if (doc.HasParseError())
	{
		///@todo Not implemented
		// Error
		POMDOG_ASSERT(false);
	}
	
	if (!doc.IsObject()) {
		///@todo Not implemented
		// Error
		POMDOG_ASSERT(false);
	}

	POMDOG_ASSERT(doc.HasMember("root"));
	POMDOG_ASSERT(doc["root"].IsUint());
	std::uint16_t rootNodeIndex = doc["root"].GetUint();
	
	std::vector<AnimationNodeDesc> nodes;
	if (doc.HasMember("nodes") && doc["nodes"].IsArray())
	{
		auto & nodeArray = doc["nodes"];
		for (auto iter = nodeArray.Begin(); iter != nodeArray.End(); ++iter)
		{
			POMDOG_ASSERT(iter->IsObject());
			POMDOG_ASSERT(iter->HasMember("id"));
			POMDOG_ASSERT(iter->HasMember("type"));
			
			auto & nodeObject = *iter;
			AnimationNodeDesc desc;
			
			auto & typeObject = nodeObject["type"];
			POMDOG_ASSERT(typeObject.IsString());
			if (0 == std::strncmp(typeObject.GetString(), "clip", 3)) {
				desc.Type = AnimationNodeType::Clip;
			}
			else if (0 == std::strncmp(typeObject.GetString(), "lerp", 3)) {
				desc.Type = AnimationNodeType::Lerp;
			}
			else {
				continue;
			}
			
			auto & idObject = nodeObject["id"];
			POMDOG_ASSERT(idObject.IsUint());
			desc.Id = idObject.GetUint();
			
			if (nodeObject.HasMember("clip") && nodeObject["clip"].IsString()) {
				desc.ClipName = nodeObject["clip"].GetString();
			}
			if (nodeObject.HasMember("param_name") && nodeObject["param_name"].IsString()) {
				desc.Parameter = nodeObject["param_name"].GetString();
			}
			if (nodeObject.HasMember("in") && nodeObject["in"].IsArray())
			{
				auto & inputs = nodeObject["in"];
				auto count = std::min<int>(inputs.Size(), desc.Inputs.size());
				for (int i = 0; i < count; ++i)
				{
					POMDOG_ASSERT(inputs[i].IsUint());
					desc.Inputs[i] = inputs[i].GetUint();
				}
			}
			nodes.push_back(std::move(desc));
		}
	}
	
	auto rootNodeDesc = std::find_if(std::begin(nodes),std::end(nodes), [&rootNodeIndex](AnimationNodeDesc const& desc){ return desc.Id == rootNodeIndex; });
	POMDOG_ASSERT(rootNodeDesc != std::end(nodes));
	
	auto animationGraph = std::make_shared<AnimationGraph>();
	std::vector<AnimationBlendInput> inputs;
	animationGraph->Tree = CreateAnimationNode(*rootNodeDesc, inputs, nodes, skeletonDesc);
	animationGraph->Inputs = std::move(inputs);

	return std::move(animationGraph);
}

}// namespace Skeletal2D
}// namespace Details
}// namespace Pomdog