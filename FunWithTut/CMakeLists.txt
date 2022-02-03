cmake_minimum_required(VERSION 3.4)

# set the project name and version
project(Karma VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# options!
  # making math library optional
option(USE_MYMATH "Use tutorial provided math implementation" ON)


# a header to pass the version number to the source code
configure_file(TutorialConfig.h.in TutorialConfig.h)


# What libraries to be linked
if(USE_MYMATH)
  add_subdirectory(MathFunctions)
  list(APPEND EXTRA_LIBS MathFunctions)
  #Since we have declared and thus made MathFunctions CMakeLists as an INTERFACE
  # we can remove extra the includes now
  #list(APPEND EXTRA_INCLUDES "${PROJECT_SOURCE_DIR}/MathFunctions")
endif()


# add the executable (the Application type and sorts)
add_executable(Karma tutorial.cxx)


# now we link
target_link_libraries(Karma PUBLIC ${EXTRA_LIBS})

# finally we install the static/dynamic libraries
# @param TARGETS     The executable binary name (see the top declaration)
# @param DESTINATION The directory where the binary form of library be copied to
# @param FILES       The files to be copied
install(TARGETS Karma DESTINATION bin)
install(FILES "${PROJECT_BINARY_DIR}/TutorialConfig.h"
  DESTINATION include
  )


# the list of paths to search for include files
target_include_directories(Karma PUBLIC
  "${PROJECT_BINARY_DIR}"
  )
