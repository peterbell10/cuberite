
#pragma once


// Changes endianness
inline UInt64 HostToNetwork8(const void * a_Value)
{
	UInt64 buf;
	memcpy(&buf, a_Value, sizeof( buf));
	return (((static_cast<UInt64>(htonl(static_cast<UInt32>(buf)))) << 32) + htonl(buf >> 32));
}





inline UInt32 HostToNetwork4(const void * a_Value)
{
	UInt32 buf;
	memcpy(&buf, a_Value, sizeof(buf));
	return htonl(buf);
}





inline UInt16 HostToNetwork2(const void * a_Value)
{
	UInt16 Buf;
	std::memcpy(&Buf, a_Value, sizeof(Buf));
	return htons(Buf);
}





inline UInt64 NetworkToHost8(const void * a_Value)
{
	UInt64 buf;
	memcpy(&buf, a_Value, sizeof(buf));
	return (((static_cast<UInt64>(ntohl(static_cast<UInt32>(buf)))) << 32) + ntohl(buf >> 32));
}





inline UInt32 NetworkToHost4(const void * a_Value)
{
	UInt32 Buf;
	std::memcpy(&Buf, a_Value, sizeof(Buf));
	return ntohl(Buf);
}





inline UInt16 NetworkToHost2(const void * a_Value)
{
	UInt16 Buf;
	std::memcpy(&Buf, a_Value, sizeof(Buf));
	return ntohs(Buf);
}





inline Int64 NetworkToHostLong8(const void * a_Value)
{
	return static_cast<Int64>(NetworkToHost8(a_Value));
}





inline Int32 NetworkToHostInt4(const void * a_Value)
{
	return static_cast<Int32>(NetworkToHost4(a_Value));
}





inline Int32 NetworkToHostShort2(const void * a_Value)
{
	return static_cast<Int32>(NetworkToHost2(a_Value));
}





inline double NetworkToHostDouble8(const void * a_Value)
{
	UInt64 buf = NetworkToHost8(a_Value);
	double x;
	memcpy(&x, &buf, sizeof(double));
	return x;
}





inline float NetworkToHostFloat4(const void * a_Value)
{
	UInt32 buf;
	float x;
	memcpy(&buf, a_Value, 4);
	buf = ntohl(buf);
	memcpy(&x, &buf, sizeof(float));
	return x;
}




