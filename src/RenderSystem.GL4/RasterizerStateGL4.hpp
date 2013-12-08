﻿//
//  Copyright (C) 2013 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#ifndef POMDOG_SRC_GL4_RASTERIZERSTATEGL4_1AEAC68F_5FFF_11E3_8B02_A8206655A22B_HPP
#define POMDOG_SRC_GL4_RASTERIZERSTATEGL4_1AEAC68F_5FFF_11E3_8B02_A8206655A22B_HPP

#if (_MSC_VER > 1000)
#	pragma once
#endif

#include <Pomdog/Graphics/detail/ForwardDeclarations.hpp>
#include "../RenderSystem/NativeRasterizerState.hpp"
#include <memory>

namespace Pomdog {
namespace Details {
namespace RenderSystem {
namespace GL4 {

class RasterizerStateGL4 final: public NativeRasterizerState
{
public:
	RasterizerStateGL4() = delete;
	
	explicit RasterizerStateGL4(RasterizerDescription const& description);
	
	~RasterizerStateGL4();
	
	///@copydoc NativeRasterizerState
	void Apply() override;
	
private:
	class Impl;
	std::unique_ptr<Impl> impl;
};

}// namespace GL4
}// namespace RenderSystem
}// namespace Details
}// namespace Pomdog

#endif // !defined(POMDOG_SRC_GL4_RASTERIZERSTATEGL4_1AEAC68F_5FFF_11E3_8B02_A8206655A22B_HPP)