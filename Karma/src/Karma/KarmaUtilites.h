#pragma once

#include "Karma/Core.h"
// PCH stuff
#include <string>

namespace Karma
{
	class KARMA_API KarmaUtilities
	{
	public:
		static std::string ReadFileToSpitString(const std::string& filePath);
	};
}
