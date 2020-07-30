#include "Socket.h"

ip::address getLocalIPAddress()
{
    ip::address addr;
    try {
        boost::asio::io_service netService;
        udp::resolver   resolver(netService);
        udp::resolver::query query(udp::v4(), "google.com", "");
        udp::resolver::iterator endpoints = resolver.resolve(query);
        udp::endpoint ep = *endpoints;
        udp::socket socket(netService);
        socket.connect(ep);
        addr = socket.local_endpoint().address();
        //std::cout << "My IP according to google is: " << addr.to_string() << std::endl;
    }
    catch (std::exception& e) {
        std::cerr << "Could not deal with socket. Exception: " << e.what() << std::endl;

    }

    return addr;
}

string MakeDaytimeString()// library problems
{
    time_t now = time(0);
    return ctime(&now);
}


void TCPThreader::BeginService()
{
    tcpServer = new TCPServer(myIoServiceType, portAddress);
    tcpServer->BeginAcceptingNewConnections();

    // give it some work, to prevent premature exit
  /*  boost::thread autoProcess(boost::bind(&boost::asio::io_service::run, boost::ref(myIoServiceType)));// call run on the process
    autoProcess.detach();*/
    
 /*   boost::thread process(boost::bind(&TCPServer::BeginAcceptingNewConnections, tcpServer));
    process.detach();*/

    boost::thread process2(boost::bind(&TCPThreader::ThreadRun, this));// maybe not needed
    process2.detach();
    boost::asio::io_service::work work(myIoServiceType);

    myIoServiceType.post(boost::bind(&TCPThreader::ThreadExit, this));
    //https://www.boost.org/doc/libs/1_67_0/doc/html/boost_asio/reference/LegacyCompletionHandler.html

    boost::asio::signal_set signals{ myIoServiceType, SIGINT, SIGTERM };// graceful exit
    signals.async_wait(boost::bind(&boost::asio::io_service::stop, &myIoServiceType));

    //myIoServiceType.run();
}

void    TCPServer::BeginAcceptingNewConnections()
{
    TCPConnection::pointer new_connection =
        TCPConnection::create(io_context_);

    std::cout << "BeginAcceptingNewConnections" << endl;
    //
    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&TCPServer::handle_accept, this, new_connection,
            boost::asio::placeholders::error));

    isAccepting = true;
    boundPortAddress = acceptor_.local_endpoint().port();
    boundIpAddress = acceptor_.local_endpoint().address();
}

void    TCPServer::handle_accept(TCPConnection::pointer new_connection, const boost::system::error_code& error)
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
        connectionsMade.push_back(new_connection);
    }

    // start all over again
    BeginAcceptingNewConnections();
}
///////////////////////////////////////////////////////////

TCPConnection::TCPConnection(io_context& io_context)
    : socket_(io_context)
{
    std::cout << "new tcp connection" << endl;
}
void    TCPConnection::start()
{
    //SetupReceive();
    
    cout << "New connection thread starting" << endl;
    boost::thread process(boost::bind(&TCPConnection::SetupReceive, this));
    process.detach();
    
}
void TCPConnection::SetupReceive()
{
    
    //auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(receiveBuffer, max_length),
        [this, buffer = std::move(receiveBuffer)](boost::system::error_code ec, std::size_t length)
    {
        if (!ec)
        {
          /*  while (inOffset < length)
            {
                BasePacket sampler;
                Serialize::In(buffer, sampleOffset, sampler, 1);
                shared_ptr<IPacketSerializable> unpack1 = PacketMethodFactory::Create(sampler.packetType, sampler.packetSubType);
                Serialize::In(buffer, inOffset, *unpack1, 1);
                // todo: needs thread safety
                //inwardPackets.push_back(unpack1);
            }*/
            int inOffset = 0;
            int index = 0;
            SizePacket sp2;
            while (sp2.IsRemainingBufferBigenough(buffer, inOffset, 1, (int)length))
            {
                Serialize::In(buffer, inOffset, sp2, 1);
                BasePacket* bp2 = dynamic_cast<BasePacket*>(sp2.packet->GetTypePtr());

                inwardPackets.push_back(sp2.packet);
                sp2.packet.reset();
                index++;
            }
        }
        else
        {
            cout << "error or DC" << endl;
            // todo, we must queue the release, not a direct removal. shared_ptr helps a bit
            server->RemoveConnection(this);
        }
    });
}

void TCPConnection::SendMessage(shared_ptr<IPacketSerializable>& bp)
{
    U8 buffer[120];

    int outOffset = 0;
    SizePacket sp;
    sp.packet = bp;
    Serialize::Out(buffer, outOffset, sp, 1);

    async_write(socket_, boost::asio::buffer(buffer, outOffset),
        boost::bind(&TCPConnection::handle_write, shared_from_this()));
}

void TCPConnection::DequeMessage()
{
    auto i = inwardPackets.front(); // todo... release back to memory mgmt
    inwardPackets.pop_front();
    PacketMethodFactory::Release(i);
}