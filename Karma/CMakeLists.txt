# Macros
MACRO(COLLECTIVEINCLUDES curdir)
  FILE(GLOB_RECURSE children LIST_DIRECTORIES true RELATIVE ${curdir} ${curdir}/*)
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      message(STATUS "foo include dir: ${curdir}/${child}")
      target_include_directories(Karma
        PRIVATE
        ${curdir}/${child}
        PUBLIC
        $<BUILD_INTERFACE:${curdir}/${child}>
        )
    ENDIF()
  ENDFOREACH()
ENDMACRO()

# Set the project name and version
project(Karma VERSION 1.0) # Versioning shall change to something more appropriate


#find_package(Vulkan REQUIRED)
#find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# Involved libraries and essentials
add_subdirectory(vendor/Glad)
add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/GLM)
add_subdirectory(vendor/ImGui)

list(APPEND ESSENTIAL_LIBS Glad)
list(APPEND ESSENTIAL_LIBS GLFW)
list(APPEND ESSENTIAL_LIBS glm)
list(APPEND ESSENTIAL_LIBS ImGui)


# Karma's personal code
file(GLOB_RECURSE  CPPFILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
# Shows the headerfile directory in project
#file(GLOB_RECURSE HEADERFILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)


# Building the project (STATIC or DYNAMIC (SHARED))
if (BUILD_SHARED_LIBS)
    add_library(Karma SHARED ${CPPFILES} ${HEADERFILES})
else()
    add_library(Karma STATIC ${CPPFILES} ${HEADERFILES})
endif()

# Linking and Interfacing and Definition for Upper Linking 
target_link_libraries(Karma PUBLIC Glad)
target_link_libraries(Karma PUBLIC glfw)
target_link_libraries(Karma PUBLIC glm)
target_link_libraries(Karma PUBLIC ImGui)

target_include_directories(Karma
    PRIVATE
        # where the library itself will look for its internal headers
		${CMAKE_CURRENT_SOURCE_DIR}/src
      
		${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include/
    PUBLIC
        # where top-level project will look for the library's public headers
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Include sub-directories includes
COLLECTIVEINCLUDES(${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(Karma PUBLIC Karma)

