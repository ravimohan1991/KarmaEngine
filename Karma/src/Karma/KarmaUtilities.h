#pragma once

#include "krpch.h"

#include "stb_image.h"

namespace Karma
{
	class KARMA_API KarmaUtilities
	{
	public:
		static std::string ReadFileToSpitString(const std::string& filePath);
		static std::string GetFilePath(const std::string& str);
		static unsigned char* GetImagePixelData(char const* fileName, int* width, int* height, int* channels, int req_comp);
	};
}