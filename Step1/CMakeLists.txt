cmake_minimum_required(VERSION 3.4)

# set the project name and version
project(Karma VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# a header to pass the version number to the source code
configure_file(TutorialConfig.h.in TutorialConfig.h)

# add the executable
add_executable(Karma tutorial.cxx)

# the list of paths to search for include files
target_include_directories(Karma PUBLIC
  "${PROJECT_BINARY_DIR}"
  )



