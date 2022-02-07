cmake_minimum_required(VERSION 3.4)

# Here is the begining of implementing our learning of build system
# from official CMake tutorial to our Engine Karma!

# set the project name and version
project(KARMA VERSION 1.0) # Versioning shall change to something more appropriate

# First we specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# First we will try compiling Glad in Linux, switching to one level deep in Karma
add_subdirectory(vendor/Glad)
# list(APPEND ESSENTIAL_LIBS Glad)

# Add the executable, no not here, we just want static library, but then again, what do I know
# add_executable(Karma)


# Linking
# target_link_libraries(Karma PUBLIC ${ESSENTIAL_LIBS})

