// Socket.h
#pragma once


#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>

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
class TCPServer;

class TCPConnection
    : public boost::enable_shared_from_this<TCPConnection>
{
public:
    typedef shared_ptr<TCPConnection> pointer;

    static pointer create(io_context& io_context)
    {
        return pointer(new TCPConnection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void SetServer(TCPServer* srv) {server = srv;}

    void start()
    {
        SetupReceive();
    }

    void SendMessage(shared_ptr<BasePacket>& bp)
    {
        U8 buffer[120];

        int outOffset = 0;
        SizePacket sp;
        sp.packet = bp;
        Serialize::Out(buffer, outOffset, sp, 1);

        //message_.length
        async_write(socket_, boost::asio::buffer(buffer, outOffset),
            boost::bind(&TCPConnection::handle_write, shared_from_this()));
    }

private:
    TCPConnection(io_context& io_context)
        : socket_(io_context)
    {
        std::cout << "new tcp connection" << endl;
    }

    void handle_write()
    {
        std::cout << "handle_write" << endl;
    }
    void SetupReceive();


    //deque<shared_ptr<IPacketSerializable>> inwardPackets;
    TCPServer* server;
    tcp::socket socket_;
    string message_;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class TCPServer
{
public:
    TCPServer(io_context& io_context, U16 portAddress = 1313)
        : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), portAddress)),
        isAccepting(false)
    {
        start_accept();
    }

    bool IsOpen() const
    {
        acceptor_.is_open();
    }
    bool IsListening() const
    {
        return isAccepting;
    }

    void Stop()
    {

        io_context_.stop();
        //acceptor_.cancel();
    }
    U16 GetPortAddr() const
    {
        return boundPortAddress;
    }
    ip::address GetAddress() const
    {
        return boundIpAddress;
    }
    int NumConnectedClients() const { return (int)connectionsMade.size(); }

    void RemoveConnection(TCPConnection* conn) 
    { 
        for(auto i= connectionsMade.begin(); i!= connectionsMade.end(); i++)
        {
            if ((*i).get() == conn)
            {
                connectionsMade.erase(i);
                return;
            }
        }
    }

private:
    void start_accept()
    {
        TCPConnection::pointer new_connection =
            TCPConnection::create(io_context_);

        std::cout << "Start accept" << endl;
        acceptor_.async_accept(new_connection->socket(),
            boost::bind(&TCPServer::handle_accept, this, new_connection,
                boost::asio::placeholders::error));

        isAccepting = true;
        boundPortAddress = acceptor_.local_endpoint().port();
        boundIpAddress = acceptor_.local_endpoint().address();
    }

    void handle_accept(TCPConnection::pointer new_connection,
        const boost::system::error_code& error)
    {
        std::cout << "handle accept" << endl;
        if (!error)
        {
            boost::asio::socket_base::receive_buffer_size optionBufferSize(8192);
            new_connection->socket().set_option(optionBufferSize);
            boost::asio::socket_base::keep_alive optionKeepAlive(true);
            new_connection->socket().set_option(optionKeepAlive);
            // std::string sClientIp = socket().remote_endpoint().address().to_string();
            // unsigned short uiClientPort = socket().remote_endpoint().port();
            boost::asio::ip::tcp::no_delay optionNagel(true); // nagel
            new_connection->socket().set_option(optionNagel);
            new_connection->start();
            new_connection->SetServer(this);
            //connectionsMade.push_back(new_connection);
            connectionsMade.push_back(new_connection);
        }

        // start all over again
        start_accept();
    }
    

    io_context& io_context_;
    tcp::acceptor acceptor_;
    bool isAccepting;
    U16 boundPortAddress;
    ip::address boundIpAddress;
    list<TCPConnection::pointer> connectionsMade;
};

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class TCPThreader
{
public:
    TCPThreader() = delete;
    TCPThreader(const TCPThreader& t) = delete;
    TCPThreader& operator=(const TCPThreader&) = delete;

public:
    TCPThreader(U16 portAddr = 1313): portAddress(portAddr), tcpServer(nullptr),
        work(new io_service::work(myIoServiceType)) {}
    ~TCPThreader()
    {
        Stop();
        delete tcpServer;
    }

    void BeginService()
    {
        tcpServer = new TCPServer(myIoServiceType, portAddress);

        // give it some work, to prevent premature exit
        boost::thread process(boost::bind(&boost::asio::io_service::run, &myIoServiceType));
        process.detach();

        myIoServiceType.post(boost::bind(&TCPThreader::ThreadExit, this));
        //https://www.boost.org/doc/libs/1_67_0/doc/html/boost_asio/reference/LegacyCompletionHandler.html


        boost::asio::signal_set signals{ myIoServiceType, SIGINT, SIGTERM };// graceful exit
        signals.async_wait(boost::bind(&boost::asio::io_service::stop, &myIoServiceType));
    }

    bool IsConnected() const 
    {
        if (tcpServer == nullptr)
            return false;
        return tcpServer->IsListening();
    }

    void Stop()
    {
        work.reset();
    }
    U16 GetPortAddr() const
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
    int NumConnectedClients() const { return tcpServer->NumConnectedClients(); }

    
private:
    TCPServer* tcpServer;
    boost::asio::io_service myIoServiceType;
    
    shared_ptr<io_service::work> work;
    U16 portAddress;

    void ThreadExit()
    {
        std::cout << "Socket closed and thread has exited" << std::endl;
    }
};

/*  void Foo()
  {
      boost::asio::io_service io_service;
      // give it some work, to prevent premature exit
      ///shared_ptr<io_service::work> work(new io_service::work(io_service));

      boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));

      t.detach();
      //...
      io_service.post(boost::bind(&TCPThreader::ThreadExit, this));
  // https://www.boost.org/doc/libs/1_67_0/doc/html/boost_asio/reference/LegacyCompletionHandler.html
      //io_service.post(yourFunctor);
  }*/