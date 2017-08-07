project (Cuberite)

include_directories ("${PROJECT_SOURCE_DIR}/../")

SET (SRCS
	CriticalSection.cpp
	Event.cpp
	File.cpp
	GZipFile.cpp
	HostnameLookup.cpp
	IPLookup.cpp
	IsThread.cpp
	NetworkInterfaceEnum.cpp
	NetworkLookup.cpp
	NetworkSingleton.cpp
	ServerHandleImpl.cpp
	StackTrace.cpp
	TCPLinkImpl.cpp
	UDPEndpointImpl.cpp
	WinStackWalker.cpp
)

SET (HDRS
	CriticalSection.h
	Event.h
	File.h
	GetAddressInfoError.h
	GZipFile.h
	HostnameLookup.h
	IPLookup.h
	IsThread.h
	Network.h
	NetworkLookup.h
	NetworkSingleton.h
	Queue.h
	ServerHandleImpl.h
	StackTrace.h
	TCPLinkImpl.h
	UDPEndpointImpl.h
	WinStackWalker.h
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	add_flags_cxx("-Wno-error=global-constructors ")
endif()

if(NOT MSVC)
	add_library(OSSupport ${SRCS} ${HDRS})
	
	# Precompiled header support
	include(cotire)
	set_target_properties(OSSupport PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "../Globals.h")
	cotire(OSSupport)
endif()
