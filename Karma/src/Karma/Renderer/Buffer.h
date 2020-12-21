#pragma once

#include "Karma/Core.h"
#include <stdint.h>

namespace Karma
{
	class KARMA_API VertexBuffer
	{
	public:
		virtual ~VertexBuffer()
		{
		}

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		static VertexBuffer* Create(float* vertices, uint32_t size);
	};

	class KARMA_API IndexBuffer
	{
	public:
		virtual ~IndexBuffer()
		{
		}

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual uint32_t GetCount() const = 0;

		static IndexBuffer* Create(uint32_t* vertices, uint32_t size);
	};
}