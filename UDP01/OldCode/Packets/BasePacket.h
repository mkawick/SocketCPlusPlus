// BasePacket.h

#pragma once

// 
#include "./IBinarySerializable.h"
#include "./CommonTypes.h"
#include "../NetworkUtils.h"
#include "../General/FixedLengthString.h"
#include <memory>
//#define _MEMLEAK_TESTING_
using namespace std;

#pragma pack(push,4)



///////////////////////////////////////////////////////////////

// todo, make login, logout, etc into authentication packets
enum PacketType
{
    PacketType_Base,
    PacketType_Login,
    PacketType_ServerTick,
    PacketType_NetworkProtocol,
    PacketType_Chat,
    PacketType_UserInfo,
    PacketType_Contact,
    PacketType_Asset,
    PacketType_UserStateChange, // from server to client, usually
    PacketType_DbQuery,
    PacketType_Gameplay,
    PacketType_GatewayWrapper,
    PacketType_ServerToServerWrapper,
    PacketType_ServerJobWrapper,
    PacketType_ServerInformation,
    PacketType_GatewayInformation, // user logged out, prepare to shutdown, etc.
    PacketType_ErrorReport,
    PacketType_Cheat,
    PacketType_Purchase,
    PacketType_Tournament,
    PacketType_Analytics,
    PacketType_Notification,
    PacketType_Invitation,
    PacketType_UserStats,
    PacketType_Num
}; 

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class IPacketSerializable : public IBinarySerializable
{
public:
    virtual string GetName() = 0;
    virtual U8 GetType() = 0;
    virtual U8 GetSubType() = 0;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class BasePacket : public IPacketSerializable
{
public:
    enum SubType 
    {
        BasePacket_Type,
        BasePacket_Hello,
        BasePacket_CommsHandshake,
        BasePacket_RerouteRequest,
        BasePacket_RerouteRequestResponse,
        BasePacket_QOS,
        BasePacket_TestOnly,
        BasePacket_SizeWrapper
    };
public:
    BasePacket(int packet_type = PacketType_Base, int packet_sub_type = BasePacket_Type);
    virtual ~BasePacket();

    void WriteTo(BasePacket* dest)
    {
        dest->gameProductId = gameProductId;
        dest->versionNumberMajor = versionNumberMajor;
        dest->versionNumberMinor = versionNumberMinor;
        dest->gameInstanceId = gameInstanceId;
    }
    void CopyFrom(const BasePacket& source)
    {
        gameProductId = source.gameProductId;
        versionNumberMajor = source.versionNumberMajor;
        versionNumberMinor = source.versionNumberMinor;
        gameInstanceId = source.gameInstanceId;
    }

    bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
    bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion = NetworkVersionMinor ) const;

    U8       packetType;
    U8       packetSubType;
    U8       gameProductId;
    U8       versionNumberMajor;
    U8       versionNumberMinor;
    U8       padding[1];// this will not be serialized
    U16      gameInstanceId;
    
    //--------------------------------------------------
public: // factory interface
    static constexpr int   GetSize();

    string GetName()    override { return GetFactoryName(); }
    U8 GetType()        override { return Type(); }
    U8 GetSubType()     override { return SubType(); }

    static unique_ptr<IPacketSerializable> CreateMethod();

    static string GetFactoryName() { return from_type<BasePacket>(); }
    static U8 Type() { return PacketType_Base; }
    static U8 SubType() { return BasePacket_Type; }

    static bool s_typeRegistered;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class SizePacket : public IPacketSerializable
{
public:
    //PacketType_NetworkProtocol

    bool  SerializeIn(const U8* data, int& bufferOffset, int minorVersion);
    bool  SerializeOut(U8* data, int& bufferOffset, int minorVersion) const;
    IBinarySerializable* packet;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
