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
add_subdirectory(Application)

# Build the Editor
add_subdirectory(Pranjal)

# Relevant linking of finished Application with the Engine!
target_link_libraries(SandBox PUBLIC KarmaEngine)
target_link_libraries(Pranjal PUBLIC KarmaEngine)

# Need to find a way to give access to the resources (assets and whatnot) to the built binaries


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
