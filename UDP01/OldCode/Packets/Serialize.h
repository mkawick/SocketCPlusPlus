// Serialize.h
#pragma once

#include "../DataTypes.h"
#include "../ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <Winsock2.h>

U64 htonll(U64 value); // used for endianness
U64 ntohll(U64 value);

// warning about loss of data for UINT 64 which does not apply
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif

/*#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#if defined ANDROID
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#endif*/

#include <string>
using namespace std;


////////////////////////////////////////////////////////

namespace Serialize
{
    // based on this here:
    // https://stackoverflow.com/questions/257288/templated-check-for-the-existence-of-a-class-member-function

   // Serialization 
   template<typename T>
   struct IsSelfSerializableIn
   {
        template< typename U, bool (U::*)( const U8*, int&, int networkVersion) > struct signature;
        template< typename U > static char& HasSerialize(signature< U, &U::SerializeIn >*);
        template< typename U > static int& HasSerialize(...);
        static const bool value = sizeof(char) == sizeof(HasSerialize<T>(0));
   };

   ////////////////////////////////////////////////////////////////
   //                       primitive serialization
   ////////////////////////////////////////////////////////////////
   template< typename T, bool hasSerializer = IsSelfSerializableIn<T>::value >
   struct Serializer
   {
       static inline void In(const U8* source, int& offset, T& value, int minorVersion)
       {
           int size = sizeof(T);
           memcpy(&value, source + offset, size);
           offset += size;
       }
       //----------------------------------
       static inline void Out(U8* destination, int& offset, const T& value, int minorVersion)
       {
           int size = sizeof(value);
           memcpy(destination + offset, &value, size);
           offset += size;
       }
   };
   template<>
   struct Serializer< U8, false >
   {
       static inline void In(const U8* source, int& offset, U8& value, int minorVersion)
       {
           value = *(source+offset);
           offset += sizeof(value);
       }
       static inline void Out(U8* destination, int& offset, const U8& value, int minorVersion)
       {
           *(destination + offset) = value;
           offset += sizeof(value);
       }
   };
   template<>
   struct Serializer< U16, false >
   {
       static inline void In(const U8* source, int& offset, U16& value, int minorVersion)
       {
           value = ntohs(*(U16*)(source + offset));
           offset += sizeof(value);
       }
       static inline void Out(U8* destination, int& offset, const U16& value, int minorVersion)
       {
           *((U16*)(destination + offset)) = htons(value);
           offset += sizeof(value);
       }
   };
   template<>
   struct Serializer< U32, false >
   {
       static inline void In(const U8* source, int& offset, U32& value, int minorVersion)
       {
           value = ntohl (*(U32*)(source + offset));
           offset += sizeof(value);
       }
       static inline void Out(U8* destination, int& offset, const U32& value, int minorVersion)
       {
           *((U32*)(destination + offset)) = htonl(value);
           offset += sizeof(value);
       }
   };
   template<>
   struct Serializer< int, false >
   {
       static inline void In(const U8* source, int& offset, int& value, int minorVersion)
       {
           value = ntohl(*(U32*)(source + offset));
           offset += sizeof(value);
       }
       static inline void Out(U8* destination, int& offset, const int& value, int minorVersion)
       {
           *((U32*)(destination + offset)) = htonl(value);
           offset += sizeof(value);
       }
   };
   template<>
   struct Serializer< U64, false >
   {
       static inline void In(const U8* source, int& offset, U64& value, int minorVersion)
       {
           value = ntohll (*(U64*)(source + offset));
           offset += sizeof(value);
       }
       static inline void Out(U8* destination, int& offset, const U64& value, int minorVersion)
       {
           *((U64*)(destination + offset)) = htonll(value);
           offset += sizeof(value);
       }
   };

   ////////////////////////////////////////////////////////////////
   //                       class serialization
   ////////////////////////////////////////////////////////////////
   template< typename T >
   struct Serializer< T, true >
   {
	   static inline void In( const U8* destination, int& offset, T& value, int minorVersion )
	   {
		   value.SerializeIn( destination, offset, minorVersion );
	   }
      static inline void Out( U8* destination, int& offset, const T& value, int minorVersion )
	   {
		   value.SerializeOut( destination, offset, minorVersion );
	   }
   };
   ////////////////////////////////////////////////////////////////
   //                       bool serialization
   ////////////////////////////////////////////////////////////////
    template<>
    struct Serializer< bool, false >
    {
        static inline void In( const U8* source, int& offset, bool& outValue, int minorVersion )
        {
            U8 tempValue = *(source + offset);
            outValue = tempValue ? true:false;
            offset += sizeof(tempValue);
        }
        static inline void Out( U8* destination, int& offset, const bool& value, int minorVersion )
        {
            U8 finalValue = value ? 1:0;
            *(destination + offset) = finalValue;
            offset += sizeof(finalValue);
        }
    };
    ////////////////////////////////////////////////////////////////
    //          global entry points for serialization
    ////////////////////////////////////////////////////////////////
    template<typename T>
    inline void In(const U8* source, int& offset, T& value, int minorVersion)
    {
        Serializer<T>::In(source, offset, value, minorVersion);
    }
    template<typename T>
    inline void Out(U8* dest, int& offset, const T& value, int minorVersion)
    {
        Serializer<T>::Out(dest, offset, value, minorVersion);
    }
    

    template<typename T, int N>
    inline void In(const U8* source, int& offset, T (&value)[N], int minorVersion)
    {
        for (auto& i : value)
        {
            In(source, offset, i, minorVersion);
        }
    }
    template<typename T, int N>
    inline void Out(U8* dest, int& offset, const T (&value)[N], int minorVersion)
    {
        for (auto& i : value)
        {
            Out(dest, offset, i, minorVersion);
        }
    }

   ////////////////////////////////////////////////////////////////
   //                       string serialization
   ////////////////////////////////////////////////////////////////
   template<>
   struct Serializer< std::string, false >
   {
	   static inline void In( const U8* source, int& offset, std::string& value, int minorVersion )
	   {
		   U16 len = 0;// there should be a maximum size to strings
            Serialize::In(source, offset, len, minorVersion);

            if( len == 0 )
            {
                value.clear();
                return;
            }
            char buffer[MaxString];
            if (len > MaxString -1)
                len = MaxString - 1;
            memcpy( buffer, source + offset, len );
            buffer[ len ] = 0;
            value = buffer;
            offset += len;
	   }
      static inline void Out( U8* destination, int& offset, const std::string& value, int minorVersion )
	   {
            U16 len = static_cast< U16 >( value.size() );// there should be a maximum size to strings
            Serialize::Out(destination, offset, len, minorVersion);

            if( len == 0 )
                return;

            memcpy( destination + offset, value.c_str(), len );// now the actual string
            offset += len;
	   }
   };

   

} // namespace Serialize

////////////////////////////////////////////////////////

#if PLATFORM == PLATFORM_WINDOWS 
#pragma warning( pop )// warning about loss of data for UINT 64 which does not apply
#endif