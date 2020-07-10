#pragma once



///////////////////////////////////////////////////////////////

struct Vector3
{
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
};

///////////////////////////////////////////////////////////////

class RotationPacker : public IBinarySerializable// using 360 degrees as an integer
{
public:
    int rotation;

    RotationPacker()
    {
        rotation = 0;
    }

    void CopyFrom(RotationPacker ext)
    {
        rotation = ext.rotation;
    }


    void Set(const Vector3& eulerAngles);
    Vector3 Get();

    bool  SerializeIn(const U8* data, int& bufferOffset, int minorVersion);
    bool  SerializeOut(U8* data, int& bufferOffset, int minorVersion) const;
private:
    int Pack(int x, int y, int z);
    void Unpack(int value, int& x, int& y, int& z);
};

///////////////////////////////////////////////////////////////


class PositionCompressed : public IBinarySerializable
{
public:
    U64 position = 0;

    
    void CopyFrom(PositionCompressed ext)
    {
        position = ext.position;
    }

    void Set(const Vector3& pos);
    Vector3 Get();


    bool  SerializeIn(const U8* data, int& bufferOffset, int minorVersion);
    bool  SerializeOut(U8* data, int& bufferOffset, int minorVersion) const;

private:
    void Unpack(U64 position, float& x, float& y, float& z);
    U64 Pack(float x, float y, float z);
};

///////////////////////////////////////////////////////////////

class PositionPacket : public BasePacket
{
public:
public:
    enum SubType
    {
        PositionPacket_Position,
        PositionPacket_Movement
    };
public:
    PositionPacket(int packet_type = PacketType_Movement, int packet_sub_type = PositionPacket_Position) : BasePacket(packet_type, packet_sub_type) {}
    bool    SerializeIn(const U8* data, int& bufferOffset, int minorVersion);
    bool    SerializeOut(U8* data, int& bufferOffset, int minorVersion) const;
    void    Set(const Vector3& position, const Vector3& direction);
    void    Get(Vector3& position, Vector3& direction);

    PositionCompressed positionCompressed;
    RotationPacker rotationCompressed;
};
///////////////////////////////////////////////////////////////

class MovementPacket : public PositionPacket
{
public:
    MovementPacket() : PositionPacket(PacketType_Movement, PositionPacket_Movement) {}
    bool    SerializeIn(const U8* data, int& bufferOffset, int minorVersion);
    bool    SerializeOut(U8* data, int& bufferOffset, int minorVersion) const;
    void    Set(const Vector3& position, const Vector3& direction, const Vector3& movementDir);
    void    Get(Vector3& position, Vector3& direction, Vector3& movementDir);

    RotationPacker movementDirCompressed;
};
///////////////////////////////////////////////////////////////
