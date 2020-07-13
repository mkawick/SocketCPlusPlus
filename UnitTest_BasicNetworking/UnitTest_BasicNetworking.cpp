//#include "../UnitTest_BasicNetworking/pch.h"
//#include "stdafx.h"
#include <CppUnitTest.h>
#include <cstdlib>

#include "../UDP01/OldCode/Packets/Serialize.h"
#include "../UDP01/OldCode/Packets/BasePacket.h"
#include "../UDP01/OldCode/Packets/MovementPacket.h"
#include "../UDP01/OldCode/Packets/PacketFactory.h"
#include "../UDP01/OldCode/Packets/MovementPacket.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTestBasicNetworking
{
	TEST_CLASS(UnitTestBasicNetworking_Serialization)
	{
	public:
		
		TEST_METHOD(BasicU16Copy)
		{
			U8 buffer[100];

			U16 outValue = 0xFEAC;
			int outOffset = 0;
			Serialize::Out(buffer, outOffset, outValue, 1);
			Assert::AreEqual(outOffset, 2);
			Assert::AreEqual(buffer[0], (U8)0xFE);// byte order in windows
			Assert::AreEqual(buffer[1], (U8)0xAC);

			U16 inValue = 0;
			int inOffset = 0;
			Serialize::In(buffer, inOffset, inValue, 1);
			Assert::AreEqual(inOffset, 2);
			Assert::AreEqual((U32)inValue, (U32)outValue);
		}

		TEST_METHOD(MultipleU16Copy)
		{
			U8 buffer[100];
			const int arrayLen = 6;

			U16 outValue[arrayLen] = { 0xFEAC, 0xf1fc, 0x1234, 0xabcd, 0xbeef, 0x55aa };
			int outOffset = 0;
			Serialize::Out(buffer, outOffset, arrayLen, 1);// 4 bytes
			Serialize::Out(buffer, outOffset, outValue, 1);

			const int len = sizeof(outValue);
			int offset = sizeof(arrayLen);
			Assert::AreEqual(outOffset, len + offset);
			
			Assert::AreEqual(buffer[0 + offset], (U8)0xFE);// spot check
			Assert::AreEqual(buffer[1 + offset], (U8)0xAC);
			Assert::AreEqual(buffer[6 + offset], (U8)0xAB);
			Assert::AreEqual(buffer[7 + offset], (U8)0xCD);
			Assert::AreEqual(buffer[10 + offset], (U8)0x55);
			Assert::AreEqual(buffer[11 + offset], (U8)0xAA);

			//-----------------------------------------------------
			U16 inValue[arrayLen];
			int inOffset = 0;
			int inArraySize = 0;
			Serialize::In(buffer, inOffset, inArraySize, 1);
			Serialize::In(buffer, inOffset, inValue, 1);
			int readLen = sizeof(inArraySize) + sizeof(inValue);
			Assert::AreEqual(inOffset, readLen);
			Assert::AreEqual(inArraySize, arrayLen);

			for (int i = 0; i < arrayLen; i++)
			{
				Assert::AreEqual((U32)inValue[i], (U32)outValue[i]);
			}
		}

		TEST_METHOD(BasicMostSizesCopy)
		{
			U8 buffer[100];

			U8 outValue1 = 0xFE;
			U16 outValue2 = 0xFEAC;
			U32 outValue4 = 0xFEAC;
			U64 outValue8 = 0xFEAC;

			int outOffset = 0;
			Serialize::Out(buffer, outOffset, outValue1, 1);
			Serialize::Out(buffer, outOffset, outValue2, 1);
			Serialize::Out(buffer, outOffset, outValue4, 1);
			Serialize::Out(buffer, outOffset, outValue8, 1);
			Serialize::Out(buffer, outOffset, outValue1, 1);// mixed up a bit
			Serialize::Out(buffer, outOffset, outValue8, 1);
			Serialize::Out(buffer, outOffset, outValue4, 1);
			Serialize::Out(buffer, outOffset, outValue2, 1);
			
			int writeOffset = sizeof(outValue1) + sizeof(outValue2) + sizeof(outValue4) + sizeof(outValue8);
			Assert::AreEqual(outOffset, writeOffset * 2);
			Assert::AreEqual(buffer[0], (U8)0xFE);

			Assert::AreEqual(buffer[1], (U8)0xFE); // byte order in windows
			Assert::AreEqual(buffer[2], (U8)0xAC);
			Assert::AreEqual(buffer[7], (U8)0x00);
			Assert::AreEqual(buffer[8], (U8)0x00);
			Assert::AreEqual(buffer[13], (U8)0xFE);
			Assert::AreEqual(buffer[14], (U8)0xAC);
			Assert::AreEqual(buffer[15], (U8)0xFE);
			Assert::AreEqual(buffer[16], (U8)0x00);
			Assert::AreEqual(buffer[17], (U8)0x00);
			Assert::AreEqual(buffer[22], (U8)0xFE);
			Assert::AreEqual(buffer[23], (U8)0xAC);

			U8 inValue1 = 0;
			U16 inValue2 = 0;
			U32 inValue4 = 0;
			U64 inValue8 = 0;
			int inOffset = 0;
			Serialize::In(buffer, inOffset, inValue1, 1);
			Assert::AreEqual(inOffset, 1);
			Serialize::In(buffer, inOffset, inValue2, 1);
			Assert::AreEqual(inOffset, 3);
			Serialize::In(buffer, inOffset, inValue4, 1);
			Assert::AreEqual(inOffset, 7);
			Serialize::In(buffer, inOffset, inValue8, 1);
			Assert::AreEqual(inOffset, 15);
			Serialize::In(buffer, inOffset, inValue1, 1);
			Serialize::In(buffer, inOffset, inValue8, 1);
			Serialize::In(buffer, inOffset, inValue4, 1);
			Serialize::In(buffer, inOffset, inValue2, 1);
			Assert::AreEqual(inOffset, writeOffset*2);
			Assert::AreEqual(outValue1, inValue1);
			Assert::AreEqual((int)outValue2, (int)inValue2);
			Assert::AreEqual(outValue4, inValue4);
			Assert::AreEqual(outValue8, inValue8);
		}

		TEST_METHOD(BasicUStringCopy)
		{
			U8 buffer[MaxString];

			string outString1 = "test1";
			string outString2 = "what the hell are you doing";
			string outString3 = "eat my shorts";
			int outOffset = 0;
			Serialize::Out(buffer, outOffset, outString1, 1);
			int len = 2 + (int)outString1.size();
			Assert::AreEqual(outOffset, len);
			Serialize::Out(buffer, outOffset, outString2, 1);
			len += 2 + (int)outString2.size();
			Assert::AreEqual(outOffset, len);
			Serialize::Out(buffer, outOffset, outString3, 1);
			len += 2 + (int)outString3.size();
			Assert::AreEqual(outOffset, len);

			
			int inOffset = 0;
			string inString1, inString2, inString3;
			Serialize::In(buffer, inOffset, inString1, 1);
			len = 2 + (int)outString1.size();
			Assert::AreEqual(inOffset, len);
			Serialize::In(buffer, inOffset, inString2, 1);
			Serialize::In(buffer, inOffset, inString3, 1);
			len += 2 + (int)outString2.size();
			len += 2 + (int)outString3.size();
			Assert::AreEqual(inOffset, len);
			Assert::AreEqual(inString1, outString1); 
			Assert::AreEqual(inString2, outString2);
			Assert::AreEqual(inString3, outString3);
		}

		TEST_METHOD(BasicBoolCopy)
		{
			U8 buffer[100];

			bool outValue1 = true;
			bool outValue2 = false;
			bool outValue3 = true;
			bool outValue4 = true;
			int outOffset = 0;
			Serialize::Out(buffer, outOffset, outValue1, 1);
			Assert::AreEqual(outOffset, 1);
			Serialize::Out(buffer, outOffset, outValue2, 1);
			Serialize::Out(buffer, outOffset, outValue3, 1);
			Serialize::Out(buffer, outOffset, outValue4, 1);
			Assert::AreEqual(outOffset, 4);
			Assert::AreEqual(buffer[0], (U8)1);
			Assert::AreEqual(buffer[1], (U8)0);
			Assert::AreEqual(buffer[2], (U8)1);
			Assert::AreEqual(buffer[3], (U8)1);

			bool inValue[4];
			int inOffset = 0;
			Serialize::In(buffer, inOffset, inValue, 1);
			Assert::AreEqual(inOffset, 4);
			Assert::AreEqual(inValue[0], outValue1);
			Assert::AreEqual(inValue[1], outValue2);
			Assert::AreEqual(inValue[2], outValue3);
			Assert::AreEqual(inValue[3], outValue4);
		}
	};

	TEST_CLASS(UnitTestBasicNetworking_MovementPacketsAndArrays)
	{
	public:
		TEST_METHOD(BasicBasePacketTest)
		{
			U8 buffer[100];

			BasePacket bp;
			bp.packetSubType = BasePacket::SubType::BasePacket_Hello;
			bp.gameProductId = 13;
			bp.versionNumberMajor = 56;
			bp.versionNumberMinor = 11;
			bp.gameInstanceId = 01;

			int outOffset = 0;
			Serialize::Out(buffer, outOffset, bp, 1);

			int inOffset = 0;
			BasePacket inPacket;
			Serialize::In(buffer, inOffset, inPacket, 1);

 			Assert::AreEqual(inOffset, outOffset);
			Assert::AreEqual(bp.packetType, inPacket.packetType);
			Assert::AreEqual(bp.packetSubType, inPacket.packetSubType);
			Assert::AreEqual(bp.gameProductId, inPacket.gameProductId);
			Assert::AreEqual(bp.versionNumberMajor, inPacket.versionNumberMajor);
			Assert::AreEqual(bp.versionNumberMinor, inPacket.versionNumberMinor);
			Assert::AreEqual((int)bp.gameInstanceId, (int)inPacket.gameInstanceId);
		}

		TEST_METHOD(BasicMovementPacketTest)
		{
		//todo: needs unit tests for set, get, and serializing the magnitude of movement

			U8 buffer[100];
			float episilon = 0.001f;
			MovementPacket bp;
			bp.packetSubType = BasePacket::SubType::BasePacket_Hello;
			bp.gameProductId = 13;
			bp.versionNumberMajor = 56;
			bp.versionNumberMinor = 11;
			bp.gameInstanceId = 01;
			float x = 18, y = 32, z = 270;// to pass the tests, we need positive angles. rotation compressed converts everything into 0-511
			float posX = 1, posY = 2, posZ = -3;
			float movX = 5, movY = 2, movZ = 0;
			bp.Set(Vector3(posX, posY, posZ), Vector3(x, y, z), Vector3(movX, movY, movZ));
			bp.serverTick = 25;

			int outOffset = 0;
			Serialize::Out(buffer, outOffset, bp, 1);

			int inOffset = 0;
			MovementPacket inPacket;
			Serialize::In(buffer, inOffset, inPacket, 1);

			Assert::AreEqual(inOffset, outOffset);
			Assert::AreEqual(bp.packetType, inPacket.packetType);
			Assert::AreEqual(bp.packetSubType, inPacket.packetSubType);
			Assert::AreEqual(bp.gameProductId, inPacket.gameProductId);
			Assert::AreEqual(bp.versionNumberMajor, inPacket.versionNumberMajor);
			Assert::AreEqual(bp.versionNumberMinor, inPacket.versionNumberMinor);
			Assert::AreEqual((int)bp.gameInstanceId, (int)inPacket.gameInstanceId);
			Assert::AreEqual((U32)bp.serverTick, (U32)inPacket.serverTick);

			//float outx, outy, outz;
			Vector3 resultPos, resultRot, resultMov;
			inPacket.Get(resultPos, resultRot, resultMov);
			Assert::IsTrue(abs(posX - resultPos.x) < episilon);
			Assert::IsTrue(abs(posY - resultPos.y) < episilon);
			Assert::IsTrue(abs(posZ - resultPos.z) < episilon);
			Assert::IsTrue(abs(x - resultRot.x) < episilon);
			Assert::IsTrue(abs(y - resultRot.y) < episilon);
			Assert::IsTrue(abs(z - resultRot.z) < episilon);
			Assert::IsTrue(abs(movX - resultMov.x) < episilon);
			Assert::IsTrue(abs(movY - resultMov.y) < episilon);
			Assert::IsTrue(abs(movZ - resultMov.z) < episilon);
		}

		TEST_METHOD(Basic_PacketHeirarchyAndPointerTest)
		{
			BasePacket* bp = new BasePacket();
			Assert::AreEqual(bp->packetType, (U8)PacketType::PacketType_Base);
			Assert::AreEqual(bp->packetSubType, (U8)BasePacket::SubType::BasePacket_Type);

			PositionPacket* pp = new PositionPacket();
			Assert::AreEqual(pp->packetType, (U8)PacketType::PacketType_ServerTick);
			Assert::AreEqual(pp->packetSubType, (U8)ServerTickPacket::SubType::ServerTick_Position);

			MovementPacket* mp = new MovementPacket(); 
			Assert::AreEqual(mp->packetType, (U8)PacketType::PacketType_ServerTick);
			Assert::AreEqual(mp->packetSubType, (U8)ServerTickPacket::SubType::ServerTick_Movement);
						

			U8 buffer[100];
			float episilon = 0.001f;
			bp->gameProductId = 13;
			bp->versionNumberMajor = 56;
			bp->versionNumberMinor = 11;
			bp->gameInstanceId = 01;

			pp->CopyFrom(*bp);
			bp->WriteTo(mp);
			Assert::AreEqual(bp->gameProductId, pp->gameProductId);
			Assert::AreEqual(bp->gameProductId, mp->gameProductId);
			Assert::AreEqual(bp->versionNumberMinor, pp->versionNumberMinor);
			Assert::AreEqual(bp->versionNumberMinor, mp->versionNumberMinor);

			float x = 18, y = 32, z = 270;// to pass the tests, we need positive angles. rotation compressed converts everything into 0-511
			pp->rotationCompressed.Set(Vector3(x, y, z));
			mp->rotationCompressed.Set(Vector3(x, y, z));
			mp->movementDirCompressed.Set(Vector3(x, y, z));

			float posX = 1, posY = 2, posZ = -3;
			pp->positionCompressed.Set(Vector3(posX, posY, posZ));
			mp->positionCompressed.Set(Vector3(posX, posY, posZ));
			

			int outOffset = 0;
			Serialize::Out(buffer, outOffset, *bp, 1);
			Serialize::Out(buffer, outOffset, *pp, 1);
			Serialize::Out(buffer, outOffset, *mp, 1);

			int inOffset = 0;
			BasePacket inPacket;
			PositionPacket inPositionPacket;
			MovementPacket inMovementPacket;
			Serialize::In(buffer, inOffset, inPacket, 1);
			Serialize::In(buffer, inOffset, inPositionPacket, 1);
			Serialize::In(buffer, inOffset, inMovementPacket, 1);

			Assert::AreEqual(inOffset, outOffset);
			Assert::AreEqual(bp->packetType, inPacket.packetType);
			Assert::AreEqual(bp->packetSubType, inPacket.packetSubType);
			Assert::AreEqual(bp->gameProductId, inPacket.gameProductId);
			Assert::AreEqual(bp->versionNumberMajor, inPacket.versionNumberMajor);

			Assert::AreEqual(pp->packetType, inPositionPacket.packetType);
			Assert::AreEqual(pp->packetSubType, inPositionPacket.packetSubType);
			Assert::AreEqual(pp->gameProductId, inPositionPacket.gameProductId);
			Assert::AreEqual(pp->versionNumberMajor, inPositionPacket.versionNumberMajor);


			Vector3 resultPos = inPositionPacket.positionCompressed.Get();
			Assert::IsTrue(abs(posX - resultPos.x) < episilon);
			Assert::IsTrue(abs(posY - resultPos.y) < episilon);
			Assert::IsTrue(abs(posZ - resultPos.z) < episilon);
			Vector3 result = inPositionPacket.rotationCompressed.Get();
			Assert::IsTrue(abs(x - result.x) < episilon);
			Assert::IsTrue(abs(y - result.y) < episilon);
			Assert::IsTrue(abs(z - result.z) < episilon);


			Assert::AreEqual(mp->packetType, inMovementPacket.packetType);
			Assert::AreEqual(mp->packetSubType, inMovementPacket.packetSubType);
			Assert::AreEqual(mp->gameProductId, inMovementPacket.gameProductId);
			Assert::AreEqual(mp->versionNumberMajor, inMovementPacket.versionNumberMajor);
			resultPos = inMovementPacket.positionCompressed.Get();
			Assert::IsTrue(abs(posX - resultPos.x) < episilon);
			Assert::IsTrue(abs(posY - resultPos.y) < episilon);
			Assert::IsTrue(abs(posZ - resultPos.z) < episilon);
			result = inMovementPacket.rotationCompressed.Get();
			Assert::IsTrue(abs(x - result.x) < episilon);
			Assert::IsTrue(abs(y - result.y) < episilon);
			Vector3 dirResult = inMovementPacket.rotationCompressed.Get();
			Assert::IsTrue(abs(x - result.x) < episilon);
			Assert::IsTrue(abs(y - result.y) < episilon);


			delete bp, pp, mp;
		}

		TEST_METHOD(MultipleMovementPacketTest)
		{
			U8 buffer[120];

			const int numPackets = 4;

			MovementPacket bp[numPackets];
			float episilon = 0.001f;
			float x = 18, y = 32, z = 270;// to pass the tests, we need positive angles. rotation compressed converts everything into 0-511
			float posX = 1, posY = 2, posZ = -3;

			for(auto& i:bp)
			{
				i.packetSubType = BasePacket::SubType::BasePacket_Hello;
				i.gameProductId = 13;
				i.versionNumberMajor = 56;
				i.versionNumberMinor = 11;
				i.gameInstanceId = 01;
				i.rotationCompressed.Set(Vector3(x, y, z));				
				i.positionCompressed.Set(Vector3(posX, posY, posZ));
			}

			int outOffset = 0;
			Serialize::Out(buffer, outOffset, bp, 1);

			int inOffset = 0;
			MovementPacket inPacket[numPackets];
			Serialize::In(buffer, inOffset, inPacket, 1);

			int index = 0;
			for (auto in : inPacket)
			{
				Assert::AreEqual(inOffset, outOffset);
				Assert::AreEqual(bp[index].packetType, in.packetType);
				Assert::AreEqual(bp[index].packetSubType, in.packetSubType);
				Assert::AreEqual(bp[index].gameProductId, in.gameProductId);
				Assert::AreEqual(bp[index].versionNumberMajor, in.versionNumberMajor);
				Assert::AreEqual(bp[index].versionNumberMinor, in.versionNumberMinor);
				Assert::AreEqual((int)bp[index].gameInstanceId, (int)in.gameInstanceId);

				//float outx, outy, outz;
				Vector3 resultPos = in.positionCompressed.Get();
				Assert::IsTrue(abs(posX - resultPos.x) < episilon);
				Assert::IsTrue(abs(posY - resultPos.y) < episilon);
				Assert::IsTrue(abs(posZ - resultPos.z) < episilon);
				Vector3 result = in.rotationCompressed.Get();
				Assert::IsTrue(abs(x - result.x) < episilon);
				Assert::IsTrue(abs(y - result.y) < episilon);
				Assert::IsTrue(abs(z - result.z) < episilon);
				index++;
			}
		}
	};


	TEST_CLASS(UnitTestBasicNetworking_PacketFactory)
	{
	};
	TEST_CLASS(UnitTestBasicNetworking_PacketMemoryPools)
	{
	};
	TEST_CLASS(UnitTestBasicNetworking_PacketWrappers)
	{
	};

	TEST_CLASS(UnitTestBasicNetworking_StreamOfPackets)
	{
	};

	TEST_CLASS(UnitTestBasicNetworking_UDPSocket)
	{
	};
}
