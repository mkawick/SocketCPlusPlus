#pragma once

#include <iostream>

#include "PacketFactory.h"
#include "BasePacket.h"
#include "MovementPacket.h"

#include <map>
using namespace std;


////////////////////////////////////////////////////////////////////////

map<string, PacketMethodFactory::TCreateMethod> PacketMethodFactory::s_methods;
map<pair<U8, U8>, PacketMethodFactory::TCreateMethod> PacketMethodFactory::s_allocationMethods;
map<pair<U8, U8>, circular_buffer<shared_ptr <IPacketSerializable>>* > PacketMethodFactory::s_creationPool;

////////////////////////////////////////////////////////////////////////

SelfRegistery(BasePacket)
SelfRegistery(ConnectionCommsPacket)
SelfRegistery(PositionPacket)
SelfRegistery(MovementPacket)

////////////////////////////////////////////////////////////////////////

void PacketMethodFactory::InitFactory()
{
    for (auto it = s_allocationMethods.begin(); it != s_allocationMethods.end(); it++)
    {
        auto typePair = it->first;
        if (IsPoolAlreadyAllocated(typePair))
            continue;

        int numToCreate = 100;

        auto buff = new circular_buffer< shared_ptr <IPacketSerializable>>(numToCreate);

        for (int i = 0; i < numToCreate; i++)
        {
            auto pointer = PacketMethodFactory::Allocate(typePair.first, typePair.second);
            buff->put(std::move(pointer));
        }
        s_creationPool[typePair] = buff;
    }
}

bool PacketMethodFactory::IsPoolAlreadyAllocated(const pair<U8, U8>& typePair)
{
    if (auto it = s_creationPool.find(typePair); it != s_creationPool.end())// make sure that we are not calling init on 
        return true;

    return false;
}

void PacketMethodFactory::Shutdown()
{
    for (auto it = s_creationPool.begin(); it != s_creationPool.end(); it++)
    {
        delete it->second;
    }
    s_methods.clear();
    s_creationPool.clear();
    s_allocationMethods.clear();
}

bool PacketMethodFactory::Register(const string& name, U8 type, U8 subType, TCreateMethod funcCreate)
{
    bool success = false;
    auto matchPair = pair<U8, U8>(type, subType);
    if (auto it = s_allocationMethods.find(matchPair); it == s_allocationMethods.end())
    { 
        s_allocationMethods[matchPair] = funcCreate;
        success = true;
    }
    if (auto it = s_methods.find(name); it == s_methods.end())
    { 
        s_methods[name] = funcCreate;
        success = true;
    }
    return success;
}

////////////////////////////////////////////////////////////////////////


bool PacketMethodFactory::Release(shared_ptr <IPacketSerializable>& data)
{
    U8 type = data.get()->GetType();
    U8 subType = data.get()->GetSubType();
    if (auto it = s_creationPool.find(pair<U8, U8>(type, subType)); it != s_creationPool.end())
    {
        it->second->put(std::move(data));
        return true;
    }

    return false;
}
////////////////////////////////////////////////////////////////////////
shared_ptr<IPacketSerializable>
PacketMethodFactory::Create(const string& name)
{
    if (auto it = s_methods.find(name); it != s_methods.end())
    {
        auto temp = it->second(); // call the createFunc
        U8 type = temp.get()->GetType();
        U8 subType = temp.get()->GetSubType();
        return Create(type, subType); // use the memory pool
    }

    return nullptr;
}
////////////////////////////////////////////////////////////////////////

shared_ptr <IPacketSerializable>
PacketMethodFactory::Create(U8 type, U8 subType)
{
    if (auto it = s_creationPool.find(pair<U8, U8>(type, subType)); it != s_creationPool.end())
    {
        return it->second->get();// find open pointer
    }

    _ASSERT(0);// , "could not match the requested type");
    return nullptr;
}
////////////////////////////////////////////////////////////////////////

shared_ptr <IPacketSerializable>
PacketMethodFactory::Allocate(U8 type, U8 subType)
{
    if (auto it = s_allocationMethods.find(pair<U8, U8>(type, subType)); it != s_allocationMethods.end())
    {
        auto alloc = it->second();// call the createFunc
        return alloc;
    }
    return nullptr;
}
////////////////////////////////////////////////////////////////////////
/*
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


    if (success && *packetOut)
    {
        (*packetOut)->SerializeIn(bufferIn, bufferOffset, networkMinorVersion);
    }

    return success;
}// be sure to check the return value*/