// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <type_traits>
#include <map>
#include "../UDP01/OldCode/General/CircularBuffer.h"
using namespace std;
typedef unsigned char U8;

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

class ICompressionMethod
{
public:
    ICompressionMethod() = default;
    virtual ~ICompressionMethod() = default;

    virtual void Compress() = 0;

public: // all part of the factory interface

    virtual string GetName() = 0;
    virtual U8 GetType() = 0;
    virtual U8 GetSubType() = 0;
};

////////////////////////////////////////////////////////////////////////

class PacketMethodFactory
{
public:
    using TCreateMethod = unique_ptr<ICompressionMethod>(*)();

public:
    PacketMethodFactory() = delete;

    static void Init();
    static void Shutdown();

    static unique_ptr<ICompressionMethod> Create(const string& name);
    static unique_ptr <ICompressionMethod> Create(U8 type, U8 subType);

    //---------------------------------------------------------------
    static bool Release(unique_ptr <ICompressionMethod>& data);


private:
    static map<string, TCreateMethod> s_methods;
    static map<pair<U8, U8>, TCreateMethod> s_allocationMethods;
    static map<pair<U8, U8>, circular_buffer<unique_ptr <ICompressionMethod>>* > s_creationPool;

    static unique_ptr <ICompressionMethod> Allocate(U8 type, U8 subType);

public:
    static bool Register(const string& name, U8 type, U8 subType, TCreateMethod funcCreate);
};

////////////////////////////////////////////////////////////////////////

map<string, PacketMethodFactory::TCreateMethod> PacketMethodFactory::s_methods;
map<pair<U8, U8>, PacketMethodFactory::TCreateMethod> PacketMethodFactory::s_allocationMethods;
map<pair<U8, U8>, circular_buffer<unique_ptr <ICompressionMethod>>* > PacketMethodFactory::s_creationPool;

////////////////////////////////////////////////////////////////////////

void PacketMethodFactory::Init()
{
    for (auto it = s_allocationMethods.begin(); it != s_allocationMethods.end(); it++)
    {
        auto typePair = it->first;
        int numToCreate = 100;

        auto buff = new circular_buffer< unique_ptr <ICompressionMethod>>(numToCreate);

        for (int i = 0; i < numToCreate; i++)
        {
            auto pointer = PacketMethodFactory::Allocate(typePair.first, typePair.second);
            buff->put(std::move(pointer));
        }
        s_creationPool[typePair] = buff;
    }
}

void PacketMethodFactory::Shutdown()
{
    for (auto it = s_creationPool.begin(); it != s_creationPool.end(); it++)
    {
        delete it->second;
    }
}

bool PacketMethodFactory::Register(const string& name, U8 type, U8 subType, TCreateMethod funcCreate)
{
    bool success = false;
    auto matchPair = pair<U8, U8>(type, subType);
    if(auto it = s_allocationMethods.find(matchPair); it == s_allocationMethods.end())
    { // C++17 init-if ^^
        s_allocationMethods[matchPair] = funcCreate;
        success = true;
    }
    if (auto it = s_methods.find(name); it == s_methods.end())
    { // C++17 init-if ^^
        s_methods[name] = funcCreate;
        success = true;
    }
    return success;
}

////////////////////////////////////////////////////////////////////////

unique_ptr<ICompressionMethod>
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

unique_ptr <ICompressionMethod>
PacketMethodFactory::Create(U8 type, U8 subType)
{
    if (auto it = s_creationPool.find(pair<U8, U8>(type, subType)); it != s_creationPool.end())
    {
        return it->second->get();// find open pointer
    }

    _ASSERT(0, "could not match the requested type");
    return nullptr;
}
////////////////////////////////////////////////////////////////////////

unique_ptr <ICompressionMethod>
PacketMethodFactory::Allocate(U8 type, U8 subType)
{
    if (auto it = s_allocationMethods.find(pair<U8, U8>(type, subType)); it != s_allocationMethods.end())
    {
        auto alloc = it->second();// call the createFunc
        return alloc;
    }
    return nullptr;
}

bool PacketMethodFactory::Release(unique_ptr <ICompressionMethod>& data)
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
////////////////////////////////////////////////////////////////////////

class ZipCompression : public ICompressionMethod
{
public:
    virtual void Compress() override
    {
        cout << "Zip" << endl;
    }

private:  // <Boilerplate that every class will need to implement> //
    string GetName() override { return GetFactoryName(); }
    U8 GetType() override { return Type(); }
    U8 GetSubType() override { return SubType(); }

    static unique_ptr<ICompressionMethod> CreateMethod() {
        return make_unique<ZipCompression>();
    }
    static string GetFactoryName() { return "ZIP"; }
    static U8 Type() { return 0; }
    static U8 SubType() { return 1; }

    static bool s_typeRegistered;
};

////////////////////////////////////////////////////////////////////////

bool ZipCompression::s_typeRegistered = PacketMethodFactory::Register(ZipCompression::GetFactoryName(), ZipCompression::Type(), ZipCompression::SubType(), ZipCompression::CreateMethod);

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

int main()
{
    string ret = "class BasePacket";
    string search("class ");
    size_t position = ret.find(search, 0);
    if (position != string::npos)
        ret.erase(position, search.length());

    PacketMethodFactory::Init();

    auto method = PacketMethodFactory::Create("ZIP");
    method.get()->Compress();

    auto method2 = PacketMethodFactory::Create(0, 1);
    method2.get()->Compress();

    PacketMethodFactory::Release(method);
    PacketMethodFactory::Release(method2);

    PacketMethodFactory::Shutdown();
    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
