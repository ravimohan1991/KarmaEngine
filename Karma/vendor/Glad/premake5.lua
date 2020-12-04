-- premake5.lua
project "Glad"
	kind "StaticLib"
	language "C"

	targetdir ("build/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files
	{
		"include/glad/glad.h",
		"include/KHR/khrplatform.h",
		"src/glad.c"
	}

	includedirs
	{
		"include"
	}

	filter "system:windows"
		systemversion "latest"
		staticruntime "off"

	filter "system:linux"
		-- https://github.com/premake/premake-core/wiki/pic
		pic "On"

		systemversion "latest"
		staticruntime "off"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"