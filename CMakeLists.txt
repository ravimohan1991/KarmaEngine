cmake_minimum_required(VERSION 3.4)

# This is the top level CMakeList file having single responsibility of
# generating enough context for entire Software (Engine + Application).


# First we specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Finally we specify policies, if any
# This one is for inter-directory linking
cmake_policy(SET CMP0079 NEW)

# Setting some options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Platform specific Defines
if(WIN32)
add_compile_definitions(KR_WINDOWS_PLATFORM)
elseif(UNIX AND NOT APPLE)
add_compile_definitions(KR_LINUX_PLATFORM)
elseif(APPLE)
add_compile_definitions(KR_MAC_PLATFORM)
endif()

#add_compile_definitions(KR_ENABLE_ASSERTS)
add_compile_definitions(KR_DYNAMIC_LINK KR_BUILD_SO)


# Configurations for Debug/Release candidates. Need more fluidity
if(CMAKE_BUILD_TYPE STREQUAL DEBUG)
    add_compile_definitions(KR_ENABLE_ASSERTS)
endif()

# Set Engine project
project(KarmaBasedProjectName VERSION 1.0)

# Then we specify the involved libraries
add_subdirectory(Karma)

# Here we define essential libraries for proofing subsequent building
list(APPEND ESSENTIAL_LIBS Karma)

# Build the Client
add_subdirectory(Application)

# Relevant linking of finished Application with the Engine!
target_link_libraries(SandBox PUBLIC KarmaEngine)

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
