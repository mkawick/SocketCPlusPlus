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
    using TCreateMethod = shared_ptr<IPacketSerializable>(*)();

public:
    PacketMethodFactory() = delete;

    static void InitFactory();
    static void Shutdown();

    static shared_ptr<IPacketSerializable> Create(const string& name);
    static shared_ptr<IPacketSerializable> Create(U8 type, U8 subType);

    //---------------------------------------------------------------
    static bool Release(shared_ptr <IPacketSerializable>& data);


private:
    static map<string, TCreateMethod> s_methods;
    static map<pair<U8, U8>, TCreateMethod> s_allocationMethods;
    static map<pair<U8, U8>, circular_buffer<shared_ptr <IPacketSerializable>>* > s_creationPool;

    static shared_ptr <IPacketSerializable> Allocate(U8 type, U8 subType);

    static bool IsPoolAlreadyAllocated(const pair<U8, U8>& typePair);


public:
    static bool Register(const string& name, U8 type, U8 subType, TCreateMethod funcCreate);
};

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
