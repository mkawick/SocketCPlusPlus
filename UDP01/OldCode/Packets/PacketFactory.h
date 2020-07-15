#pragma once
// PacketFactory.h
#include <type_traits>
#include <map>
#include "../Packets/BasePacket.h"
#include "../General/CircularBuffer.h"

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class PacketMethodFactory
{
public:
    using TCreateMethod = unique_ptr<IPacketSerializable>(*)();

public:
    PacketMethodFactory() = delete;

    static void InitFactory();
    static void Shutdown();

    static unique_ptr<IPacketSerializable> Create(const string& name);
    static unique_ptr<IPacketSerializable> Create(U8 type, U8 subType);

    //---------------------------------------------------------------
    static bool Release(unique_ptr <IPacketSerializable>& data);


private:
    static map<string, TCreateMethod> s_methods;
    static map<pair<U8, U8>, TCreateMethod> s_allocationMethods;
    static map<pair<U8, U8>, circular_buffer<unique_ptr <IPacketSerializable>>* > s_creationPool;

    static unique_ptr <IPacketSerializable> Allocate(U8 type, U8 subType);

    static bool IsPoolAlreadyAllocated(const pair<U8, U8>& typePair);


public:
    static bool Register(const string& name, U8 type, U8 subType, TCreateMethod funcCreate);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/*
unique_ptr<IPacketSerializable>
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

unique_ptr <IPacketSerializable>
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

unique_ptr <IPacketSerializable>
PacketMethodFactory::Allocate(U8 type, U8 subType)
{
    if (auto it = s_allocationMethods.find(pair<U8, U8>(type, subType)); it != s_allocationMethods.end())
    {
        auto alloc = it->second();// call the createFunc
        return alloc;
    }
    return nullptr;
}*/
