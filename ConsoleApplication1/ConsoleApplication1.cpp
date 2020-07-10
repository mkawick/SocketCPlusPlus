// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <type_traits>
using namespace std;

typedef unsigned char U8;

template<typename T>
struct IsSelfSerializableIn
{
    template< typename U, bool (U::*)(const U8*, int&, int networkVersion) > struct signature;
    template< typename U > static char& HasSerialize(signature< U, &U::SerializeIn >*);
    template< typename U > static int& HasSerialize(...);
    static const bool value = sizeof(char) == sizeof(HasSerialize<T>(0));
};

/*template <typename T>
struct Data;

template <>
struct Data<int> {
    static void print(int val)
    {
        cout << val << endl;
    }
};

template <typename T, std::size_t N>
struct Data<T[N]> {
    static void print(T val)
    {
        for(auto i: val)
        {
            cout << i << endl;
        }
    }
   // const T(&val)[N];
};

template<typename T>
inline void print(T val)
{
    Data<T>::print(val);
}
template<typename T, std::size_t N>
inline void print(T val)
{
    Data<T, N>::print(val);
}*/

template<typename T>   void print(T& t) { cout << t << endl; }
template<typename T, int N> void print(T(&t)[N]) { for (auto i : t) print(i);  }


int main()
{
    int val1 = 0;
    print(val1);

    int val2 [3] = { 3, 2, 1 };
    //std::cout << std::extent<val2>::value << '\n';
    print(val2);

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
