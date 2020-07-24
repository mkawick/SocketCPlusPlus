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

void TCPConnection::SetupReceive()
{
    const int max_length = 512;
    U8 buffer[max_length];
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(buffer, max_length),
        [this, self, buffer = std::move(buffer)](boost::system::error_code ec, std::size_t length)
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

                //inwardPackets.push_back(sp2.packet);
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