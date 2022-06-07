# For adding multiple files
list(APPEND GladSource src/glad.c)

#Use SHARED for .so/.dll extentions
add_library(Glad STATIC ${GladSource})

# INTERFACE (Something that Consumers require but Producers don't)
# Since I am a library coder here, I am the Producer
# Rest of the Engine is Consumer here

target_include_directories(Glad
    PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(Glad PUBLIC Glad)