// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#include "FormatHelper.hpp"
#include "Pomdog/Graphics/ComparisonFunction.hpp"

namespace Pomdog::Detail::Vulkan {

VkCompareOp ToComparisonFunction(ComparisonFunction compareFunction) noexcept
{
    switch (compareFunction) {
    case ComparisonFunction::Never:
        return VK_COMPARE_OP_NEVER;
    case ComparisonFunction::Less:
        return VK_COMPARE_OP_LESS;
    case ComparisonFunction::Equal:
        return VK_COMPARE_OP_EQUAL;
    case ComparisonFunction::LessEqual:
        return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ComparisonFunction::Greater:
        return VK_COMPARE_OP_GREATER;
    case ComparisonFunction::NotEqual:
        return VK_COMPARE_OP_NOT_EQUAL;
    case ComparisonFunction::GreaterEqual:
        return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ComparisonFunction::Always:
        return VK_COMPARE_OP_ALWAYS;
    }
    return VK_COMPARE_OP_LESS_OR_EQUAL;
}

} // namespace Pomdog::Detail::Vulkan
