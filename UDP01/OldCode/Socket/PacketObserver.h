
#include <map>
using namespace std;
#include "../Packets/BasePacket.h"

class IPacketNotification
{
	virtual void Notify(U8 packetType, U8 packetSubType) = 0;
};

class PacketObserver
{
public:
	void PacketReceived(const BasePacket* bp)
	{
		PacketPair key(bp->packetType, bp->packetSubType);
		std::map<PacketPair, int>::iterator i = numberOfEachType.find(key);

		if (i == numberOfEachType.end())
			numberOfEachType.insert(std::make_pair(key, 1));
		else
			Update(i->second);
	}

	void Update(int& value)
	{
		value++;
	}

	int GetCount(U8 packetType, U8 packetSubType)
	{
		PacketPair key(packetType, packetSubType);
		if (auto i = numberOfEachType.find(key); i == numberOfEachType.end())
		{
			return i->second;
		}
		
		return 0;
	}

	typedef pair<int,int> PacketPair;
	map< PacketPair, int> numberOfEachType;
};