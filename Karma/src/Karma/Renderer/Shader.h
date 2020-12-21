#pragma once

#include "Karma/Core.h"
#include <string>

namespace Karma
{
	class KARMA_API Shader
	{
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void Bind() const;
		void UnBind() const;

	private:
		// OpenGL's identification scheme
		uint32_t m_RendererID;
	};
}