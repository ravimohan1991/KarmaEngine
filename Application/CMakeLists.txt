#[[
    Abstractions and Models are NOT at WAR!
                                            - Cobwoy's Talisman
    But Abstractions don't care and Models can't understand!!
                                            - Lul, Practicality
 ]]

# Platform specific Defines
if(WIN32)
    add_compile_definitions(KR_WINDOWS_PLATFORM)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(KR_LINUX_PLATFORM)
elseif(APPLE)
    add_compile_definitions(KR_MAC_PLATFORM)
endif()

# Handling MSVC static class members for dynamic linkage. I know!
if(MSVC AND BUILD_SHARED_LIBS)
    if(WIN32)
        # Let Karma handle them
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
        add_compile_definitions(KR_DYNAMIC_LINK)
        # Disable stupid C4251 warnings due to STL's negligence
		# https://docs.microsoft.com/en-us/cpp/build/reference/compiler-option-warning-level?view=msvc-170#remarks
        add_compile_options(/wd4251)
    elseif(APPLE)
        add_compile_definitions(KR_DYNAMIC_LINK KR_BUILD_SO) # May need to find appropriate define name because Apple doesn't generate SO
    endif()
endif()


# Finally the runnable file!
add_executable(SandBox src/KarmaApp.cpp)

# MSVC specific extra mile of likning crap
if(MSVC)
    target_include_directories(SandBox
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../Karma/vendor/spdlog/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../Karma/vendor/GLM
        ${CMAKE_CURRENT_SOURCE_DIR}/../Karma/vendor/assimp/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../Karma/vendor/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/../Karma/vendor/GLFW/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../Karma/src/Karma/
		${CMAKE_CURRENT_SOURCE_DIR}/../Karma/vendor/SDL/include
		${CMAKE_CURRENT_SOURCE_DIR}/../Karma/vendor/ImGui/
		${CMAKE_CURRENT_SOURCE_DIR}/../Karma/vendor/ImGui/backends
    	${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()

# Experimental
    target_include_directories(SandBox
        PRIVATE
        ${Vulkan_INCLUDE_DIR}
    )

target_compile_definitions(SandBox PRIVATE SandBox)


if(MSVC)
    # Set local debugger path
    set_target_properties(SandBox PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${REPOSITORYROOT}/KarmaBin")
elseif(XCODE)
    # Set local debugger path
    set_target_properties(SandBox PROPERTIES
    XCODE_GENERATE_SCHEME TRUE
    XCODE_SCHEME_WORKING_DIRECTORY "${REPOSITORYROOT}/KarmaBin")
endif()

#[[
 *                                          /\
 *                                         / /
 *                                      /\| |
 *                                      | | |/\
 *                                      | | / /
 *                                      | `  /
 *                                      `\  (___
 *                                     _.->  ,-.-.
 *                                  _.'      |  \ \
 *                                 /    _____| 0 |0\
 *                                |    /`    `^-.\.-'`-._
 *                                |   |                  `-._
 *                                |   :                      `.
 *                                \    `._     `-.__         O.'
 *         _.--,                   \     `._     __.^--._O_..-'
 *        `---, `.                  `\     /` ` `
 *             `\ `,                  `\   |
 *              |   :                   ;  |
 *              /    `.              ___|__|___
 *             /       `.           (          )
 *            /    `---.:____...---' `--------`.
 *           /        (         `.      __      `.
 *          |          `---------' _   /  \       \
 *          |    .-.      _._     (_)  `--'        \
 *          |   (   )    /   \                       \
 *           \   `-'     \   /                       ;-._
 *            \           `-'           \           .'   `.
 *            /`.                  `\    `\     _.-'`-.    `.___
 *           |   `-._                `\    `\.-'       `-.   ,--`
 *            \      `--.___        ___`\    \           ||^\\
 *             `._        | ``----''     `.   `\         `'  `
 *                `--;     \  jgs          `.   `.
 *                   //^||^\\               //^||^\\
 *                   '  `'  `               '   '  `
 ]]
