# Finally the runnable file!
add_executable(SandBox src/KarmaApp.cpp ${SPDFHEADERFILES})

target_compile_definitions(SandBox PUBLIC SandBox)

