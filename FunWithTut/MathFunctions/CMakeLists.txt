add_library(MathFunctions mysqrt.cxx)

# INTERFACE (Something that Consumers require but Producers don't)
# Since I am a library coder here, I am the Producer
# Rest of the Engine is Consumer here
target_include_directories(
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
	)