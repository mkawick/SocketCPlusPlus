// BasePacket.cpp


#include "../ServerConstants.h"
#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "BasePacket.h"
#include "Serialize.h"
#include "PacketFactory.h"
//#include "PacketFactory.h"
#include <assert.h>


const U8   NetworkVersionMajor = 51;
const U8   NetworkVersionMinor = 0;

//#include <boost/static_assert.hpp>
//BOOST_STATIC_ASSERT( NetworkVersionMajor < (1<<5) );// 5 bits for major
//BOOST_STATIC_ASSERT( NetworkVersionMinor < (1<<3) );

#ifdef _MEMORY_TEST_
int BasePacket::m_counter = 0;
#endif

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

const char* GetPacketTypename(PacketType type)
{
    switch (type)
    {
    case PacketType_Base:
        return "Base";

    case PacketType_Login:
        return "Login";

    case PacketType_ServerTick:
        return "ServerTick";

    case PacketType_NetworkProtocol:
        return "NetworkProtocol";

    case PacketType_Chat:
        return "Chat";

    case PacketType_UserInfo:
        return "UserInfo";

    case PacketType_Contact:
        return "Contact";

    case PacketType_Asset:
        return "Asset";

    case PacketType_UserStateChange: // from server to client, usually
        return "User state change";

    case PacketType_DbQuery:
        return "DbQuery";

    case PacketType_Gameplay:
        return "Gameplay";

    case PacketType_GatewayWrapper:
        return "GW wrapper";

    case PacketType_ServerToServerWrapper:
        return "S2S Wrapper";

    case PacketType_ServerJobWrapper:
        return "Server job wrapper";

    case PacketType_ServerInformation:
        return "Server info";

    case PacketType_GatewayInformation: // user logged out, prepare to shutdown, etc.
        return "GW info";

    case PacketType_ErrorReport:
        return "Error";

    case PacketType_Cheat:
        return "Cheat";

    case PacketType_Purchase:
        return "Purchase";

    case PacketType_Tournament:
        return "Tournament";

    case PacketType_Analytics:
        return "Analytics";

    case PacketType_Notification:
        return "Notification";

    case PacketType_Invitation:
        return "Invitation";

    case PacketType_UserStats:
        return "UserStats";

    default:
        return "";
    }
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

 // compiler forces me to make this function first due to reliance on it later.
constexpr int BasePacket::GetSize()
{
    return sizeof(BasePacket) -
        sizeof(padding) -
        sizeof(long*); // this accounts for the virtual pointer.
}

BasePacket::BasePacket(int packet_type, int packet_sub_type) :
    packetType(packet_type),
    packetSubType(packet_sub_type),
    gameProductId(0),
    versionNumberMajor(NetworkVersionMajor),
    versionNumberMinor(NetworkVersionMinor),
    //packetSize( 0 ),
    gameInstanceId(0)
{
    const int predictedSize = sizeof(packetType) +
                        sizeof(packetSubType) +
                        sizeof(gameProductId) +
                        sizeof(versionNumberMajor) +
                        sizeof(versionNumberMinor) +
                        sizeof(gameInstanceId);

    const int featureSize = GetSize();
    static_assert(GetSize() == predictedSize, "error");// this is meant to enforce maintenence as these values change
}

SelfRegistery(BasePacket)
/*#ifdef _UNIT_TESTING_
bool BasePacket::s_typeRegistered = false;
#else
bool BasePacket::s_typeRegistered = PacketMethodFactory::Register(BasePacket::GetFactoryName(), BasePacket::Type(), BasePacket::SubType(), BasePacket::CreateMethod);
#endif*/

BasePacket::~BasePacket()
{
    gameInstanceId = 0;// for a place to set breakpoints.
}


bool  BasePacket::SerializeIn(const U8* data, int& bufferOffset, int minorVersion)
{
    Serialize::In(data, bufferOffset, packetType, minorVersion);
    Serialize::In(data, bufferOffset, packetSubType, minorVersion);
    Serialize::In(data, bufferOffset, versionNumberMajor, minorVersion);
    Serialize::In(data, bufferOffset, versionNumberMinor, minorVersion);
    Serialize::In(data, bufferOffset, gameProductId, minorVersion);
    //Serialize::In( data, bufferOffset, packetSize ); 
    Serialize::In(data, bufferOffset, gameInstanceId, minorVersion);

    return true;
}

bool  BasePacket::SerializeOut(U8* data, int& bufferOffset, int minorVersion) const
{
    Serialize::Out(data, bufferOffset, packetType, minorVersion);
    Serialize::Out(data, bufferOffset, packetSubType, minorVersion);
    Serialize::Out(data, bufferOffset, versionNumberMajor, minorVersion);
    Serialize::Out(data, bufferOffset, versionNumberMinor, minorVersion);
    Serialize::Out(data, bufferOffset, gameProductId, minorVersion);
    //Serialize::Out( data, bufferOffset, packetSize );
    Serialize::Out(data, bufferOffset, gameInstanceId, minorVersion);

    return true;
}

///////////////////////////////////////////////////////////////

bool  SizePacket::SerializeIn(const U8* data, int& bufferOffset, int minorVersion)
{
    U8 type;
    Serialize::In(data, bufferOffset, type, minorVersion);
    U16 size;
    Serialize::In(data, bufferOffset, size, minorVersion);
    // needs the factory in place to deserialize the next packet
    BasePacket* ptr = new BasePacket();
    ptr->SerializeIn(data, bufferOffset, minorVersion);
    
    return true;
}
bool  SizePacket::SerializeOut(U8* data, int& bufferOffset, int minorVersion) const
{
    Serialize::Out(data, bufferOffset, (U8)PacketType_NetworkProtocol, minorVersion);
    int pos = bufferOffset;
    bufferOffset += sizeof(U16);
    packet->SerializeOut(data, bufferOffset, minorVersion);
    U16 size = bufferOffset - pos;
    Serialize::Out(data, pos, size, minorVersion);

    return true;
}
