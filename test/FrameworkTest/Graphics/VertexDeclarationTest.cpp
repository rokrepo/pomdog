﻿//
//  Copyright (C) 2013-2014 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#include <gtest/iutest_switch.hpp>
#include <Pomdog/Graphics/VertexDeclaration.hpp>

using Pomdog::VertexDeclaration;
using Pomdog::VertexElement;
using Pomdog::VertexElementFormat;

TEST(VertexDeclaration, CopyConstructor)
{
	std::vector<VertexElement> elements {
		{ 0, VertexElementFormat::Float }
	};
	VertexDeclaration declaration(elements);
	ASSERT_EQ(4, sizeof(float));
	EXPECT_EQ(sizeof(float), declaration.GetStrideBytes());
	
	ASSERT_FALSE(declaration.GetVertexElements().empty());
	ASSERT_EQ(1, declaration.GetVertexElements().size());
	auto element = declaration.GetVertexElements()[0];
	
	EXPECT_EQ(VertexElementFormat::Float, element.VertexElementFormat);
	EXPECT_EQ(0, element.Offset);
}

TEST(VertexDeclaration, Constructor_InitializerList)
{
	VertexDeclaration declaration {
		{ 0, VertexElementFormat::Float4 },
		{ 16, VertexElementFormat::Float3 },
		{ 32, VertexElementFormat::Float2 },
		{ 48, VertexElementFormat::Float }
	};
	ASSERT_EQ(4, sizeof(float));
	EXPECT_EQ(48 + 4, declaration.GetStrideBytes());
	
	ASSERT_FALSE(declaration.GetVertexElements().empty());
	ASSERT_EQ(4, declaration.GetVertexElements().size());
	auto elements = declaration.GetVertexElements();
	
	EXPECT_EQ(VertexElementFormat::Float4, elements[0].VertexElementFormat);
	EXPECT_EQ(VertexElementFormat::Float3, elements[1].VertexElementFormat);
	EXPECT_EQ(VertexElementFormat::Float2, elements[2].VertexElementFormat);
	EXPECT_EQ(VertexElementFormat::Float, elements[3].VertexElementFormat);
	
	EXPECT_EQ(0, elements[0].Offset);
	EXPECT_EQ(16, elements[1].Offset);
	EXPECT_EQ(32, elements[2].Offset);
	EXPECT_EQ(48, elements[3].Offset);
}
