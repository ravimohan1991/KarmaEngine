add_library(MathFunctions mysqrt.cxx)

# INTERFACE (Something that Consumers require but Producers don't)
# Since I am a library coder here, I am the Producer
# Rest of the Engine is Consumer here
target_include_directories(MathFunctions
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	)

# Now we shall install the built libraries
# @param TARGETS     The library name (see the top declaration)
# @param DESTINATION The directory where the binary form of library be copied to
# Need to check if symentics are OS based. The following are working in Linux. 
install(TARGETS MathFunctions DESTINATION lib)

# Install the includes
install(FILES MathFunctions.h DESTINATION include)
