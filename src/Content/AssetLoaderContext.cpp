// Copyright (c) 2013-2016 mogemimi. Distributed under the MIT license.

#include "Pomdog/Content/detail/AssetLoaderContext.hpp"
#include "Pomdog/Utility/PathHelper.hpp"
#include <utility>

namespace Pomdog {
namespace Detail {
//-----------------------------------------------------------------------
BinaryFileStream AssetLoaderContext::OpenStream(std::string const& assetName) const
{
    auto filePath = PathHelper::Join(RootDirectory, assetName);
    return PathHelper::OpenStream(filePath);
}
//-----------------------------------------------------------------------
} // namespace Detail
} // namespace Pomdog
