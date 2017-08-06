project (Cuberite)

include_directories ("${PROJECT_SOURCE_DIR}/../")

SET (SRCS
	Noise.cpp
)

SET (HDRS
	InterpolNoise.h
	Noise.h
	OctavedNoise.h
	RidgedNoise.h
)

if(NOT MSVC)
	add_library(Noise ${SRCS} ${HDRS})
	target_link_libraries(Noise OSSupport)
	
	# Precompiled header support
	include(cotire)
	set_target_properties(Noise PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "../Globals.h")
	cotire(Noise)
endif()
