#pragma once

class PacketFactory
{
public:
    PacketFactory();
    bool     Parse(const U8* bufferIn, int& bufferOffset, BasePacket** packetOut, int networkMinorVersion) const;// be sure to check the return value
    void     CleanupPacket(BasePacket*& packetOut);

private:
    
};