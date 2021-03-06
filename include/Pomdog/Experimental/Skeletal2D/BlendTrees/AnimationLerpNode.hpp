// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include "Pomdog/Experimental/Skeletal2D/BlendTrees/AnimationNode.hpp"
#include <memory>

namespace Pomdog::Skeletal2D::Detail {

class AnimationLerpNode final : public AnimationNode {
public:
    AnimationLerpNode(
        std::unique_ptr<AnimationNode>&& blendNode1,
        std::unique_ptr<AnimationNode>&& blendNode2,
        std::uint16_t weightIndex);

    void Calculate(
        const AnimationTimeInterval& time,
        const Detail::AnimationGraphWeightCollection& weights,
        const Skeleton& skeleton,
        SkeletonPose& skeletonPose,
        Skin* skin) const override;

    AnimationTimeInterval GetLength() const override;

    [[nodiscard]] const AnimationNode* A() const noexcept;
    [[nodiscard]] const AnimationNode* B() const noexcept;

private:
    std::unique_ptr<AnimationNode> nodeA;
    std::unique_ptr<AnimationNode> nodeB;
    AnimationTimeInterval length;
    std::uint16_t weightIndex;
};

} // namespace Pomdog::Skeletal2D::Detail
