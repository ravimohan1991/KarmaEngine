# Set the project name and version
project(Karma VERSION 1.0) # Versioning shall change to something more appropriate

# Involved libraries and essentials
add_subdirectory(vendor/Glad)
list(APPEND ESSENTIAL_LIBS Glad)

# Building the project (use SHARED for .dll/.so)
add_library(Karma STATIC Placeholder.cpp)

# Linking and Interfacing and Definition for Upper Linking 
target_link_libraries(Karma PUBLIC Glad)

target_include_directories(Karma
    PRIVATE
        # where the library itself will look for its internal headers
        
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(Karma PUBLIC Karma)

