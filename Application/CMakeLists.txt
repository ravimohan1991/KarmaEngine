# Finally the runnable file!
add_executable(Application KarmaApp.cpp)

# Relevant linking!
target_link_libraries(Application PUBLIC Karma)
