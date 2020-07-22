
#include <map>
using namespace std;
#include "../Packets/BasePacket.h"

class PacketListener
{
public:
	void PacketReceived(BasePacket* bp)
	{
		PacketPair key(bp->packetType, bp->packetSubType);
		std::map<Key, Value>::iterator i = numberOfEachType.find(key);

		if (i == amap.end())
			amap.insert(std::make_pair(key, 1));
		else
			Update(&(i->second));
	}

	void Update(int& value)
	{
		value++;
	}

	typedef pair<int,int> PacketPair;
	map< PacketPair, int> numberOfEachType;
};