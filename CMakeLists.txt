#[[
    Abstractions and Models are NOT at WAR!
                                            - Cobwoy's Talisman
    But Abstractions don't care and Models can't understand!!
                                            - Lul, Practicality
 ]]
cmake_minimum_required(VERSION 3.4)

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

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${REPOSITORYROOT}/KarmaBin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${REPOSITORYROOT}/KarmaBin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${REPOSITORYROOT}/KarmaBin")

# Finally we specify policies, if any
# This one is for inter-directory linking
cmake_policy(SET CMP0079 NEW)

# Setting some options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

#----------------------------------------------------------------------------#

# Set Engine project
project(KarmaBasedProjectName VERSION 1.0)

# Then we specify the involved libraries
add_subdirectory(Karma)

# Here we define essential libraries for proofing subsequent client building
list(APPEND ESSENTIAL_LIBS Karma)

message(STATUS "Libraries list ${ESSENTIAL_LIBS}")

# Build the Client
add_subdirectory(Application)

# Relevant linking of finished Application with the Engine!
target_link_libraries(SandBox PUBLIC KarmaEngine)

# PostBuild processes, experimental now
#add_custom_command(TARGET KarmaEngine POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#    "$(TargetPath)"
#    "$(ProjectDir)$(Platform)\\$(Configuration)" 
#)

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