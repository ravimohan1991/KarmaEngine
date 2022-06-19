# Macros
MACRO(COLLECTIVEINCLUDES CurrentDir)
    FILE(GLOB_RECURSE Children LIST_DIRECTORIES true RELATIVE ${CurrentDir} ${CurrentDir}/*)
    FOREACH(Child ${Children})
        IF(IS_DIRECTORY ${CurrentDir}/${Child})
        target_include_directories(KarmaEngine
            PRIVATE
            ${CurrentDir}/${Child}
            PUBLIC
            $<BUILD_INTERFACE:${CurrentDir}/${Child}>
        )
        ENDIF()
    ENDFOREACH()
ENDMACRO()

# Then the checks
find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
message(STATUS "Checks are satisfied, we are good to proceed!")

# Vulkan specific code
# also see https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-To-Write-Platform-Checks#platform-variables
if(WIN32)
  set(_Vulkan_library_name vulkan-1)
  set(_Vulkan_hint_include_search_paths
    "$ENV{VULKAN_SDK}/Include"
  )
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_Vulkan_hint_executable_search_paths
      "$ENV{VULKAN_SDK}/Bin"
    )
    set(_Vulkan_hint_library_search_paths
      "$ENV{VULKAN_SDK}/Lib"
      "$ENV{VULKAN_SDK}/Bin"
    )
  else()
    set(_Vulkan_hint_executable_search_paths
      "$ENV{VULKAN_SDK}/Bin32"
    )
    set(_Vulkan_hint_library_search_paths
      "$ENV{VULKAN_SDK}/Lib32"
      "$ENV{VULKAN_SDK}/Bin32"
    )
  endif()
else()
  set(_Vulkan_library_name vulkan)
  set(_Vulkan_hint_include_search_paths
    "$ENV{VULKAN_SDK}/include"
  )
  set(_Vulkan_hint_executable_search_paths
    "$ENV{VULKAN_SDK}/bin"
  )
  set(_Vulkan_hint_library_search_paths
    "$ENV{VULKAN_SDK}/lib"
  )
endif()
if(APPLE AND DEFINED ENV{VULKAN_SDK})
  cmake_path(SET _MoltenVK_path NORMALIZE "$ENV{VULKAN_SDK}/../MoltenVK")
  if(EXISTS "${_MoltenVK_path}")
    list(APPEND _Vulkan_hint_include_search_paths
      "${_MoltenVK_path}/include"
    )
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
      list(APPEND _Vulkan_hint_library_search_paths
        "${_MoltenVK_path}/dylib/iOS"
      )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "tvOS")
      list(APPEND _Vulkan_hint_library_search_paths
        "${_MoltenVK_path}/dylib/tvOS"
      )
    else()
      list(APPEND _Vulkan_hint_library_search_paths
        "${_MoltenVK_path}/dylib/macOS"
      )
    endif()
  endif()
  unset(_MoltenVK_path)
endif()
if(UNIX AND NOT APPLE)
    # May need to set up in Windows and Mac
    message(STATUS "YEEHAW FOUND LINUX only")
    find_path(Vulkan_INCLUDE_DIR
      NAMES vulkan/vulkan.h
      HINTS
        ${_Vulkan_hint_include_search_paths}
      )
    mark_as_advanced(Vulkan_INCLUDE_DIR)

    find_library(Vulkan_LIBRARY
      NAMES ${_Vulkan_library_name}
      HINTS
        ${_Vulkan_hint_library_search_paths}
      )
    mark_as_advanced(Vulkan_LIBRARY)
endif()

if(WIN32)
    if(BUILD_SHARED_LIBS)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()
endif()

# Platform specific Defines
if(WIN32)
    add_compile_definitions(KR_WINDOWS_PLATFORM)
	if(MSVC)
	add_definitions(/MP)
	endif()
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(KR_LINUX_PLATFORM)
elseif(APPLE)
    add_compile_definitions(KR_MAC_PLATFORM)
endif()

# Involved libraries and essentials
add_subdirectory(vendor/Glad)
add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/GLM)
add_subdirectory(vendor/ImGui)
add_subdirectory(vendor/spdlog)
add_subdirectory(vendor/assimp)
add_subdirectory(vendor/glslang)
add_subdirectory(vendor/stb)

list(APPEND ESSENTIAL_LIBS Glad)
list(APPEND ESSENTIAL_LIBS GLFW)
list(APPEND ESSENTIAL_LIBS glm)
list(APPEND ESSENTIAL_LIBS ImGui)
list(APPEND ESSENTIAL_LIBS spdlog)
list(APPEND ESSENTIAL_LIBS AssImp)
list(APPEND ESSENTIAL_LIBS ${Vulkan_LIBRARY})
list(APPEND ESSENTIAL_LIBS GlSlang)
list(APPEND ESSENTIAL_LIBS stbimage)

# Set the project name and version
project(KarmaEngine VERSION 1.0) # Versioning shall change to something more appropriate

# Handling MSVC static class members for dynamic linkage. I know!
if(MSVC AND BUILD_SHARED_LIBS)
    if(WIN32)
        # Let Karma handle them
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
        add_compile_definitions(KR_DYNAMIC_LINK KR_BUILD_DLL)
        # Disable stupid C4251 warnings due to STL's negligence
        add_definitions(/wd4251)
    elseif(APPLE)
        add_compile_definitions(KR_DYNAMIC_LINK KR_BUILD_SO) # May need to find appropriate define name because Apple doesn't generate SO
    endif()
endif()

# Karma's personal code
file(GLOB_RECURSE CPPFILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
# Shows the headerfile directory in project
file(GLOB_RECURSE HEADERFILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# Building the project (STATIC or DYNAMIC (SHARED))
if (BUILD_SHARED_LIBS)
    add_library(KarmaEngine SHARED ${CPPFILES} ${HEADERFILES})
else()
    add_library(KarmaEngine STATIC ${CPPFILES} ${HEADERFILES})
endif()

# Linking to common point
target_link_libraries(KarmaEngine PUBLIC Glad)
target_link_libraries(KarmaEngine PUBLIC glfw)
target_link_libraries(KarmaEngine PUBLIC glm)
target_link_libraries(KarmaEngine PUBLIC ImGui)
target_link_libraries(KarmaEngine PUBLIC spdlogging)
target_link_libraries(KarmaEngine PUBLIC AssImp)
target_link_libraries(KarmaEngine PUBLIC ${Vulkan_LIBRARY})
target_link_libraries(KarmaEngine PUBLIC GlSlang)
target_link_libraries(KarmaEngine PUBLIC stbimage)

# INTERFACE (Something that Consumers require but Producers don't)
# Since I am a library coder here, I am the Producer
# Rest of the Engine is Consumer here

target_include_directories(KarmaEngine
    PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp/contrib
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp/include
        ${Vulkan_INCLUDE_DIR}
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Include sub-directories includes
COLLECTIVEINCLUDES(${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(KarmaEngine PUBLIC KarmaEngine)
