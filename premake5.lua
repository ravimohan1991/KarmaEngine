-- premake5.lua
workspace "KarmaEngine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Application"
	location "Application"
	kind "ConsoleApp"
	language "C++"

	targetdir ("build/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	-- find macros corresponding to Karma project instead of hardcoding
	includedirs
	{
		"Karma/vendor/spdlog/include",
		"Karma/src"
	}

	links
	{
		"Karma"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"KR_WINDOWS_PLATFORM"
		}

	filter "configurations:Debug"
		defines "KR_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "KR_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "KR_DIST"
		optimize "On"

project "Karma"
	location "Karma"
	kind "SharedLib"
	language "C++"

	targetdir ("build/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/vendor/spdlog/include"
	}

	filter "system:windows"
		cppdialect "C++17"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"KR_WINDOWS_PLATFORM",
			"KR_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../build/" .. outputdir .. "/Application")
		}

	filter "configurations:Debug"
		defines "KR_DEBUG"
		symbols "On"

	filter "configurations:Release"
		defines "KR_RELEASE"
		optimize "On"

	filter "configurations:Dist"
		defines "KR_DIST"
		optimize "On"