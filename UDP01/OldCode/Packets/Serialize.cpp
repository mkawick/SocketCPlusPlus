// Serialize.cpp

#include <iostream>
#include "../DataTypes.h"

#if PLATFORM == PLATFORM_WINDOWS

#include <winsock.h>
// warning about loss of data for UINT 64 which does not apply
#pragma warning( push )
#pragma warning( disable : 4244 )

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <sys/types.h>
#include <netinet/in.h>
#endif

#if PLATFORM == PLATFORM_WINDOWS
#ifndef htonll
U64 htonll( U64 value ) 
{
    int num = 42;
    if (*(char *)&num == 42) 
    {
        U32 high_part = htonl((U32)(value >> 32));
        U32 low_part = htonl((U32)(value & 0xFFFFFFFFLL));
        return (((U64)low_part) << 32) | high_part;
    } 
    else 
    {
        return value;
    }
}
#endif /* htonll */

#ifndef ntohll
U64 ntohll( U64 value ) 
{
   return htonll( value );
}

#endif

#endif