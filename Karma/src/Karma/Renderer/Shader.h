#pragma once

#include "Karma/Core.h"
#include "glm/glm.hpp"
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

		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		// OpenGL's identification scheme
		uint32_t m_RendererID;
	};
}