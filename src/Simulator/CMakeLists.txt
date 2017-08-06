project (Cuberite)

include_directories ("${PROJECT_SOURCE_DIR}/../")

SET (SRCS
	DelayedFluidSimulator.cpp
	FireSimulator.cpp
	FloodyFluidSimulator.cpp
	FluidSimulator.cpp
	SandSimulator.cpp
	Simulator.cpp
	SimulatorManager.cpp
	VanillaFluidSimulator.cpp
	VaporizeFluidSimulator.cpp
)

SET (HDRS
	DelayedFluidSimulator.h
	FireSimulator.h
	FloodyFluidSimulator.h
	FluidSimulator.h
	NoopFluidSimulator.h
	NoopRedstoneSimulator.h
	RedstoneSimulator.h
	SandSimulator.h
	Simulator.h
	SimulatorManager.h
	VanillaFluidSimulator.h
	VaporizeFluidSimulator.h
)


if(NOT MSVC)
	add_library(Simulator ${SRCS} ${HDRS})
	
	# Precompiled header support
	include(cotire)
	set_target_properties(Simulator PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "../Globals.h")
	cotire(Simulator)
endif()
