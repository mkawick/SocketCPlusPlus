// UDP01-ThreadedServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <thread>
#include <map>
//using namespace std;



//#include "../UDP01/OldCode/ServerConstants.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#pragma comment(lib, "Ws2_32.lib")
#include "../Common/Connection.h"

//////////////////////////////////////////////////////////////////
typedef std::pair <ULONG, U16> SocketPair;





class UDPServer
{
private:
    bool                isValid;
    U16                 portId;
    SOCKET              ReceivingSocket;
    SOCKADDR_IN         ReceiverAddr;
    char                ipString[INET_ADDRSTRLEN];

    bool                extraLogging;

    
    std::map< SocketPair, Connection*> connections;
public:
    UDPServer(U16 port, bool isThreaded): isValid(false), extraLogging(false)
    {

    }

    void operator()() 
    {// main thread
        if (isValid == false)
            Init();

        if (isValid == false)
            return;
        while (isValid)
        {
            Receive();
            ServiceOutgoingConnections();
        }
    }

    void Init()
    {
        // Initialize Winsock version 2.2
        if (NetworkingInit() == false)
        {
            isValid = false;
            return;
        }

        ReceivingSocket = OpenDGramSocket();
        if (ReceivingSocket == 0)
        {
            isValid = false;
            return;
        }

        int                 Port = 5150;
        SetupSocketDetails(ReceiverAddr, Port);

        if (isValid = BindSocket(); isValid == false)
            return;

        GetLocalSocketInfo();

        isValid = true;
    }

private:
    void Receive()
    {
        int socketBlockInSeconds = 0;
        int socketBlockInMicroSeconds = 20000;
        int SelectTiming = ReceiveFromSocketWithTimeOut(ReceivingSocket, socketBlockInSeconds, socketBlockInMicroSeconds);

        switch (SelectTiming)
        {
        case 0:
            // Timed out, do whatever you want to handle this situation
            if(extraLogging) 
                cout << "Server : Timeout lor while waiting you bastard client!..." << endl;
            break;

        case -1:
            // Error occurred, maybe we should display an error message?
           // Need more tweaking here and the recvfromTimeOutUDP()...
            printf("Server: Some error encountered with code number : % ld\n", WSAGetLastError());
            break;

        default:
            {
                ReceiveAllDataFromEndpoint();
            }
        }
    }

    void ReceiveAllDataFromEndpoint()
    {
        const int           BufLength = 1024;
        char                ReceiveBuf[BufLength];
        SOCKADDR_IN         SenderAddr;
        int SenderAddrSize = sizeof(SenderAddr);
        int BytesReceived = 0;
        do
        {
            // recvfrom is blocking, so I use select to tell me that there is data before receiving
            BytesReceived = recvfrom(ReceivingSocket, ReceiveBuf, BufLength, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
            if (BytesReceived == SOCKET_ERROR)
            {
                printf("Server: recvfrom() failed with error code : %id\n", WSAGetLastError());
                break;
            }
            else if (BytesReceived == 0)
            {
                break;
            }
            LogBytesReceived(BytesReceived, ReceiveBuf);
            
            PassBufferToConnection(SenderAddr, ReceiveBuf, BufLength);
        } while (BytesReceived > BufLength);
    }

    void PassBufferToConnection(SOCKADDR_IN SenderAddr, char* buffer, int bufferLen )
    {
        int SenderAddrSize = sizeof(SenderAddr);
        // Some info on the sender side
        getpeername(ReceivingSocket, (SOCKADDR*)&SenderAddr, &SenderAddrSize);
        ULONG senderKey = SenderAddr.sin_addr.S_un.S_addr;

        //-----------------------------------------
        Connection* connPtr;
        const auto deets = SocketPair(senderKey, SenderAddr.sin_port);
        auto foundItem = connections.find(deets);
        if (foundItem != connections.end())
        {
            connPtr = foundItem->second;
        }
        else
        {
            auto newItem = connections.insert(std::pair(deets, new Connection(ReceivingSocket, SenderAddr, SenderAddr.sin_port)));
            connPtr = newItem.first->second;
        }
        
        connPtr->HandleDataIn(buffer, bufferLen);
        LogReceive(SenderAddr, SenderAddrSize);
    }

    void LogBytesReceived(int BytesReceived, const char* ReceiveBuf)
    {
        if (extraLogging)
        {
            printf("Server: Total Bytes received : %d\n", BytesReceived);
            printf("Server: The data is %s\\n", ReceiveBuf);
        }
    }

    void LogReceive(SOCKADDR_IN SenderAddr, int SenderAddrSize)
    {
        if (extraLogging)
        {
            char ipAddressBuffer[INET_ADDRSTRLEN];
            int err = getnameinfo((struct sockaddr*) & SenderAddr, SenderAddrSize, ipAddressBuffer, sizeof(ipAddressBuffer),
                0, 0, NI_NUMERICHOST);
            printf("Server: Sending IP used : %s\n", ipAddressBuffer);
            printf("Server: Sending port used : %d\n", htons(SenderAddr.sin_port));
        }
    }

    void ServiceOutgoingConnections()
    {
        for (auto connection : connections)
        {
            if (connection.second->NeedsService())
                connection.second->RunService();
            if (connection.second->HasExpired())
            {
                // todo, signal architecture that things are gone
                connection.second->Cleanup();
                delete connection.second;
                auto it = connections.find(connection.first);
                connections.erase(it);
            }
        }
    }

    void GetLocalSocketInfo()
    {
        // Some info on the receiver side...
        int addrlen = sizeof(ReceiverAddr);
        getsockname(ReceivingSocket, (SOCKADDR*)&ReceiverAddr, &addrlen);
        
        inet_ntop(ReceiverAddr.sin_family, (void*)&ReceiverAddr.sin_addr, (PSTR)ipString, sizeof(ipString));

        printf("Server: Receiving IP(s) used : % s\n", ipString);
        printf("Server: Receiving port used : % d\n", htons(ReceiverAddr.sin_port));
        printf("Server: I\'m ready to receive a datagram...\n");
    }

    void CleanupAndExit()
    {
        printf("Server: Finished receiving.Closing the listening socket...\n");

        if (closesocket(ReceivingSocket) != 0)
            printf("Server: closesocket() failed!Error code : % ld\n", WSAGetLastError());
        else
            printf("Server: closesocket() is OK...\n");

        // When your application is finished call WSACleanup.
        printf("Server: Cleaning up...\n");
        if (WSACleanup() != 0)
            printf("Server: WSACleanup() failed!Error code : % ld\n", WSAGetLastError());
        else
            printf("Server: WSACleanup() is OK\n");

        for (auto connection : connections)
        {
            connection.second->Cleanup();
            delete connection.second;
        }
        isValid = false;
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
            CleanupAndExit();

            return 0;
        }
        else
            printf("Server: socket() is OK!\n");

        int enable = 1;
        if (setsockopt(ReceivingSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable)) < 0)
        {
            perror("setsockopt(SO_REUSEADDR) failed");
            CleanupAndExit();
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

    int ReceiveFromSocketWithTimeOut(SOCKET socket, long sec, long usec)
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
    UDPServer server(5150, true);
    //std::shared_ptr< UDPServer> threader;
    //UDPServer threader;
    std::thread t2(server);
    //std::shared_ptr<std::thread> t2(new thread(*threader));

    std::cout << "Hello World!\n";
    cout << from_type<UDPServer>() << endl;
    //t2.detach();
    t2.join();
    _getch();
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
