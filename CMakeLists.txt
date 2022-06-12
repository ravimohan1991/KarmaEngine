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
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Defines
add_compile_definitions(KR_LINUX_PLATFORM)
#add_compile_definitions(KR_ENABLE_ASSERTS)


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

# Due to stupid property of spdf logging library(is it?) we are putting a jugaad
#file(GLOB_RECURSE SPDFHEADERFILES ${CMAKE_CURRENT_SOURCE_DIR}/Karma/vendor/spdlog/include/*.h)

# Build the Client
add_subdirectory(Application)

# Relevant linking!
target_link_libraries(SandBox PUBLIC KarmaEngine)
