// Copyright (c) 2013-2020 mogemimi. Distributed under the MIT license.

#pragma once

#include <system_error>

namespace Pomdog::Detail {

std::errc ToErrc(int err);

} // namespace Pomdog::Detail
