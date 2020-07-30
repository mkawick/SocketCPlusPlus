// Socket.h
#pragma once

#include "./Socket.h"
#include <list>
#include <memory>
#include <iostream>
using namespace std;
class PacketObserver;

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class ServerSocket
{
public:
    ServerSocket() = delete;
    ServerSocket(const ServerSocket&) = delete;
    ServerSocket& operator=(const ServerSocket&) = delete;
public:
    ServerSocket(U16 port): threader( new TCPThreader(port))
    {
        threader->BeginService();
    }
    ~ServerSocket() {
        if(threader)
            threader->Stop();
        Sleep(50);
        delete threader;
    }

    //--------------------------------------------------------

    //shared_ptr<TCPThreader> GetServer() { return make_shared<TCPThreader>(threader); }

    int NumConnectedClients() const { if (threader == nullptr) return 0; return threader->NumConnectedClients(); }

    //------------------------------------------
    void Register(PacketObserver* listener)
    {
        if (find(listeners.begin(), listeners.end(), listener) != listeners.end())// already in there
            return;
        listeners.push_back(listener);
    }
    void Unregister(PacketObserver* listener)
    {
        auto var = find(listeners.begin(), listeners.end(), listener);
        if (var != listeners.end())// already in there
            listeners.erase(var);
    }

    int GetNumListeners() const { return (int) listeners.size(); }
private:
    TCPThreader* threader;     
    list<PacketObserver*> listeners;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
