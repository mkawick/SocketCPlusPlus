// UDP01-ThreadedServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
using namespace std;
#include "../UDP01/OldCode/Packets/CommonTypes.h"
#include "../UDP01/OldCode/DataTypes.h"
//#include "../UDP01/OldCode/ServerConstants.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")

//////////////////////////////////////////////////////////////////

class UDPServer
{
public:
    UDPServer(U16 port)
    {

    }

    void operator()() {// main thread
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

private:
    void Receive()
    {
        int SelectTiming = recvfromTimeOutUDP(ReceivingSocket, 100, 0);

        switch (SelectTiming)
        {
        case 0:
            // Timed out, do whatever you want to handle this situation
            printf("Server : Timeout lor while waiting you bastard client!...\n");
            break;

        case -1:
            // Error occurred, maybe we should display an error message?
           // Need more tweaking here and the recvfromTimeOutUDP()...
            printf("Server: Some error encountered with code number : % ld\n", WSAGetLastError());
            break;

        default:
            {
                while (1)
                {
                    const int           BufLength = 1024;
                    char                ReceiveBuf[1024];
                    SOCKADDR_IN         SenderAddr;
                    int                 SenderAddrSize = sizeof(SenderAddr);
                    // Call recvfrom() to get it then display the received data...
                     int ByteReceived = recvfrom(ReceivingSocket, ReceiveBuf, BufLength,
                        0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
                    if (ByteReceived > 0)
                    {
                        printf("Server: Total Bytes received : %d\n", ByteReceived);
                        printf("Server: The data is %s\\n", ReceiveBuf);
                    }

                    else if (ByteReceived <= 0)
                        printf("Server: Connection closed with error code : %ld\n", WSAGetLastError());
                    else
                        printf("Server: recvfrom() failed with error code : %d\n", WSAGetLastError());
                    // Some info on the sender side
                    getpeername(ReceivingSocket, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
                    printf("Server: Sending IP used : %s\n", ipStr);
                    printf("Server: Sending port used : %d\n", htons(SenderAddr.sin_port));


                }
            }

        }
    }
    void GetSomeInfoOnTheSocket()
    {
        // Some info on the receiver side...
        int addrlen = sizeof(ReceiverAddr);
        getsockname(ReceivingSocket, (SOCKADDR*)&ReceiverAddr, &addrlen);
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(ReceiverAddr.sin_family, (void*)&ReceiverAddr.sin_addr, (PSTR)ipStr, sizeof(ipStr));

        printf("Server: Receiving IP(s) used : % s\n", ipStr);
        printf("Server: Receiving port used : % d\n", htons(ReceiverAddr.sin_port));
        printf("Server: I\'m ready to receive a datagram...\n");
    }

    void CleanupAndExit()
    {
        closesocket(ReceivingSocket); // Close the socket
        WSACleanup(); // Do the clean up
    }
    // utils 
    bool NetworkingInit()
    {
        WSADATA             wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            printf("Server: WSAStartup failed with error % ld\n", WSAGetLastError());
            return false;
        }
        else
            printf("Server: The Winsock DLL status is % s.\n", wsaData.szSystemStatus);
        return true;
    }

    void SetupSocketDetails(SOCKADDR_IN& ReceiverAddr, int port)
    {
        // The IPv4 family
        ReceiverAddr.sin_family = AF_INET;

        // Port no. 5150
        ReceiverAddr.sin_port = htons(port);

        // From all interface (0.0.0.0)
        ReceiverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    SOCKET OpenDGramSocket()
    {
        // Create a new socket to receive datagrams on.
        SOCKET ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (ReceivingSocket == INVALID_SOCKET)
        {
            printf("Server: Error at socket() : % ld\n", WSAGetLastError());
            // Clean up
            WSACleanup();// feels a bit wonky doing this here

            return 0;
        }
        else
            printf("Server: socket() is OK!\n");

        int enable = 1;
        if (setsockopt(ReceivingSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable)) < 0)
        {
            perror("setsockopt(SO_REUSEADDR) failed");
            closesocket(ReceivingSocket);

            // Do the clean up
            WSACleanup();
            return 0;
        }

        return ReceivingSocket;
    }

    bool BindSocket()
    {
        // Associate the address information with the socket using bind.
    // At this point you can receive datagrams on your bound socket.
        if (bind(ReceivingSocket, (SOCKADDR*)&ReceiverAddr, sizeof(ReceiverAddr)) == SOCKET_ERROR)
        {
            printf("Server: bind() failed!Error : % ld.\n", WSAGetLastError());
            return false;
        }
        else
            printf("Server: bind() is OK!\n");

        return true;
    }

    int recvfromTimeOutUDP(SOCKET socket, long sec, long usec)
    {
        // Setup timeval variable
        struct timeval timeout;
        struct fd_set fds;

        timeout.tv_sec = sec;
        timeout.tv_usec = usec;
        // Setup fd_set structure
        FD_ZERO(&fds);
        FD_SET(socket, &fds);
        // Return value:
        // -1: error occurred
        // 0: timed out
        // > 0: data ready to be read
        return select(0, &fds, 0, 0, &timeout);

    }
};

//////////////////////////////////////////////////////////////////


int main()
{
    UDPServer server (5150);

    std::cout << "Hello World!\n";
    cout << from_type<UDPServer>() << endl;
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
