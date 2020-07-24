// BoostASIO_Client04_TCP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <conio.h>
#include "../UDP01/OldCode/Packets/Serialize.h"
#include "../UDP01/OldCode/Packets/BasePacket.h"
#include "../UDP01/OldCode/Packets/MovementPacket.h"
#include "../UDP01/OldCode/Packets/PacketFactory.h"
#include "../UDP01/OldCode/Packets/MovementPacket.h"
#include "../UDP01/OldCode/Socket/PacketObserver.h"

#include "../UDP01/OldCode/Socket/Socket.h"
#include "../UDP01/OldCode/Socket/ClientSocket.h"
#include "../UDP01/OldCode/Socket/ServerSocket.h"


int main()
{
	PacketMethodFactory::InitFactory();

	boost::asio::io_context io_context;
	tcp::resolver::query query(tcp::v4(), "localhost", "1313");
	tcp::resolver resolve(io_context);
	client testClient(io_context);


	testClient.start(resolve.resolve(query));
	Sleep(100);

	//Assert::AreNotEqual(server.NumConnectedClients(), 0);
	//Assert::AreEqual(testClient.IsConnected(), true);

	//BasePacket bp;
	//bp.packetSubType = BasePacket::SubType::BasePacket_Type;
	auto pack = PacketMethodFactory::Create(PacketType_Base, ServerTickPacket::BasePacket_Type);
	BasePacket* bp = dynamic_cast<BasePacket*>(pack->GetTypePtr());
	bp->gameInstanceId = 13;
	bp->gameProductId = 51;
	testClient.Write(pack);

	//Assert::AreEqual(testClient.GetNumPendingOutgoingPackets(), 1);

	//----------------------------------------
	io_context.run_for(boost::asio::chrono::seconds(65));
    std::cout << "Hello World!\n";

	PacketMethodFactory::Shutdown();
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
