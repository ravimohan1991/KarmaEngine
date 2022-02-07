#[[This is a bracket comment.
It runs until the close bracket.]]

cmake_minimum_required(VERSION 3.4)

# Here is the begining of implementing our learning of build system
# from official CMake tutorial to our Engine Karma!

# set the project name and version
project(KARMA VERSION 1.0) # Versioning shall change to something more appropriate

# First we specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# First we will try compiling Glad in Linux, switching to one level deep in Karma
