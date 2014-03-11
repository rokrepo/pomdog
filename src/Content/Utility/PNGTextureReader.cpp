﻿//
//  Copyright (C) 2013-2014 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#include "PNGTextureReader.hpp"
#include <vector>
extern "C" {
	#include <png.h>
}
#include <Pomdog/Utility/Assert.hpp>
#include <Pomdog/Utility/Exception.hpp>
#include <Pomdog/Graphics/Texture2D.hpp>
#include <Pomdog/Graphics/SurfaceFormat.hpp>
#include "../../Utility/ScopeGuard.hpp"

namespace Pomdog {
namespace Details {
namespace {

struct PNGBinaryContext
{
	std::uint8_t const* Data;
	std::size_t ByteLength;
	std::size_t Offset;
};

struct Texture2DParsingData
{
	std::vector<std::uint8_t> Binary;
	std::uint32_t Height;
	std::uint32_t Width;
	std::uint32_t MipmapLevelCount;
	SurfaceFormat Format;
};
//-----------------------------------------------------------------------
static void ReadPNGDataCallback(::png_structp png_ptr, ::png_bytep data, ::png_size_t length)
{
	auto context = static_cast<PNGBinaryContext*>(::png_get_io_ptr(png_ptr));

	if ((context->Offset + length) <= context->ByteLength) {
		std::memcpy(data, context->Data + context->Offset, length);
		context->Offset += length;
	}
	else {
		::png_error(png_ptr, "ReadPngCallback failed.");
	}
}
//-----------------------------------------------------------------------
static Texture2DParsingData ReadPNG(std::uint8_t const* data, std::size_t byteLength)
{
	auto pngPtr = ::png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (nullptr == pngPtr) {
		POMDOG_THROW_EXCEPTION(std::runtime_error, "Not implemented.");
	}
	
	auto infoPtr = ::png_create_info_struct(pngPtr);
	
	ScopeGuard scopedDestroyReadStruct([&] {
		if (nullptr != pngPtr)
		{
			if (nullptr != infoPtr) {
				::png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
			}
			else {
				::png_destroy_read_struct(&pngPtr, nullptr, nullptr);
			}
		}
	});
	
	if (nullptr == infoPtr) {
		POMDOG_THROW_EXCEPTION(std::runtime_error, "Not implemented.");
	}
	
	PNGBinaryContext context;
	context.Data = data;
	context.ByteLength = byteLength;
	context.Offset = 0;
	::png_set_read_fn(pngPtr, &context, ReadPNGDataCallback);
	
	constexpr auto pngSignatureByteLength = sizeof(std::uint8_t) * 8;
	::png_set_sig_bytes(pngPtr, pngSignatureByteLength);
	
	// Read PNG Header
	::png_read_info(pngPtr, infoPtr);
	auto const originalColorType = ::png_get_color_type(pngPtr, infoPtr);
	auto const originalBitDepth = ::png_get_bit_depth(pngPtr, infoPtr);
	
	// Settings
	if ((PNG_COLOR_TYPE_GRAY == originalColorType) && (originalBitDepth < 8)) {
		::png_set_expand_gray_1_2_4_to_8(pngPtr);
	}
	else if (originalBitDepth < 8) {
		::png_set_packing(pngPtr);
	}
	else if (originalBitDepth == 16) {
		::png_set_strip_16(pngPtr);
	}
	
	if (PNG_COLOR_TYPE_PALETTE == originalColorType) {
		::png_set_palette_to_rgb(pngPtr);
	}
	
	if (::png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) {
		::png_set_tRNS_to_alpha(pngPtr);
	}
	
	::png_read_update_info(pngPtr, infoPtr);
	
	// Texture2D Info
	auto const pixelWidth = ::png_get_image_width(pngPtr, infoPtr);
	auto const pixelHeight = ::png_get_image_height(pngPtr, infoPtr);
	auto const colorType = ::png_get_color_type(pngPtr, infoPtr);
	//auto const bitDepth = ::png_get_bit_depth(pngPtr, infoPtr);
	
	POMDOG_ASSERT(pixelWidth > 0);
	POMDOG_ASSERT(pixelHeight > 0);
	
	// Read PNG Image Data
	auto const rowBytes = ::png_get_rowbytes(pngPtr, infoPtr);
	std::vector<std::uint8_t> rowData(rowBytes * pixelHeight * sizeof(png_byte));

	std::vector<::png_bytep> bytePointers(pixelHeight, nullptr);
	
	for (std::uint32_t index = 0; index < pixelHeight; ++index) {
		POMDOG_ASSERT(rowData.size() > rowBytes * index);
		bytePointers[index] = &rowData[rowBytes * index];
	}
	
	::png_read_image(pngPtr, bytePointers.data());
	::png_read_end(pngPtr, nullptr);
	
	Texture2DParsingData parsingData;
	parsingData.Width = pixelWidth;
	parsingData.Height = pixelHeight;
	parsingData.MipmapLevelCount = 1;
	parsingData.Format = ([](::png_byte colorTypeIn)->SurfaceFormat{
		switch (colorTypeIn) {
		case PNG_COLOR_TYPE_GRAY:
			return SurfaceFormat::R8_UNorm;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			return SurfaceFormat::R8G8_UNorm;
		case PNG_COLOR_TYPE_RGB:
			return SurfaceFormat::R8G8B8_UNorm;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			return SurfaceFormat::R8G8B8A8_UNorm;
		default:
			POMDOG_ASSERT(colorTypeIn != PNG_COLOR_TYPE_PALETTE);
			break;
		}
		return SurfaceFormat::A8_UNorm;
	})(colorType);
	
	parsingData.Binary = std::move(rowData);

	return std::move(parsingData);
}

}// unnamed namespace
//-----------------------------------------------------------------------
std::shared_ptr<Texture2D> PNGTextureReader::Read(std::shared_ptr<GraphicsDevice> const& graphicsDevice,
	std::uint8_t const* data, std::size_t byteLength)
{
	POMDOG_ASSERT(graphicsDevice);
	POMDOG_ASSERT(data != nullptr);
	POMDOG_ASSERT(byteLength > 0);
	
	auto parsingData = ReadPNG(data, byteLength);

	auto texture = std::make_shared<Texture2D>(graphicsDevice,
		parsingData.Width, parsingData.Height,
		parsingData.MipmapLevelCount, parsingData.Format);

	POMDOG_ASSERT(!parsingData.Binary.empty());
	texture->SetData(parsingData.Binary.data());
	return std::move(texture);
}

}// namespace Details
}// namespace Pomdog
