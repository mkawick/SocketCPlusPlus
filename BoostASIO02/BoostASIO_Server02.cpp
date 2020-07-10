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
#include <ctime>
#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include <iostream>
using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

string MakeDaytimeString()
{
    time_t now = time(0);
    return ctime(&now);
}

class TCPConnection
    : public boost::enable_shared_from_this<TCPConnection>
{
public:
    typedef boost::shared_ptr<TCPConnection> pointer;

    static pointer create(io_context& io_context)
    {
        return pointer(new TCPConnection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        message_ = MakeDaytimeString();

        async_write(socket_, boost::asio::buffer(message_),
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

    tcp::socket socket_;
    string message_;
};

class TCPServer
{
public:
    TCPServer(io_context& io_context)
        : io_context_(io_context),
        acceptor_(io_context, tcp::endpoint(tcp::v4(), 13))
    {
        start_accept();
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
    }

    void handle_accept(TCPConnection::pointer new_connection,
        const boost::system::error_code& error)
    {
        std::cout << "handle accept" << endl;
        if (!error)
        {
            new_connection->start();
        }

        start_accept();
    }

    io_context& io_context_;
    tcp::acceptor acceptor_;
};

class UDPServer
{
public:
    UDPServer(io_context& io_context)
        : socket_(io_context, udp::endpoint(udp::v4(), 13))
    {
        start_receive();
    }

private:
    void start_receive()
    {
        socket_.async_receive_from(
            buffer(recv_buffer_), remote_endpoint_,
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
    boost::array<char, 1> recv_buffer_;
};

int main()
{
    try
    {
        io_context io_context;
        TCPServer server1(io_context);
        UDPServer server2(io_context);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}