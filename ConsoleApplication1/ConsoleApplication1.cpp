// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <type_traits>
#include <map>
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
};

////////////////////////////////////////////////////////////////////////

class PacketMethodFactory
{
public:
    using TCreateMethod = unique_ptr<ICompressionMethod>(*)();

public:
    PacketMethodFactory() = delete;

    static bool Register(const string& name, TCreateMethod funcCreate);
    static bool Register(U8 type, U8 subType, TCreateMethod funcCreate);

    static unique_ptr<ICompressionMethod> Create(const string& name);
    static unique_ptr<ICompressionMethod> Create(U8 type, U8 subType);

private:
    static map<string, TCreateMethod> s_methods;
    static map<pair<U8, U8>, TCreateMethod> s_creatorMethods;
};

////////////////////////////////////////////////////////////////////////

map<string, PacketMethodFactory::TCreateMethod> PacketMethodFactory::s_methods;
map<pair<U8, U8>, PacketMethodFactory::TCreateMethod> PacketMethodFactory::s_creatorMethods;

////////////////////////////////////////////////////////////////////////

bool PacketMethodFactory::Register(const string& name, PacketMethodFactory::TCreateMethod funcCreate)
{
    if (auto it = s_methods.find(name); it == s_methods.end())
    { // C++17 init-if ^^
        s_methods[name] = funcCreate;
        return true;
    }
    return false;
}

bool PacketMethodFactory::Register(U8 type, U8 subType, TCreateMethod funcCreate)
{
    auto matchPair = pair<U8, U8>(type, subType);
    if(auto it = s_creatorMethods.find(matchPair); it == s_creatorMethods.end())
    { // C++17 init-if ^^
        s_creatorMethods[matchPair] = funcCreate;
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////

class ZipCompression : public ICompressionMethod
{
public:
    virtual void Compress() override
    {
        cout << "Zip" << endl;
    }

    static unique_ptr<ICompressionMethod> CreateMethod() {
        return make_unique<ZipCompression>();
    }
    static std::string GetFactoryName() { return "ZIP"; }
    static U8 Type() { return 0; }
    static U8 SubType() { return 1; }

private:
    static bool s_registered;
    static bool s_typeRegistered;
};

////////////////////////////////////////////////////////////////////////

unique_ptr<ICompressionMethod>
PacketMethodFactory::Create(const string& name)
{
    if (auto it = s_methods.find(name); it != s_methods.end())
        return it->second(); // call the createFunc

    return nullptr;
}
////////////////////////////////////////////////////////////////////////

unique_ptr<ICompressionMethod>
PacketMethodFactory::Create(U8 type, U8 subType)
{
    if (auto it = s_creatorMethods.find(pair<U8, U8>(type, subType)); it != s_creatorMethods.end())
        return it->second(); // call the createFunc
    return nullptr;
}

////////////////////////////////////////////////////////////////////////

bool ZipCompression::s_registered = PacketMethodFactory::Register(ZipCompression::GetFactoryName(), ZipCompression::CreateMethod);
bool ZipCompression::s_typeRegistered = PacketMethodFactory::Register(ZipCompression::Type(), ZipCompression::SubType(), ZipCompression::CreateMethod);

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

int main()
{
    
    auto method = PacketMethodFactory::Create("ZIP");
    method.get()->Compress();

    auto method2 = PacketMethodFactory::Create(0, 1);
    method2.get()->Compress();


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
