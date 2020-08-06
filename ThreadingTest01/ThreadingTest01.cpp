// ThreadingTest01.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <mutex>

#include <Windows.h>
#include <conio.h>
//#include <synchapi.h>
using namespace std;

// std::mutex mu;
void thread_function()
{
    int key = 0;
    while (key != '2')
    {
        std::cout << ".2.";
        Sleep(10);
        if (_kbhit())
        {
            key = _getch();
        }
    }
}

class MyThreadClass
{
public:
    void operator()() {
        int key = 0;
        while (key != '3')
        {
            std::cout << "-3-";
            Sleep(10);
            if (_kbhit())
            {
                key = _getch();
            }
        }
    }
};

int main()
{
    std::cout << "main thread\n";
    std::thread t(&thread_function);
    
    
    t.detach();

    std::shared_ptr< MyThreadClass> threader;;
    std::shared_ptr<std::thread> t2(new thread(*threader));
    t2->detach();
    int key = 0;
    while (key != 27)
    {
        Sleep(10);
        cout << "1";
        if (_kbhit())
        {
            key = _getch();
        }
    }
    // t.join();
    t2.reset();
    return 0;
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
