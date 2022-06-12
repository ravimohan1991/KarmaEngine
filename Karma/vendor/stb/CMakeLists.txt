# For adding multiple files
list(APPEND STBC dummy.cpp)

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

# Building the project (STATIC or DYNAMIC (SHARED))
if (BUILD_SHARED_LIBS)
    add_library(stbimage SHARED ${STBC} stb_image.h)
else()
    add_library(stbimage STATIC ${STBC} stb_image.h)
endif()



# INTERFACE (Something that Consumers require but Producers don't)
# Since I am a library coder here, I am the Producer
# Rest of the Engine is Consumer here

target_include_directories(stbimage
    PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_definitions(stbimage PUBLIC stbimage)
