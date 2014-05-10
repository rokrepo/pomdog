//
//  Copyright (C) 2013-2014 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#include "Skeleton.hpp"

namespace Pomdog {
//-----------------------------------------------------------------------
Skeleton::Skeleton(std::vector<Joint> && jointsIn)
	: joints(std::move(jointsIn))
{
	POMDOG_ASSERT(!joints.empty());
	POMDOG_ASSERT(!joints.front().Parent);
}
//-----------------------------------------------------------------------
Joint const& Skeleton::Root() const
{
	POMDOG_ASSERT(!joints.empty());
	return joints.front();
}
//-----------------------------------------------------------------------
std::vector<Joint> const& Skeleton::Joints() const
{
	return joints;
}
//-----------------------------------------------------------------------
Joint const& Skeleton::Joints(JointIndex const& jointIndex) const
{
	POMDOG_ASSERT(jointIndex);
	POMDOG_ASSERT(*jointIndex < joints.size());
	return joints[*jointIndex];
}
//-----------------------------------------------------------------------
std::uint16_t Skeleton::JointCount() const
{
	POMDOG_ASSERT(joints.size() < std::numeric_limits<std::uint16_t>::max());
	return static_cast<std::uint16_t>(joints.size());
}
//-----------------------------------------------------------------------
}// namespace Pomdog