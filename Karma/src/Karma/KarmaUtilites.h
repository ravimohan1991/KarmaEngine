#pragma once

#include "Karma/Core.h"


namespace Karma
{
	class KARMA_API KarmaUtilities
	{
	public:
		static std::string ReadFileToSpitString(const std::string& filePath);
	};
}