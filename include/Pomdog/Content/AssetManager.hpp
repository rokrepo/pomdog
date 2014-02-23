//
//  Copyright (C) 2013-2014 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#ifndef POMDOG_ASSETMANAGER_FAF5E4A4_3E1E_4A97_BA13_57E85E5B9156_HPP
#define POMDOG_ASSETMANAGER_FAF5E4A4_3E1E_4A97_BA13_57E85E5B9156_HPP

#if (_MSC_VER > 1000)
#	pragma once
#endif

#include <utility>
#include "detail/AssetLoader.hpp"

namespace Pomdog {

/// @addtogroup Framework
/// @{
/// @addtogroup Content
/// @{

///@~Japanese
/// @brief アセットを読み込みます。
class AssetManager
{
public:
	AssetManager(Details::AssetLoaderContext && loaderContext);

	///@~Japanese
	/// @brief アセットを読み込みます。
	template <typename T>
	std::shared_ptr<T> Load(std::string const& assetPath)
	{
		Details::AssetLoader<T> loader;
		return loader(loaderContext, assetPath);
	}
	
	std::string RootDirectory() const
	{
		return loaderContext.rootDirectory;
	}
	
private:
	Details::AssetLoaderContext loaderContext;
};

/// @}
/// @}

}// namespace Pomdog

#endif // !defined(POMDOG_ASSETMANAGER_FAF5E4A4_3E1E_4A97_BA13_57E85E5B9156_HPP)