#pragma once
#include <cmath>


///////////////////////////////////////////////////////////////

struct Vector3
{
    float x, y, z;
    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    float magnitude() const { return std::sqrt(x * x + y * y + z * z); }

    void operator *= (float mag) 
    {
        x *= mag; 
        y *= mag; 
        z *= mag; 
    }
};

///////////////////////////////////////////////////////////////

class RotationCompressed // : public IBinarySerializable// using 360 degrees as an integer
{
public:
    int rotation;

    RotationCompressed()
    {
        rotation = 0;
    }

    void CopyFrom(RotationCompressed ext)
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

class PositionCompressed //: public IBinarySerializable
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

class FloatCompressed
{
public:
    void Set(float pos);
    float Get();

private:
    U16 value;
};

///////////////////////////////////////////////////////////////

class ServerTickPacket : public BasePacket
{
public:
    enum SubType
    {
        ServerTick_Base,
        ServerTick_Position,
        ServerTick_Movement
    };
public:
    ServerTickPacket(int packet_sub_type = ServerTick_Base) : BasePacket(PacketType_ServerTick, packet_sub_type), serverTick(0){}
    bool    SerializeIn(const U8* data, int& bufferOffset, int minorVersion);
    bool    SerializeOut(U8* data, int& bufferOffset, int minorVersion) const;


    U16 serverTick;
};
///////////////////////////////////////////////////////////////

class PositionPacket : public ServerTickPacket
{
public:
    PositionPacket(int packet_sub_type = ServerTick_Position) : ServerTickPacket(packet_sub_type) {}
    bool    SerializeIn(const U8* data, int& bufferOffset, int minorVersion);
    bool    SerializeOut(U8* data, int& bufferOffset, int minorVersion) const;
    void    Set(const Vector3& position, const Vector3& direction);
    void    Get(Vector3& position, Vector3& direction);


    PositionCompressed positionCompressed;
    RotationCompressed rotationCompressed;

    //--------------------------------------------------
public: // factory interface
    static constexpr int   GetSize();

    string GetName()    override { return GetFactoryName(); }
    U8 GetType()        override { return Type(); }
    U8 GetSubType()     override { return SubType(); }

    static shared_ptr<IPacketSerializable> CreateMethod();

    static string GetFactoryName() { return from_type<PositionPacket>(); }
    static U8 Type() { return PacketType_ServerTick; }
    static U8 SubType() { return ServerTick_Position; }

    static bool s_typeRegistered;
};
///////////////////////////////////////////////////////////////

class MovementPacket : public PositionPacket
{
public:
    MovementPacket() : PositionPacket(ServerTick_Movement) {}
    bool    SerializeIn(const U8* data, int& bufferOffset, int minorVersion);
    bool    SerializeOut(U8* data, int& bufferOffset, int minorVersion) const;
    void    Set(const Vector3& position, const Vector3& direction, const Vector3& movementDir);
    void    Get(Vector3& position, Vector3& direction, Vector3& movementDir);

    PositionCompressed  movementDirCompressed;
    //FloatCompressed     movementMagnitudeCompressed;

 //--------------------------------------------------
public: // factory interface
    static constexpr int   GetSize();

    string GetName()    override { return GetFactoryName(); }
    U8 GetType()        override { return Type(); }
    U8 GetSubType()     override { return SubType(); }

    static shared_ptr<IPacketSerializable> CreateMethod();

    static string GetFactoryName() { return from_type<MovementPacket>(); }
    static U8 Type() { return PacketType_ServerTick; }
    static U8 SubType() { return ServerTick_Movement; }

    static bool s_typeRegistered;
};
///////////////////////////////////////////////////////////////
