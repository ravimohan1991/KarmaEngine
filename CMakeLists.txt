#[[
    Abstractions and Models are NOT at WAR!
                                            - Cobwoy's Talisman
    But Abstractions don't care and Models can't understand!!
                                            - Lul, Practicality
 ]]
cmake_minimum_required(VERSION 4.0)
cmake_policy(VERSION 4.0)

# This is the top level CMakeList file having single responsibility of
# generating enough context for entire Software (Engine + Application).

# First we specify the C++/C standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 17)

# Control where the static and shared libraries are built
# We would want a semi-packaging fashioned way where all bianries are together
# whilst accessible from IDE (local debugger for instance!).
set(REPOSITORYROOT "${CMAKE_CURRENT_SOURCE_DIR}")
# Global variable for Karma's Libraries
set(KARMALIBRARYFOLDER "CompanionLibraries")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${REPOSITORYROOT}/KarmaBin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${REPOSITORYROOT}/KarmaBin")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${REPOSITORYROOT}/KarmaBin")

# For MacOS .app packaging
# Choose install scheme for packaging
set(MAC_PACKAGE FALSE)

# Set custom install prefix inside CMakeLists.txt
# This is so that .app can be installed in KarmaBin
if (APPLE)
	if(MAC_PACKAGE)
	add_compile_definitions(KR_MAC_APP_BUNDLE)
		# We will be needing the install scheme so generate all the schemes
		set(CMAKE_XCODE_GENERATE_SCHEME ON CACHE BOOL "")
		set(CMAKE_INSTALL_PREFIX "${REPOSITORYROOT}/KarmaBin" CACHE PATH
			"Custom install location for .app bundle" FORCE)
	else()
		# No need for generating all the schemes. Selected schemes can be
		# be generated for appropriate targets. See Pranjal
		#     set_target_properties(Pranjal PROPERTIES
		#             XCODE_GENERATE_SCHEME TRUE
		#             XCODE_SCHEME_WORKING_DIRECTORY "${REPOSITORYROOT}/KarmaBin")
		set(CMAKE_XCODE_GENERATE_SCHEME OFF CACHE BOOL "")
	endif()
endif()

# Set the GameName
set(KARMA_GAME_NAME "Game_Of_Lands")

# Finally we specify policies, if any
# This one is for inter-directory linking
cmake_policy(SET CMP0079 NEW)

# Setting some options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Setting some properties
# For grouping Libraries in folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# Clustering ALL_BUILD and related crap projects (within the context of IDE)
# set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "Si")
# If want to disable ZERO_CHECK
# set(CMAKE_SUPPRESS_REGENERATION true)
#----------------------------------------------------------------------------#

# Set Engine project
project(${KARMA_GAME_NAME} VERSION 1.0)
add_compile_definitions(KR_APPLICATION_NAME="${KARMA_GAME_NAME}")

# Setting some flags

# For complete list of Compiler IDs check output_required_files
# https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html#variable:CMAKE_%3CLANG%3E_COMPILER_ID
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-enum-enum-conversion")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror-implicit-function-declaration")
endif()

# Then we specify the involved libraries
add_subdirectory(Karma)

# Here we define essential libraries for proofing subsequent client building
list(APPEND ESSENTIAL_LIBS Karma)

# Build the Client
# add_subdirectory(Application)

# Build the Editor
add_subdirectory(Pranjal)

# Relevant linking of finished Application with the Engine!
# target_link_libraries(SandBox PUBLIC KarmaEngine)
target_link_libraries(Pranjal PUBLIC KarmaEngine)

if (APPLE AND MAC_PACKAGE)
set_target_properties(Pranjal PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Pranjal"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.pranjal"
    MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
	# Ensure rpath for finding Frameworks
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "@executable_path/../Frameworks"
)

target_link_libraries(Pranjal PUBLIC "-framework CoreFoundation")

# Install .app to custom prefix (creates KarmaBin/Pranjal.app)
install(TARGETS Pranjal
    BUNDLE DESTINATION "${CMAKE_INSTALL_PREFIX}"
    RUNTIME DESTINATION Pranjal.app/Contents/Frameworks
    COMPONENT Runtime
)

# Install library target inside app bundle (uses target name automatically)
install(TARGETS Glad glfw glm SDL2 spdlogging AssImp GlSlang stbimage hwinfo KarmaEngine
    hwinfo hwinfo_battery hwinfo_cpu hwinfo_disk hwinfo_gpu hwinfo_mainboard hwinfo_os hwinfo_ram hwinfo_network
    LIBRARY DESTINATION Pranjal.app/Contents/Frameworks
    COMPONENT Runtime
)

# Get CORRECT Vulkan SDK path
if(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_SDK_PATH $ENV{VULKAN_SDK})
else()
    # Fallback paths
    set(VULKAN_SDK_PATH "/Users/$ENV{USER}/VulkanSDK/1.3.268.1/macOS")  # Adjust version
endif()

set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/lib")
message(STATUS "Vulkan SDK: ${VULKAN_SDK_PATH}")
message(STATUS "Vulkan lib dir: ${VULKAN_LIB_DIR}")

# Copy BOTH real Vulkan libraries
install(FILES
    "${VULKAN_LIB_DIR}/libvulkan.1.dylib"
    "${VULKAN_LIB_DIR}/libvulkan.1.3.268.dylib"  # Real versioned file
    DESTINATION Pranjal.app/Contents/Frameworks
    COMPONENT Runtime
)

# Create libvulkan.dylib symlink pointing to libvulkan.1.dylib
install(CODE "
    execute_process(
        COMMAND ln -sf libvulkan.1.dylib libvulkan.dylib
        WORKING_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/Pranjal.app/Contents/Frameworks\"
    )
")

# Finally install the Resources folder
install(DIRECTORY ${REPOSITORYROOT}/Resources
    DESTINATION Pranjal.app/Contents
    COMPONENT Runtime
)
endif()

#[[
 *                                          /\
 *                                         / /
 *                                      /\| |
 *                                      | | |/\
 *                                      | | / /
 *                                      | `  /
 *                                      `\  (___
 *                                     _.->  ,-.-.
 *                                  _.'      |  \ \
 *                                 /    _____| 0 |0\
 *                                |    /`    `^-.\.-'`-._
 *                                |   |                  `-._
 *                                |   :                      `.
 *                                \    `._     `-.__         O.'
 *         _.--,                   \     `._     __.^--._O_..-'
 *        `---, `.                  `\     /` ` `
 *             `\ `,                  `\   |
 *              |   :                   ;  |
 *              /    `.              ___|__|___
 *             /       `.           (          )
 *            /    `---.:____...---' `--------`.
 *           /        (         `.      __      `.
 *          |          `---------' _   /  \       \
 *          |    .-.      _._     (_)  `--'        \
 *          |   (   )    /   \                       \
 *           \   `-'     \   /                       ;-._
 *            \           `-'           \           .'   `.
 *            /`.                  `\    `\     _.-'`-.    `.___
 *           |   `-._                `\    `\.-'       `-.   ,--`
 *            \      `--.___        ___`\    \           ||^\\
 *             `._        | ``----''     `.   `\         `'  `
 *                `--;     \  jgs          `.   `.
 *                   //^||^\\               //^||^\\
 *                   '  `'  `               '   '  `
 ]]
