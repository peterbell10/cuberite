project (Cuberite)

include_directories ("${PROJECT_SOURCE_DIR}/../")

SET (SRCS
	Authenticator.cpp
	ChunkDataSerializer.cpp
	MojangAPI.cpp
	Packetizer.cpp
	Protocol_1_8.cpp
	Protocol_1_9.cpp
	Protocol_1_10.cpp
	Protocol_1_11.cpp
	Protocol_1_12.cpp
	ProtocolRecognizer.cpp
)

SET (HDRS
	Authenticator.h
	ChunkDataSerializer.h
	MojangAPI.h
	Packetizer.h
	Protocol.h
	Protocol_1_8.h
	Protocol_1_9.h
	Protocol_1_10.h
	Protocol_1_11.h
	Protocol_1_12.h
	ProtocolRecognizer.h
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set_source_files_properties(Protocol_1_9.cpp  PROPERTIES COMPILE_FLAGS "-Wno-error=switch")
	set_source_files_properties(Protocol_1_8.cpp  PROPERTIES COMPILE_FLAGS "-Wno-error=switch")
	set_source_files_properties(Protocol_1_10.cpp PROPERTIES COMPILE_FLAGS "-Wno-error=switch")
	set_source_files_properties(Protocol_1_11.cpp PROPERTIES COMPILE_FLAGS "-Wno-error=switch")
	set_source_files_properties(Protocol_1_12.cpp PROPERTIES COMPILE_FLAGS "-Wno-error=switch")
endif()

if (NOT MSVC)
	add_library(Protocol ${SRCS} ${HDRS})
	
	# Precompiled header support
	include(cotire)
	set_target_properties(Protocol PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "../Globals.h")
	cotire(Protocol)
endif()
