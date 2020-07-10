#pragma once

#include "./PacketFactory.h"
#include "BasePacket.h"
#include "MovementPacket.h"
#include "../General/CircularBuffer.h"
#include <hash_map>
using namespace std;

PacketFactory::PacketFactory() 
{
    hash_map<BasePacket::SubType, circular_buffer<BasePacket>*> mp;
    hash_map<int, circular_buffer<BasePacket>*> mp;
    circular_buffer<BasePacket>* bpBuffer = new circular_buffer<BasePacket>(100);
}

template< typename PacketType >
PacketType* SerializeIn(const U8* bufferIn, int& bufferOffset, int networkMinorVersion)
{
    PacketType* newPacket = new PacketType();
    newPacket->SerializeIn(bufferIn, bufferOffset, networkMinorVersion);

    return newPacket;
}

template< typename PacketType >
PacketType* CreatePacket()
{
    return new PacketType();
}

//-----------------------------------------------------------------------------------------

bool	PacketFactory::Parse(const U8* bufferIn, int& bufferOffset, BasePacket** packetOut, int networkMinorVersion) const
{
    *packetOut = NULL;// just to be sure that no one misuses this

    BasePacket firstPassParse;
    int offset = bufferOffset;
    firstPassParse.SerializeIn(bufferIn, offset, networkMinorVersion);
    bool success = Create(firstPassParse.packetType, firstPassParse.packetSubType, packetOut);

    /*  switch( firstPassParse.packetType )
      {
      case PacketType_Base:
         {
            success = ParseBasePacket( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Login:
         {
            success = ParseLogin( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Chat:
         {
            success = ParseChat( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_UserInfo:
         {
            success = ParseUserInfo( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Contact:
         {
            success = ParseContact( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Asset:
         {
            success = ParseAsset( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_DbQuery:
         {
            success = ParseDbQuery( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_ServerToServerWrapper:
         {
            success = ParseServerToServerWrapper( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_ServerInformation:
         {
            success = ParseServerInfo( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_GatewayWrapper:
         {
            success = ParseGatewayWrapper( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Gameplay:
         {
            success = ParseGame( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_ErrorReport:
         {
            // put here just to avoid a function call for only one type.
            PacketErrorReport* error = new PacketErrorReport();
            //error->SerializeIn( bufferIn, bufferOffset, networkMinorVersion );
            *packetOut = error;
            success = true;
         }
      case PacketType_Cheat:
         {
            success = ParseCheat( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Purchase:
         {
            success = ParsePurchase( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Analytics:
         {
            success = ParseAnalytics( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Tournament:
         {
            success = ParseTournament( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_UserStats:
         {
            success = ParseUserStats( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Notification:
         {
            success = ParseNotification( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      case PacketType_Invitation:
         {
            success = ParseInvitation( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
         }
      }*/

    if (success && *packetOut)
    {
        (*packetOut)->SerializeIn(bufferIn, bufferOffset, networkMinorVersion);
    }

    return success;
}// be sure to check the return value