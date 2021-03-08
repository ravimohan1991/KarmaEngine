#pragma once

#include "Karma/Core.h"
#include <string>
#include "glm/glm.hpp"

namespace Karma
{
	class KARMA_API Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const {}
		virtual void UnBind() const {}

		virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) {}

		static Shader* Create(const std::string& vertexSrc, const std::string& fragmentSrc);
	};
}