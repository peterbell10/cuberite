
// WindowsUndefs.h

// Include after Windows.h to undefine macros that interfere with Cuberite
// Note that libevent headers also include Windows.h interally

#ifdef GetFreeSpace
	#undef GetFreeSpace
#endif

#ifdef DeleteFile
	#undef DeleteFile
#endif
