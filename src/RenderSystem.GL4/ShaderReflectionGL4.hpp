//
//  Copyright (C) 2013-2014 mogemimi.
//
//  Distributed under the MIT License.
//  See accompanying file LICENSE.md or copy at
//  http://enginetrouble.net/pomdog/LICENSE.md for details.
//

#ifndef POMDOG_SRC_GL4_SHADERREFLECTIONGL4_98947466_A9FF_4349_A3F0_876FEF470821_HPP
#define POMDOG_SRC_GL4_SHADERREFLECTIONGL4_98947466_A9FF_4349_A3F0_876FEF470821_HPP

#if (_MSC_VER > 1000)
#	pragma once
#endif

#include <string>
#include <vector>
#include "OpenGLPrerequisites.hpp"
#include <Pomdog/Config/FundamentalTypes.hpp>
#include "../RenderSystem/NativeShaderReflection.hpp"
#include "TypesafeGL4.hpp"

namespace Pomdog {
namespace Details {
namespace RenderSystem {
namespace GL4 {

struct UniformVariableGL4
{
	std::string Name;
	GLuint Offset;
	GLenum ByteLength;
	GLenum Type;
	GLuint ArrayStride;
	GLuint MatrixStride;
	bool IsRowMajor;
};

struct UniformBlockGL4
{
	std::vector<UniformVariableGL4> Uniforms;
	std::string Name;
	std::uint32_t ByteConstants;
	std::uint32_t BlockIndex;
};

class ShaderReflectionGL4: public NativeShaderReflection
{
public:
	ShaderReflectionGL4() = delete;
	
	explicit ShaderReflectionGL4(ShaderProgramGL4 const& shaderProgram);
	~ShaderReflectionGL4() = default;

	std::vector<EffectBufferDescription> GetConstantBuffers() const override;

public:
	static std::vector<UniformBlockGL4> GetNativeUniformBlocks(ShaderProgramGL4 const& shaderProgram);
	
private:
	ShaderProgramGL4 shaderProgram;
};

}// namespace GL4
}// namespace RenderSystem
}// namespace Details
}// namespace Pomdog

#endif // !defined(POMDOG_SRC_GL4_SHADERREFLECTIONGL4_98947466_A9FF_4349_A3F0_876FEF470821_HPP)
