
#include "./BasePacket.h"
#include "./MovementPacket.h"

///////////////////////////////////////////////////////////////

bool    ServerTickPacket::SerializeIn(const U8* data, int& bufferOffset, int minorVersion)
{
    BasePacket::SerializeIn(data, bufferOffset, minorVersion);
    Serialize::In(data, bufferOffset, serverTick, minorVersion);

    return true;
}
bool    ServerTickPacket::SerializeOut(U8* data, int& bufferOffset, int minorVersion) const
{
    BasePacket::SerializeOut(data, bufferOffset, minorVersion);
    Serialize::Out(data, bufferOffset, serverTick, minorVersion);

    return true;
}

///////////////////////////////////////////////////////////////


bool    PositionPacket::SerializeIn(const U8* data, int& bufferOffset, int minorVersion)
{
    ServerTickPacket::SerializeIn(data, bufferOffset, minorVersion);
    Serialize::In(data, bufferOffset, positionCompressed, minorVersion);
    Serialize::In(data, bufferOffset, rotationCompressed, minorVersion);

    return true;
}
bool    PositionPacket::SerializeOut(U8* data, int& bufferOffset, int minorVersion) const
{
    ServerTickPacket::SerializeOut(data, bufferOffset, minorVersion);
    Serialize::Out(data, bufferOffset, positionCompressed, minorVersion);
    Serialize::Out(data, bufferOffset, rotationCompressed, minorVersion);

    return true;
}

void    PositionPacket::Set(const Vector3& position, const Vector3& direction)
{
    positionCompressed.Set(position);
    rotationCompressed.Set(direction);
}
void    PositionPacket::Get(Vector3& position, Vector3& direction)
{
    position = positionCompressed.Get();
    direction = rotationCompressed.Get();
}
#ifdef _UNIT_TESTING_
bool PositionPacket::s_typeRegistered = false;
#else
bool 
#endif
///////////////////////////////////////////////////////////////PositionPacket::s_typeRegistered = PacketMethodFactory::Register(PositionPacket::GetFactoryName(), PositionPacket::Type(), PositionPacket::SubType(), PositionPacket::CreateMethod);

bool    MovementPacket::SerializeIn(const U8* data, int& bufferOffset, int minorVersion)
{
    PositionPacket::SerializeIn(data, bufferOffset, minorVersion);
    //Serialize::In(data, bufferOffset, movementMagnitudeCompressed, minorVersion);
    Serialize::In(data, bufferOffset, movementDirCompressed, minorVersion);

    return true;
}
bool    MovementPacket::SerializeOut(U8* data, int& bufferOffset, int minorVersion) const
{
    PositionPacket::SerializeOut(data, bufferOffset, minorVersion);
    //Serialize::Out(data, bufferOffset, movementMagnitudeCompressed, minorVersion);
    Serialize::Out(data, bufferOffset, movementDirCompressed, minorVersion);

    return true;
}
void    MovementPacket::Set(const Vector3& position, const Vector3& direction, const Vector3& movementDir)
{
    PositionPacket::Set(position, direction);

   /* float mag = movementDir.magnitude();
    if (mag == 0)
        mag = 0.0001f;
    Vector3 dir = movementDir;
    float mult = 1 / mag;
    dir *= mult;*/
    
    //movementMagnitudeCompressed.Set(mag);
    movementDirCompressed.Set(movementDir);
}
void    MovementPacket::Get(Vector3& position, Vector3& direction, Vector3& movementDir)
{
    PositionPacket::Get(position, direction);
    movementDir = movementDirCompressed.Get();
    //float mag = movementMagnitudeCompressed.Get();
    //movementDir = movementDirCompressed;
}

#ifdef _UNIT_TESTING_
bool MovementPacket::s_typeRegistered = false;
#else
bool MovementPacket::s_typeRegistered = PacketMethodFactory::Register(MovementPacket::GetFactoryName(), MovementPacket::Type(), MovementPacket::SubType(), MovementPacket::CreateMethod);
#endif
///////////////////////////////////////////////////////////////

void FloatCompressed::Set(float pos)
{
    _ASSERT(abs(pos * Network::Settings::Float::shift) < 16537);
    value = static_cast<U16>(pos* Network::Settings::Float::shift);
}
float FloatCompressed::Get()
{
    float ret = value;
    ret /= Network::Settings::Float::shift;
    return ret;
}

///////////////////////////////////////////////////////////////


bool  RotationCompressed::SerializeIn(const U8* data, int& bufferOffset, int minorVersion)
{
    Serialize::In(data, bufferOffset, rotation, minorVersion);

    return true;
}
bool  RotationCompressed::SerializeOut(U8* data, int& bufferOffset, int minorVersion) const
{
    Serialize::Out(data, bufferOffset, rotation, minorVersion);

    return true;
}

void RotationCompressed::Set(const Vector3& eulerAngles)
{
    int repAngleX = Network::Utils::ConvertDegToQuantitized(eulerAngles.x, Network::Settings::Rotation::quantizationX);
    int repAngleY = Network::Utils::ConvertDegToQuantitized(eulerAngles.y, Network::Settings::Rotation::quantizationY);
    int repAngleZ = Network::Utils::ConvertDegToQuantitized(eulerAngles.z, Network::Settings::Rotation::quantizationZ);
    rotation = Pack(repAngleX, repAngleY, repAngleZ);
}
Vector3 RotationCompressed::Get()
{
    int x = 0, y = 0, z = 0;
    Unpack(rotation, x, y, z);
    return Vector3(Network::Utils::ConvertToDeg(x, Network::Settings::Rotation::quantizationX),
        Network::Utils::ConvertToDeg(y, Network::Settings::Rotation::quantizationY),
        Network::Utils::ConvertToDeg(z, Network::Settings::Rotation::quantizationZ));
}

int RotationCompressed::Pack(int x, int y, int z)
{
    int result = (x << (Network::Settings::Rotation::shiftX + Network::Settings::Rotation::shiftY)) + (y << Network::Settings::Rotation::shiftY) + z;
    return result;
}

void RotationCompressed::Unpack(int value, int& x, int& y, int& z)
{
    x = value & (Network::Settings::Rotation::maskX << (Network::Settings::Rotation::shiftY + Network::Settings::Rotation::shiftX));
    value -= x;
    x >>= Network::Settings::Rotation::shiftX + Network::Settings::Rotation::shiftY;

    y = value & (Network::Settings::Rotation::maskY << (Network::Settings::Rotation::shiftY));
    value -= y;
    y >>= Network::Settings::Rotation::shiftY;
    z = value;
}

///////////////////////////////////////////////////////////////

bool  PositionCompressed::SerializeIn(const U8* data, int& bufferOffset, int minorVersion)
{
    Serialize::In(data, bufferOffset, position, minorVersion);

    return true;
}
bool  PositionCompressed::SerializeOut(U8* data, int& bufferOffset, int minorVersion) const
{
    Serialize::Out(data, bufferOffset, position, minorVersion);

    return true;
}

void PositionCompressed::Set(const Vector3& pos)
{
    position = Pack(pos.x, pos.y, pos.z);
}
Vector3 PositionCompressed::Get()
{
    float x = 0, y = 0, z = 0;
    Unpack(position, x, y, z);
    return Vector3(x, y, z);
}
void PositionCompressed::Unpack(U64 position, float& x, float& y, float& z)
{
    float uX = static_cast<float>((position & Network::Settings::Position::xMask) >> Network::Settings::Position::xShift);
    uX /= static_cast<float>(Network::Settings::Position::precision);
    if ((position & 1 << 2) != 0)
        uX = -uX;

    x = (float)uX;

    float uY = static_cast<float>((position & Network::Settings::Position::yMask) >> Network::Settings::Position::yShift);
    uY /= static_cast<float>(Network::Settings::Position::precision);
    if ((position & 1 << 1) != 0)
        uY = -uY;

    y = (float)uY;

    float uZ = static_cast<float>((position & Network::Settings::Position::zMask) >> Network::Settings::Position::zShift);
    uZ /= static_cast<float>(Network::Settings::Position::precision);
    if ((position & 1 << 0) != 0)
        uZ = -uZ;

    z = (float)uZ;
}
U64 PositionCompressed::Pack(float x, float y, float z)
{
    bool xNeg = x < 0;
    bool yNeg = y < 0;
    bool zNeg = z < 0;

    if (xNeg) x = -x;
    if (yNeg) y = -y;
    if (zNeg) z = -z;

    U64 x0 = (long)(x * Network::Settings::Position::precision);
    U64 y0 = (long)(y * Network::Settings::Position::precision);
    U64 z0 = (long)(z * Network::Settings::Position::precision);

    U64 xPacked = (x0 << Network::Settings::Position::xShift) & Network::Settings::Position::xMask;
    U64 yPacked = (y0 << Network::Settings::Position::yShift) & Network::Settings::Position::yMask;
    U64 zPacked = (z0 << Network::Settings::Position::zShift) & Network::Settings::Position::zMask;

    long signs = 0;
    if (xNeg) signs |= 1 << 2;
    if (yNeg) signs |= 1 << 1;
    if (zNeg) signs |= 1 << 0;

    U64 result = xPacked | yPacked | zPacked | signs;
    return result;
}
///////////////////////////////////////////////////////////////
