# add_library(Glad src/glad.c)

list(APPEND GladSource src/glad.c)
add_library(Glad STATIC ${GladSource})


# target_sources(${PROJECT_NAME}
#    PRIVATE
#        src/some.cpp
# )


# message(${CMAKE_CURRENT_SOURCE_DIR}/include)

# INTERFACE (Something that Consumers require but Producers don't)
# Since I am a library coder here, I am the Producer
# Rest of the Engine is Consumer here
# target_include_directories(Glad
#	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
#	)

target_include_directories(Glad
    PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
