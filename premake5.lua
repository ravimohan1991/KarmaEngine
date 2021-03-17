-- premake5.lua
workspace "KarmaEngine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Application"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder
IncludeDir = {}
IncludeDir["GLFW"] = "Karma/vendor/GLFW/include"
IncludeDir["Glad"] = "Karma/vendor/Glad/include"
IncludeDir["ImGui"] = "Karma/vendor/ImGui"
IncludeDir["GLM"] = "Karma/vendor/GLM"
IncludeDir["Vulkan"] = "C:/VulkanSDK/1.2.162.0/Include"
IncludeDir["glslang"] = "Karma/vendor/glslang"

include "Karma/vendor/GLFW"
include "Karma/vendor/Glad"
include "Karma/vendor/ImGui"
include "Karma/vendor/glslang"

project "Karma"
	location "Karma"
	kind "StaticLib"
	language "C++"
	staticruntime "on"

	targetdir ("build/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/GLM/glm/**.hpp",
		"%{prj.name}/vendor/GLM/glm/**.inl"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.glslang}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"glslang"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		libdirs
		{
			"C:/VulkanSDK/1.2.162.0/Lib"
		}

		defines
		{
			"KR_WINDOWS_PLATFORM",
			"KR_BUILD_DLL"
		}

		links
		{
			--"opengl32.lib"
			"vulkan-1.lib"
		}		
	
	filter "system:linux"
		buildoptions "-std=c++11"

		defines
		{
			"KR_LINUX_PLATFORM",
			"KR_BUILD_SO"		
		}

		links
		{
			"GL"	
		}

	filter "system:macosx"
		buildoptions "-std=c++11"

		defines
		{
			"KR_MAC_PLATFORM",
			"KR_BUILD_SO"		
		}

		postbuildcommands
		{
		--	("{COPY} %{cfg.buildtarget.relpath} ../build/" .. outputdir .. "/Application")
		}	

	filter "configurations:Debug"
		defines
		{
			"KR_DEBUG",
			"KR_ENABLE_ASSERTS"
		}
		symbols "On"

	filter "configurations:Release"
		defines 
		{
			"KR_RELEASE",
			"KR_ENABLE_ASSERTS"
		}
		optimize "On"

	filter "configurations:Dist"
		defines "KR_DIST"
		optimize "On"

project "Application"
	location "Application"
	kind "ConsoleApp"
	language "C++"
	staticruntime "on"

	targetdir ("build/" .. outputdir .. "/%{prj.name}")
	objdir ("obj/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"Karma/vendor/spdlog/include",
		"Karma/src",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLM}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glslang}"				
	}

	links
	{
		"Karma"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"KR_WINDOWS_PLATFORM"
		}

	filter "system:linux"
		buildoptions "-std=c++11"
		linkoptions {"-pthread"}

		defines
		{
			"KR_LINUX_PLATFORM"		
		}	
	

		-- Karma application likning to Karma still wants these linking. In windows it works without these linkings.
		-- The issue began when we started compiling Karma as static library.	
		links
		{
			"dl",
			"GLFW",
			"Glad",
			"ImGui"	
		}

	filter "system:macosx"
		buildoptions "-std=c++11"
		linkoptions { '-Wl,-install_name', '-Wl,@loader_path/%{cfg.linktarget.name}' }

		defines
		{
			"KR_MAC_PLATFORM"		
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