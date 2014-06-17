﻿//
//  Copyright (C) 2013-2014 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#ifndef POMDOG_TEXTUREREGION_9F020F68_0E51_40CE_8219_6F207E478559_HPP
#define POMDOG_TEXTUREREGION_9F020F68_0E51_40CE_8219_6F207E478559_HPP

#if (_MSC_VER > 1000)
#	pragma once
#endif

#include <cstdint>
#include <Pomdog/Math/Rectangle.hpp>

namespace Pomdog {

class TextureRegion {
public:
	Rectangle Subrect;
	std::uint16_t XOffset;
	std::uint16_t YOffset;
	std::uint16_t Width;
	std::uint16_t Height;
	bool Rotate;
	bool Flip;
};

}// namespace Pomdog

#endif // !defined(POMDOG_TEXTUREREGION_9F020F68_0E51_40CE_8219_6F207E478559_HPP)