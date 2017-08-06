project (Cuberite)

include_directories ("${PROJECT_SOURCE_DIR}/../")

set (SRCS
	IncrementalRedstoneSimulator.cpp
)

set (HDRS
	CommandBlockHandler.h
	DoorHandler.h
	DropSpenserHandler.h
	IncrementalRedstoneSimulator.h
	RedstoneHandler.h
	RedstoneSimulatorChunkData.h
	SolidBlockHandler.h
	RedstoneComparatorHandler.h
	RedstoneRepeaterHandler.h
	RedstoneBlockHandler.h
	RedstoneTorchHandler.h
	RedstoneWireHandler.h
	RedstoneLampHandler.h
	RedstoneToggleHandler.h
	PistonHandler.h
	SmallGateHandler.h
	NoteBlockHandler.h
	TNTHandler.h
	TrappedChestHandler.h
	TripwireHookHandler.h
	PoweredRailHandler.h
	PressurePlateHandler.h
)

if(NOT MSVC)
	add_library(IncrementalRedstoneSimulator ${SRCS} ${HDRS})
	
	# Precompiled header support
	include(cotire)
	set_target_properties(IncrementalRedstoneSimulator PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "../../Globals.h")
	cotire(IncrementalRedstoneSimulator)
endif()

