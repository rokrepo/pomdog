﻿//
//  Copyright (C) 2013-2015 mogemimi.
//  Distributed under the MIT License. See LICENSE.md or
//  http://enginetrouble.net/pomdog/license for details.
//

#ifndef POMDOG_SRC_OPENGLCONTEXTCOCOA_306ECA78_18CF_4A87_A039_0C823668622C_HPP
#define POMDOG_SRC_OPENGLCONTEXTCOCOA_306ECA78_18CF_4A87_A039_0C823668622C_HPP

#if (_MSC_VER > 1000)
#pragma once
#endif

#include "../RenderSystem.GL4/OpenGLContext.hpp"
#import <Cocoa/Cocoa.h>

@class NSOpenGLContext, NSOpenGLPixelFormat;

namespace Pomdog {
namespace Details {
namespace Cocoa {

class CocoaOpenGLContext final: public RenderSystem::GL4::OpenGLContext {
public:
	CocoaOpenGLContext() = delete;

	explicit CocoaOpenGLContext(NSOpenGLPixelFormat* pixelFormat);
	
	~CocoaOpenGLContext();

	void MakeCurrentContext() override;

	void ClearCurrentContext() override;

	void SwapBuffers() override;
	
	void LockContext();
	
	void UnlockContext();
	
	NSOpenGLContext* NativeOpenGLContext();
	
private:
	NSOpenGLContext* openGLContext;
	NSOpenGLPixelFormat* pixelFormat;
};

}// namespace Cocoa
}// namespace Details
}// namespace Pomdog

#endif // !defined(POMDOG_SRC_OPENGLCONTEXTCOCOA_306ECA78_18CF_4A87_A039_0C823668622C_HPP)