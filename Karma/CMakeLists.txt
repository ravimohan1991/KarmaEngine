# Set the project name and version
project(Karma VERSION 1.0) # Versioning shall change to something more appropriate


#find_package(Vulkan REQUIRED)
#find_package(OpenGL REQUIRED)

# Involved libraries and essentials
add_subdirectory(vendor/Glad)
add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/GLM)
add_subdirectory(vendor/ImGui)

list(APPEND ESSENTIAL_LIBS Glad)
list(APPEND ESSENTIAL_LIBS GLFW)
list(APPEND ESSENTIAL_LIBS glm)
list(APPEND ESSENTIAL_LIBS ImGui)

# Building the project (STATIC or DYNAMIC (SHARED))
if (BUILD_SHARED_LIBS)
    add_library(Karma SHARED Placeholder.cpp)
else()
    add_library(Karma STATIC Placeholder.cpp)
endif()

# Linking and Interfacing and Definition for Upper Linking 
target_link_libraries(Karma PUBLIC Glad)
target_link_libraries(Karma PUBLIC glfw)
target_link_libraries(Karma PUBLIC glm)
target_link_libraries(Karma PUBLIC ImGui)

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

