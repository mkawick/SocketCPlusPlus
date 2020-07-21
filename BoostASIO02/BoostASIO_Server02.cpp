// BoostASIO02.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
//
// server.cpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "../UDP01/OldCode/Socket/socket.h"
#include <conio.h>

int main()
{
    try
    {
        TCPThreader thread;
        thread.Test();

        _getch();

       /* io_context io_context;
        TCPServer server1(io_context);
        UDPServer server2(io_context);
        io_context.run();*/
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}