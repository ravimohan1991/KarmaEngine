# Macros
MACRO(COLLECTIVEINCLUDES curdir)
  FILE(GLOB_RECURSE children LIST_DIRECTORIES true RELATIVE ${curdir} ${curdir}/*)
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      #message(STATUS "foo include dir: ${curdir}/${child}")
      target_include_directories(KarmaEngine
        PRIVATE
        ${curdir}/${child}
        PUBLIC
        $<BUILD_INTERFACE:${curdir}/${child}>
        )
    ENDIF()
  ENDFOREACH()
ENDMACRO()

#find_package(Vulkan REQUIRED)
#find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

# Involved libraries and essentials
add_subdirectory(vendor/Glad)
add_subdirectory(vendor/GLFW)
add_subdirectory(vendor/GLM)
add_subdirectory(vendor/ImGui)
add_subdirectory(vendor/spdlog)

list(APPEND ESSENTIAL_LIBS Glad)
list(APPEND ESSENTIAL_LIBS GLFW)
list(APPEND ESSENTIAL_LIBS glm)
list(APPEND ESSENTIAL_LIBS ImGui)
list(APPEND ESSENTIAL_LIBS spdlog)

# Set the project name and version
project(KarmaEngine VERSION 1.0) # Versioning shall change to something more appropriate

# Karma's personal code
file(GLOB_RECURSE CPPFILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
# Shows the headerfile directory in project
file(GLOB_RECURSE HEADERFILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# Building the project (STATIC or DYNAMIC (SHARED))
if (BUILD_SHARED_LIBS)
    add_library(KarmaEngine SHARED ${CPPFILES} ${HEADERFILES})
else()
    add_library(KarmaEngine STATIC ${CPPFILES} ${HEADERFILES})
endif()

# Linking to common point
target_link_libraries(KarmaEngine PUBLIC Glad)
target_link_libraries(KarmaEngine PUBLIC glfw)
target_link_libraries(KarmaEngine PUBLIC glm)
target_link_libraries(KarmaEngine PUBLIC ImGui)
target_link_libraries(KarmaEngine PUBLIC spdlogging)

# INTERFACE (Something that Consumers require but Producers don't)
# Since I am a library coder here, I am the Producer
# Rest of the Engine is Consumer here

target_include_directories(KarmaEngine
    PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/include
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Include sub-directories includes
COLLECTIVEINCLUDES(${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(KarmaEngine PUBLIC KarmaEngine)
