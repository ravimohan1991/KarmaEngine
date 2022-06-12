#include "KarmaUtilites.h"

//PCH stuff
#include <fstream>

namespace Karma
{
	std::string KarmaUtilities::ReadFileToSpitString(const std::string& filePath)
	{
		std::string result;
        //std::ifstream in(filePath, std::ios::in, std::ios::binary); found the bug in Linux only
        std::ifstream in(filePath, std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			KR_CORE_ASSERT(false, "Could not open the file " + filePath);
		}

		return result;	
	}
}
