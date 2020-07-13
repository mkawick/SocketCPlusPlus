// NetworkUtils.h

#pragma once

namespace Network
{
	bool	InitializeSockets();
	void	ShutdownSockets();
	int		SetSocketToNonblock(int ListenSocket);
	void	DisableNagle(int socketId);
	void	GetLocalIpAddress(char* buffer, size_t buflen);
	bool	IsPortBusy(int port);

    namespace Utils
    {
        static bool IsPowerOfTwo(int x)
        {
            return (x & (x - 1)) == 0;
        }
        static float ConvertToDeg(int value, int range)
        {
            float angle = (float)value / (float)range;
            return angle * 512;
        }
        static int ConvertRange(float value, int range)
        {
            int repAngle = (int)(value * range);
            repAngle %= range;
            if (repAngle < 0)
                repAngle += range;
            return repAngle;
        }
        static int ConvertDegToQuantitized(float value, int quantitization)
        {
            float deg = value / 512;

            int repAngle = ConvertRange(deg, quantitization);
            return repAngle;
        }
        static int GetBitPosition(int input)
        {
            for (int i = 0; i < 32; i++)
            {
                if ((input & (1 << i)) != 0)
                    return i;
            }
            return -1;
        }
    }
    namespace Settings
    {
        namespace Rotation
        {
            const int quantizationX = 512;
            const int quantizationY = 512;
            const int quantizationZ = 512;

            const int shiftX = Network::Utils::GetBitPosition(quantizationY);
            const int shiftY = Network::Utils::GetBitPosition(quantizationZ);
            const int maskZ = quantizationZ - 1, maskY = quantizationY - 1, maskX = quantizationX - 1;

        };
        namespace Float
        {
            const float shift = 512;
        };

        namespace Position
        {
            const int precision = 256;
            const int xShift = 43;
            const int yShift = 23;
            const int zShift = 3;
            /*      public const long xMask = 0x7FFFF0000000000;
                    public const long yMask = 0x00000FFFFF00000;
                    public const long zMask = 0x0000000000FFFF8;*/
            const U64 TwentyBits = 0xfffff;//2^20-1
            const U64 xMask = TwentyBits << xShift;
            const U64 yMask = TwentyBits << yShift;
            const U64 zMask = TwentyBits << zShift;
        };
    }
}