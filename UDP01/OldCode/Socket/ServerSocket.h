// Socket.h
#pragma once

#include "./Socket.h"
#include <list>
using namespace std;
class PacketListener;

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class ServerSocket
{
public:
    ServerSocket(U16 port) {
        threader = new TCPThreader(port);
        threader->BeginService();
    }
    ~ServerSocket() {
        if(threader)
            threader->Stop();
        Sleep(50);
        delete threader;
    }

    shared_ptr< TCPThreader> GetServer() { return make_shared(threader); }

    void Register(PacketListener* listener)
    {
        if (listeners.find(listener) != listeners.end())
            return;
        listeners.push_back(listener);
    }
private:
    TCPThreader* threader;
    list< PacketListener*> listeners;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
