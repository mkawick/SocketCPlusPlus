// Socket.h
#pragma once


#include <ctime>
#include <iostream>
#include <string>
#include <queue> 
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/core/noncopyable.hpp>

#include <iostream>
using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;
#include "../DataTypes.h"
#include "../Packets/BasePacket.h"
#include "../Packets/PacketFactory.h"

// be sure to add D:\Develop\boost\stage\lib to the Additional Libraries
// building boost: https://stackoverflow.com/questions/13042561/fatal-error-lnk1104-cannot-open-file-libboost-system-vc110-mt-gd-1-51-lib

// _WIN32_WINNT=0x0A00
// _WIN32_WINNT_WIN10   

ip::address getLocalIPAddress();

string MakeDaytimeString();

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class UDPServer
{
public:
    UDPServer(io_context& io_context, U16 portAddr = 1313)
        : socket_(io_context, udp::endpoint(udp::v4(), portAddr))
    {
        start_receive();
    }

private:
    void start_receive()
    {
        socket_.async_receive_from(
            buffer(ReceiveBuffer), remote_endpoint_,
            boost::bind(&UDPServer::handle_receive, this,
                boost::asio::placeholders::error));
    }

    void handle_receive(const boost::system::error_code& error)
    {
        if (!error)
        {
            boost::shared_ptr<string> message(
                new string(MakeDaytimeString()));

            socket_.async_send_to(buffer(*message), remote_endpoint_,
                boost::bind(&UDPServer::handle_send, this, message));

            start_receive();
        }
    }

    void handle_send(boost::shared_ptr<string> message)
    {
        cout << "UDP: " << message.get() << endl;
    }

    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    boost::array<char, 1> ReceiveBuffer;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
class TCPServer;

class TCPConnection
    : public std::enable_shared_from_this<TCPConnection>//, private boost::noncopyable
{
public:
    typedef std::shared_ptr<TCPConnection> pointer;

    static pointer Create(io_context& io_context)
    {
        struct make_shared_enabler : public TCPConnection { make_shared_enabler(boost::asio::io_context& io_context) :TCPConnection(io_context) {} };

        return std::make_shared<make_shared_enabler>(io_context);
    }

    std::shared_ptr<TCPConnection> getptr() {
        return shared_from_this();
    }

    //-----------------------------------------------------------
    tcp::socket& GetSocket() { return socket_; }
    void    Close()
    {
        hasClosed = true;

        socket_.close();
        socket_.release();
    }
    bool    IsClosed() const { return hasClosed; }

    void    SetConnectionId(U16 connId) { connectionId = connId; }
    U16     GetConnectionId() const { return connectionId; }
    //-------------------------------------------

    void    SetServer(TCPServer* srv) {server = srv;}

    void    Start();

    void    SendMessage(shared_ptr<IPacketSerializable>& bp);
    int     GetNumMessages() const noexcept { return (int)inwardPackets.size(); }
    const shared_ptr<IPacketSerializable> 
            PeekMessage() const { return inwardPackets.front(); }
    void    DequeMessage();
private:
    TCPConnection(io_context& io_context);
    TCPConnection() = delete;

    void handle_write()
    {
        std::cout << "handle_write" << endl;
    }


    static const int max_length = 512;
    U8 receiveBuffer[max_length];
    //deque<shared_ptr<IPacketSerializable>> inwardPackets;
    TCPServer* server;
    tcp::socket socket_;
    string message_;
    deque< shared_ptr<IPacketSerializable>> inwardPackets;
    bool hasClosed;
    U16 connectionId;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class TCPServer
{
public:
    TCPServer(io_context& io_context, U16 portAddress = 1313)
        : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), portAddress)),
        isAccepting(false),
        connectionIdTracking(connectionIdStart)
    {}

    bool    IsOpen() const
    {
        acceptor_.is_open();
    }
    bool    IsListening() const
    {
        return isAccepting;
    }

    void    Start();
    void    Stop()
    {
        io_context_.stop();
        //acceptor_.cancel();
    }

    //--------------------------------------------------------
    U16     GetPortAddr() const
    {
        return boundPortAddress;
    }
    ip::address GetAddress() const
    {
        return boundIpAddress;
    }

    int     NumConnectedClients() const { return (int)connectionsMade.size(); }

    //--------------------------------------------------------
    void    RemoveConnection(TCPConnection* conn) 
    { 
        for(auto i= connectionsMade.begin(); i!= connectionsMade.end(); i++)
        {
            if ((*i).get() == conn)
            {
                // todo, clean up old connections
                //conn->Close();
                //connectionsMade.erase(i);

                return;
            }
        }
    }
    void    CleanupClosedConnections();

    void    HandleReschedule();
    void    RescheduleMe(TCPConnection::pointer conn);

private:
    void    BeginAcceptingNewConnections();
    void    NewConnectionAcceptedHandler(TCPConnection::pointer new_connection,
        const boost::system::error_code& error);
    
    const static int connectionIdStart = 1024;
    int connectionIdTracking;
    io_context& io_context_;
    tcp::acceptor acceptor_;
    bool isAccepting;
    U16 boundPortAddress;
    ip::address boundIpAddress;
    list<TCPConnection::pointer> connectionsMade;
    list<TCPConnection::pointer> connectionsAwaitingSchedule;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class TCPThreader// : public IPacketNotifcation
{
public:
    TCPThreader() = delete;
    TCPThreader(const TCPThreader& t) = delete;
    TCPThreader& operator=(const TCPThreader&) = delete;

public:
    TCPThreader(U16 portAddr = 1313);
    ~TCPThreader();

    void    BeginService();

    bool    IsConnected() const 
    {
        if (tcpServer == nullptr)
            return false;
        return tcpServer->IsListening();
    }

    void PrepareStop()
    {
        isThreadRunning = false;
    }
    void    Stop()
    {
        std::cout << "work.Reset()" << std::endl;
        work.reset();
    }
    U16     GetPortAddr() const
    {
        if (tcpServer == nullptr)
            return portAddress;
        return tcpServer->GetPortAddr();
    }
    ip::address GetSocketAddr()
    {
        if (tcpServer == nullptr)
            return ip::address::from_string("0.0.0.0");
        return tcpServer->GetAddress();
    }
    int     NumConnectedClients() const
    { 
        return tcpServer->NumConnectedClients(); 
    }

    //---------------------------------------------------

private:
    TCPServer* tcpServer;
    boost::asio::io_service myIoServiceType;
    
    executor_work_guard<io_context::executor_type> work;
    boost::thread_group threads;
    U16     portAddress;
    bool    isThreadRunning;
    //-----------------------------------
    void    ThreadHasStarted();
    void    ThreadRun();
    void    CleanupClosedConnections();
};
