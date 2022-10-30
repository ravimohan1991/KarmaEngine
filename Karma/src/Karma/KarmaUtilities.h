#pragma once

#include "krpch.h"

#include "Karma/KarmaUtilities.h"

namespace Karma
{
	class KARMA_API KarmaUtilities
	{
	public:
		static std::string ReadFileToSpitString(const std::string& filePath);
		static std::string GetFilePath(const std::string& str);
	};
}