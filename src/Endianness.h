
#pragma once

#ifdef _WIN32
	#include <intrin.h>  // for _byteswap functions
#endif

namespace Detail
{
	inline UInt64 ByteSwap(UInt64 a_Value)
	{
		#ifdef _WIN32
			return _byteswap_uint64(a_Value);
		#else
			return __builtin_bswap64(a_Value);
		#endif
	}

	inline UInt32 ByteSwap(UInt32 a_Value)
	{
		#ifdef _WIN32
			return _byteswap_ulong(a_Value);
		#else
			return __builtin_bswap32(a_Value);
		#endif
	}
	
	inline UInt16 ByteSwap(UInt16 a_Value)
	{
		#ifdef _WIN32
			return _byteswap_ushort(a_Value);
		#else
			return __builtin_bswap16(a_Value);
		#endif
	}

	template <typename T>
	T ByteSwapIfLittleEndian(T a_Value)
	{
		#if !defined(__BYTE_ORDER__) && !defined(_WIN32)
			#error Could not determine host byte order
		#endif

		#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || defined(_WIN32)
			return ByteSwap(a_Value);
		#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
			return a_Value;
		#else
			#error Unsupported byte order
		#endif
	}

	template <typename T>
	T ReadMemory(const void * a_Ptr)
	{
		T Buffer;
		std::memcpy(&Buffer, a_Ptr, sizeof(Buffer));
		return Buffer;
	}
}


inline UInt64 HostToNet(UInt64 a_Value) { return Detail::ByteSwapIfLittleEndian(a_Value); }
inline UInt32 HostToNet(UInt32 a_Value) { return Detail::ByteSwapIfLittleEndian(a_Value); }
inline UInt16 HostToNet(UInt16 a_Value) { return Detail::ByteSwapIfLittleEndian(a_Value); }

inline UInt64 NetToHost(UInt64 a_Value) { return Detail::ByteSwapIfLittleEndian(a_Value); }
inline UInt32 NetToHost(UInt32 a_Value) { return Detail::ByteSwapIfLittleEndian(a_Value); }
inline UInt16 NetToHost(UInt16 a_Value) { return Detail::ByteSwapIfLittleEndian(a_Value); }





inline UInt64 HostToNetwork8(const void * a_Value)
{
	return HostToNet(Detail::ReadMemory<UInt64>(a_Value));
}

inline UInt32 HostToNetwork4(const void * a_Value)
{
	return HostToNet(Detail::ReadMemory<UInt32>(a_Value));
}

inline UInt16 HostToNetwork2(const void * a_Value)
{
	return HostToNet(Detail::ReadMemory<UInt16>(a_Value));
}

inline UInt64 NetworkToHost8(const void * a_Value) { return HostToNetwork8(a_Value); }
inline UInt32 NetworkToHost4(const void * a_Value) { return HostToNetwork4(a_Value); }
inline UInt16 NetworkToHost2(const void * a_Value) { return HostToNetwork2(a_Value); }





inline double NetworkToHostDouble8(const void * a_Value)
{
	static_assert(sizeof(double) == sizeof(UInt64), "");
	UInt64 Buffer = NetworkToHost8(a_Value);
	return Detail::ReadMemory<double>(&Buffer);
}





inline Int64 NetworkToHostLong8(const void * a_Value)
{
	UInt64 Buffer = NetworkToHost8(a_Value);
	return Detail::ReadMemory<Int64>(&Buffer);
}





inline UInt64 NetworkToHostULong8(const void * a_Value)
{
	return NetworkToHost8(a_Value);
}





inline float NetworkToHostFloat4(const void * a_Value)
{
	static_assert(sizeof(float) == sizeof(UInt32), "");
	UInt32 Buffer = NetworkToHost4(a_Value);
	return Detail::ReadMemory<float>(&Buffer);
}




