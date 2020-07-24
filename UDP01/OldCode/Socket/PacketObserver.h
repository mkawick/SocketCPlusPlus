
#include <map>
using namespace std;
#include "../Packets/BasePacket.h"

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

	typedef pair<int,int> PacketPair;
	map< PacketPair, int> numberOfEachType;
};