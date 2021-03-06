// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include "AddressFamily.hpp"
#include <string>
#include <tuple>

namespace Pomdog::Detail::AddressParser {

std::tuple<AddressFamily, std::string_view, std::string_view>
TransformAddress(const std::string_view& address);

} // namespace Pomdog::Detail::AddressParser
