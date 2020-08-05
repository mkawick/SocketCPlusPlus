// BasePacket.h

#pragma once
#include "./CommonTypes.h"
#include "../NetworkUtils.h"
#include "../General/FixedLengthString.h"

//#define _MEMLEAK_TESTING_

extern const U8   NetworkVersionMajor;
extern const U8   NetworkVersionMinor;

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class IPacketSerializable //: public IBinarySerializable
{
public:
    virtual bool  SerializeIn(const U8* data, int& bufferOffset, int minorVersion) = 0;
    virtual bool  SerializeOut(U8* data, int& bufferOffset, int minorVersion = NetworkVersionMinor) const = 0;

public:
    virtual string GetName() = 0;
    virtual U8 GetType() = 0;
    virtual U8 GetSubType() = 0;
    virtual IPacketSerializable* GetTypePtr() { return this; }
};

///////////////////////////////////////////////////////////////
/*
#ifdef _MEMORY_TEST_
PacketDebug()
{
    m_counter++;
    cout << "BasePacket +count: " << m_counter << endl;
}
virtual ~PacketDebug()
{
    m_counter--;
    cout << "BasePacket ~count: " << m_counter << endl;
}
static int      m_counter;
#else

#endif
*/

/*class IBinarySerializable
{
public:
    virtual bool  SerializeIn(const U8* data, int& bufferOffset, int minorVersion) = 0;
    virtual bool  SerializeOut(U8* data, int& bufferOffset, int minorVersion = NetworkVersionMinor) const = 0;
};*/

///////////////////////////////////////////////////////////////
