// Copyright (c) 2013-2015 mogemimi. Distributed under the MIT license.

#pragma once

#include "Pomdog/Basic/Export.hpp"
#include "BinaryFileStream.hpp"
#include <string>
#include <tuple>

namespace Pomdog {
namespace Detail {

struct POMDOG_EXPORT PathHelper {
    static std::string Join(std::string const& path1, std::string const& path2);

    static BinaryFileStream OpenStream(std::string const& path);

    static std::string GetBaseName(std::string const& path);

    static std::string GetDirectoryName(std::string const& path);

    static std::tuple<std::string, std::string> Split(std::string const& path);

    static std::tuple<std::string, std::string> SplitExtension(std::string const& path);
};

} // namespace Detail
} // namespace Pomdog
