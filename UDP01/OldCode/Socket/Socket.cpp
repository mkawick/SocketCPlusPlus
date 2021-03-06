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

void LogPacket(shared_ptr<IPacketSerializable> packet)
{
    cout << "Packet received:" << endl;
    cout << "    " << packet->GetName() << endl;
    cout << "     (" << (int)packet->GetType() << ", " << (int)packet->GetSubType() << ")" << endl;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void    TCPServer::Start()
{
    boost::thread process(boost::bind(&TCPServer::BeginAcceptingNewConnections, this));
    process.detach();
}

void    TCPServer::BeginAcceptingNewConnections()
{
    TCPConnection::pointer newConnection = TCPConnection::Create(io_context_);
    newConnection->SetConnectionId(connectionIdTracking++);

    std::cout << "** BeginAcceptingNewConnections ***" << endl;
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.async_accept(newConnection->GetSocket(),
        boost::bind(&TCPServer::NewConnectionAcceptedHandler, this, newConnection,
            boost::asio::placeholders::error));

    isAccepting = true;
    boundPortAddress = acceptor_.local_endpoint().port();
    boundIpAddress = acceptor_.local_endpoint().address();
}

void    TCPServer::NewConnectionAcceptedHandler(TCPConnection::pointer newConnection, const boost::system::error_code& error)
{
    if (error)
    {
        std::cout << " Error on listening socket during accepting New Connection " << endl;
    }
    else
    {
        std::cout << " Accepting New Connection " << endl;
        boost::asio::socket_base::receive_buffer_size optionBufferSize(8192);
        newConnection->GetSocket().set_option(optionBufferSize);
        boost::asio::socket_base::keep_alive optionKeepAlive(true);
        newConnection->GetSocket().set_option(optionKeepAlive);
        boost::asio::ip::tcp::no_delay optionNagel(true); // nagel
        newConnection->GetSocket().set_option(optionNagel);

        // std::string sClientIp = socket().remote_endpoint().address().to_string();
        // unsigned short uiClientPort = socket().remote_endpoint().port();

        newConnection->SetServer(this);
        
        RescheduleMe(newConnection);
        connectionsMade.push_back(newConnection);
    }

    // start all over again
    BeginAcceptingNewConnections();
}

void    TCPServer::RescheduleMe(TCPConnection::pointer conn)
{
    // needs threading protection
    connectionsAwaitingSchedule.push_back(conn);
}

void    TCPServer::HandleReschedule()
{
    // needs threading magic
    for (auto it : connectionsAwaitingSchedule)
    {
        it->Start();
    }

    connectionsAwaitingSchedule.clear();
}

void    TCPServer::CleanupClosedConnections()
{
    for (auto it = connectionsMade.begin(); it != connectionsMade.end(); )
    {
        auto it2 = it++;
        auto val = *it2;
        if ((*it2)->IsClosed())
        {
            connectionsMade.erase(it2);
            TCPConnection* memory = val.get();
            val.reset();
            cout << "releasing connection" << endl;
            //delete memory;
            //delete val.;
        }
    }
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
TCPThreader::TCPThreader(U16 portAddr) : 
    portAddress(portAddr), 
    tcpServer(nullptr), 
    isThreadRunning(false), 
    work(boost::asio::make_work_guard(myIoServiceType))
{
}
TCPThreader::~TCPThreader()
{
    Stop();
    threads.join_all();
    delete tcpServer;
}
void TCPThreader::BeginService()
{
    tcpServer = new TCPServer(myIoServiceType, portAddress);
    tcpServer->Start();
    
    isThreadRunning = true;

    for (size_t i = 0; i < 5; ++i) {
        boost::thread* t = threads.create_thread(boost::bind(&boost::asio::io_service::run, &myIoServiceType));
        std::cout << "Creating thread " << i << " with id " << t->get_id() << std::endl;
    }

    boost::thread process2(boost::bind(&TCPThreader::ThreadRun, this));// maybe not needed
    process2.detach();
    //boost::asio::io_service::work work(myIoServiceType);

    myIoServiceType.post(boost::bind(&TCPThreader::ThreadHasStarted, this));
    //https://www.boost.org/doc/libs/1_67_0/doc/html/boost_asio/reference/LegacyCompletionHandler.html

    boost::asio::signal_set signals{ myIoServiceType, SIGINT, SIGTERM };// graceful exit
    // , SIGQUIT, SIGTSTP
    signals.async_wait(boost::bind(&boost::asio::io_service::stop, &myIoServiceType));
}

void    TCPThreader::ThreadHasStarted()
{
    string networkName = "Axiom";
    std::cout << "----------------------------------------------------------------------------------" << std::endl;
    std::cout << "-------------- Successful start of networking engine: " << networkName << " ----------------------" << std::endl;
    std::cout << "-------------- Listening socket started and main service running -----------------" << std::endl;
    std::cout << "----------------------------------------------------------------------------------" << std::endl;
}
void    TCPThreader::ThreadRun()
{
    std::cout << "Servicing thread running" << std::endl;
    while (isThreadRunning)
    {
        myIoServiceType.run();
        
        Sleep(10);

        if(isThreadRunning == true)
            CleanupClosedConnections();

        if (isThreadRunning == true)
            tcpServer->HandleReschedule();
    }
    std::cout << "Servicing thread exited" << std::endl;
}

void    TCPThreader::CleanupClosedConnections()
{
    // there may need to be threading protections
    tcpServer->CleanupClosedConnections();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

TCPConnection::TCPConnection(io_context& io_context)
    : socket_(io_context),
    hasClosed(false)
{
    std::cout << "new tcp connection" << endl;
}
void    TCPConnection::Start()
{    
    cout << "SetupReceive starting" << endl << "    connectionId: " << connectionId << endl;
    socket_.async_read_some(boost::asio::buffer(receiveBuffer, max_length),
        [this, buffer = std::move(receiveBuffer)](boost::system::error_code errorCode, std::size_t length)
    {
        if (!errorCode)
        {
            int inOffset = 0;
            int index = 0;
            SizePacket sp2;
            while (sp2.IsRemainingBufferBigenough(buffer, inOffset, 1, (int)length))
            {
                Serialize::In(buffer, inOffset, sp2, 1);
                BasePacket* bp2 = dynamic_cast<BasePacket*>(sp2.packet->GetTypePtr());

                inwardPackets.push_back(sp2.packet);

                LogPacket(sp2.packet);
                sp2.packet.reset();
                index++;
            }

            server->RescheduleMe(shared_from_this());
        }
        else
        {
            cout << "error or DC" << endl;
            cout << "error code was: " << errorCode << endl;
            // todo, we must queue the release, not a direct removal. shared_ptr helps a bit
                
            this->Close();
            server->RemoveConnection(this); // needs to be moved out of this thread
            return;
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
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
