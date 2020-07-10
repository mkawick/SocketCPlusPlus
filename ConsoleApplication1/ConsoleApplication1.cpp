// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <type_traits>
#include <map>
using namespace std;

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

class CompressionMethodFactory
{
public:
    using TCreateMethod = unique_ptr<ICompressionMethod>(*)();

public:
    CompressionMethodFactory() = delete;

    static bool Register(const string& name, TCreateMethod funcCreate);

    static unique_ptr<ICompressionMethod> Create(const string& name);

private:
    static map<string, TCreateMethod> s_methods;
};

////////////////////////////////////////////////////////////////////////

map<string, CompressionMethodFactory::TCreateMethod> CompressionMethodFactory::s_methods;

////////////////////////////////////////////////////////////////////////

bool CompressionMethodFactory::Register(const string& name, CompressionMethodFactory::TCreateMethod funcCreate)
{
    if (auto it = s_methods.find(name); it == s_methods.end())
    { // C++17 init-if ^^
        s_methods[name] = funcCreate;
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

private:
    static bool s_registered;
};

////////////////////////////////////////////////////////////////////////

unique_ptr<ICompressionMethod>
CompressionMethodFactory::Create(const string& name)
{
    if (auto it = s_methods.find(name); it != s_methods.end())
        return it->second(); // call the createFunc

    return nullptr;
}

////////////////////////////////////////////////////////////////////////

bool ZipCompression::s_registered = CompressionMethodFactory::Register(ZipCompression::GetFactoryName(), ZipCompression::CreateMethod);

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

int main()
{
    
    auto method = CompressionMethodFactory::Create("ZIP");
    method.get()->Compress();


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
