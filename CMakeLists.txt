cmake_minimum_required(VERSION 3.4)

# This is the top level CMakeList context having single responsibility of 
# generating enough context for entire Software (Engine + Application).

# First we specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Setting some options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Then we specify the involved libraries
add_subdirectory(Karma)

# Here we define essential libraries for proofing subsequent building
list(APPEND ESSENTIAL_LIBS Karma)


# Set the Client project
project(Application VERSION 1.0)

# Build the Client
add_subdirectory(Application)
