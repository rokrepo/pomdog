﻿//
//  Copyright (C) 2013-2014 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#ifndef POMDOG_DYNAMICVERTEXBUFFER_A2CEAA26_EDB4_40C8_B035_ADEEC90F7EA7_HPP
#define POMDOG_DYNAMICVERTEXBUFFER_A2CEAA26_EDB4_40C8_B035_ADEEC90F7EA7_HPP

#if (_MSC_VER > 1000)
#	pragma once
#endif

#include <memory>
#include "../Config/Export.hpp"
#include "detail/ForwardDeclarations.hpp"
#include "VertexBuffer.hpp"
#include "VertexDeclaration.hpp"

namespace Pomdog {

/// @addtogroup Framework
/// @{
/// @addtogroup Graphics
/// @{

///@~Japanese
/// @brief 動的な頂点バッファです。
class POMDOG_EXPORT DynamicVertexBuffer: public VertexBuffer
{
public:
	DynamicVertexBuffer() = delete;
	DynamicVertexBuffer(DynamicVertexBuffer const&) = delete;
	DynamicVertexBuffer(DynamicVertexBuffer &&) = default;

	DynamicVertexBuffer(std::shared_ptr<GraphicsDevice> const& graphicsDevice,
		Pomdog::VertexDeclaration const& vertexDeclaration, void const* vertices, std::uint32_t vertexCount);

	DynamicVertexBuffer(std::shared_ptr<GraphicsDevice> const& graphicsDevice,
		Pomdog::VertexDeclaration && vertexDeclaration, void const* vertices, std::uint32_t vertexCount);

	~DynamicVertexBuffer() override;

	DynamicVertexBuffer & operator=(DynamicVertexBuffer const&) = delete;
	DynamicVertexBuffer & operator=(DynamicVertexBuffer &&) = default;

	///@~Japanese
	/// @brief 頂点データの定義を取得します。
	Pomdog::VertexDeclaration const& VertexDeclaration() const override;

	///@~Japanese
	/// @brief バッファの使用方法を取得します。
	Pomdog::BufferUsage BufferUsage() const override;

	///@~Japanese
	/// @brief 頂点の数を取得します。
	std::uint32_t VertexCount() const override;

	///@~Japanese
	/// @brief 頂点データを格納します。
	/// @param source ソースバッファを指定します。
	/// @param elementCount 頂点の数を指定します。
	void SetData(void const* source, std::uint32_t elementCount);
	
public:
	Details::RenderSystem::NativeVertexBuffer* NativeVertexBuffer() override;

private:
	Pomdog::VertexDeclaration vertexDeclaration;
	std::unique_ptr<Details::RenderSystem::NativeVertexBuffer> nativeVertexBuffer;
	std::uint32_t vertexCount;
};

/// @}
/// @}

}// namespace Pomdog

#endif // !defined(POMDOG_DYNAMICVERTEXBUFFER_A2CEAA26_EDB4_40C8_B035_ADEEC90F7EA7_HPP)
