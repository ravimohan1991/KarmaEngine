#include "OpenGLBuffer.h"
#include "glad/glad.h"

#include "Karma/KarmaUtilities.h"

namespace Karma
{
	// VertexBuffer

	OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
	{
		//glCreateBuffers(1, &m_RendererID);
		// Courtsey MacOS Monterey's OpenGL (v 4.1) bottle neck
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
		// Upload to GPU
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	OpenGLVertexBuffer::~OpenGLVertexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLVertexBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLVertexBuffer::UnBind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLVertexBuffer::SetLayout(const BufferLayout& layout)
	{
		m_Layout = layout;
	}

	// IndexBuffer

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		//glCreateBuffers(1, &m_RendererID);
		glGenBuffers(1, &m_RendererID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
		// Upload to GPU
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	OpenGLIndexBuffer::~OpenGLIndexBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLIndexBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
	}

	void OpenGLIndexBuffer::UnBind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	// OpenGLImageBuffer
	unsigned int OpenGLImageBuffer::m_ImageBufferID;
	void OpenGLImageBuffer::SetUpImageBuffer(const char* filenames)
	{
		// Load and create a texture. Need proper texture loading abstraction
		//unsigned int texture1;
		glGenTextures(1, &m_ImageBufferID);
		glBindTexture(GL_TEXTURE_2D, m_ImageBufferID);
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// enable z-test
		glEnable(GL_DEPTH_TEST);

		// load image, create texture, and generate mipmaps
		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
		// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
		unsigned char* data = KarmaUtilities::GetImagePixelData(filenames, &width, &height, &nrChannels, STBI_rgb_alpha);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			KR_CORE_ASSERT(data, "Failed to load textures image!");
		}
		stbi_image_free(data);
	}

	void OpenGLImageBuffer::BindTexture()
	{
		glBindTexture(GL_TEXTURE_2D, m_ImageBufferID);
	}

	OpenGLUniformBuffer::OpenGLUniformBuffer(std::vector<ShaderDataType> dataTypes, uint32_t bindingPointIndex) :
		UniformBufferObject(dataTypes, bindingPointIndex)
	{
		GenerateUniformBufferObject();
		BindUniformBufferObject();
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_UniformsID);
	}

	void OpenGLUniformBuffer::GenerateUniformBufferObject()
	{
		//glCreateBuffers(1, &m_UniformsID);
		glGenBuffers(1, &m_UniformsID);
		glBindBuffer(GL_ARRAY_BUFFER, m_UniformsID);
	}

	void OpenGLUniformBuffer::BindUniformBufferObject() const
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_UniformsID);
		glBufferData(GL_UNIFORM_BUFFER, GetBufferSize(), NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void OpenGLUniformBuffer::UploadUniformBuffer(size_t frameIndex)
	{
		uint32_t index = 0;
		for (auto it : GetUniformList())
		{
			uint32_t uniformSize = GetUniformSize()[index];
			uint32_t offset = GetAlignedOffsets()[index++];

			glBindBuffer(GL_UNIFORM_BUFFER, GetUniformsID());
			glBufferSubData(GL_UNIFORM_BUFFER, offset, uniformSize, it.GetDataPointer());
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		glBindBufferRange(GL_UNIFORM_BUFFER, GetBindingPointIndex(), GetUniformsID(), 0, GetBufferSize());
	}
};
