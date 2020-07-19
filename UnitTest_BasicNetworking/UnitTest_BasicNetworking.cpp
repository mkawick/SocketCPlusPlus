//#include "../UnitTest_BasicNetworking/pch.h"
//#include "stdafx.h"
#include <CppUnitTest.h>
#include <cstdlib>
#include <iostream>
#include <set>
//using namespace std;

#include "../UDP01/OldCode/Packets/Serialize.h"
#include "../UDP01/OldCode/Packets/BasePacket.h"
#include "../UDP01/OldCode/Packets/MovementPacket.h"
#include "../UDP01/OldCode/Packets/PacketFactory.h"
#include "../UDP01/OldCode/Packets/MovementPacket.h"
#include "../UDP01/OldCode/Socket/Socket.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

class FactoryMock
{
public:
	FactoryMock() 
	{ 
		MockRegistery(BasePacket);
		MockRegistery(PositionPacket);
		MockRegistery(MovementPacket);

		PacketMethodFactory::InitFactory();
	}

	~FactoryMock()
	{
		PacketMethodFactory::Shutdown();
		BasePacket::s_typeRegistered = false;
		PositionPacket::s_typeRegistered = false;
		MovementPacket::s_typeRegistered = false;
	}

	shared_ptr<IPacketSerializable>  Create(string name)
	{
		return  PacketMethodFactory::Create(name);

		
	}
	shared_ptr<IPacketSerializable>  Create(int type, int subType )
	{
		return PacketMethodFactory::Create(type, subType);
	}
};
float Wrap(float value, float range)
{
	assert((value > -2000.f) && (value < 2000.0f));
	while (value < 0)
		value += range;
	while (value >= range)
		value -= range;
	return value;
}

float AbsDiffWithWrap(float x1, float x2, float wrap)
{
	return abs(Wrap(x1, wrap) - Wrap(x2, wrap));
}

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

		TEST_METHOD(TestRotationNormalizationAndQuantization)
		{
			//U8 buffer[100];

			float epsilon = 0.5f;
			PositionCompressed positionCompressed;
			RotationCompressed rotationCompressed;
			PositionCompressed moveCompressed;
			Vector3 positionTest(1, 20, -3);
			Vector3 rotationTest(20, 17, -20);
			Vector3 rotationTest2(0, 0, -20);
			float movX = 5, movY = 2, movZ = 0;
			//bp.Set(Vector3(posX, posY, posZ), Vector3(x, y, z), Vector3(movX, movY, movZ));
			Vector3 mov(movX, movY, movZ);
			
			positionCompressed.Set(positionTest);
			rotationCompressed.Set(rotationTest2);
			
			auto retRot = rotationCompressed.Get();

			Assert::AreEqual(retRot.x, rotationTest2.x);
			Assert::AreEqual(retRot.y, rotationTest2.y);
			//float diff = abs(Wrap(retRot.z, 360.0f) - Wrap(rotationTest2.z, 360.0f));
			Assert::IsTrue(AbsDiffWithWrap(retRot.z, rotationTest2.z, 360.0f) <epsilon);


			// check full range
			float largest = 0;
			epsilon = 0.71f;
			for (float z = 0; z < 365; z += 0.3f)
			{
				rotationTest2.z = z;
				rotationCompressed.Set(rotationTest2);
				retRot = rotationCompressed.Get();
				float diff = abs(Wrap(retRot.z, 360.0f) - Wrap(rotationTest2.z, 360.0f));
				if (diff > largest)
					largest = diff;
				if(diff>=epsilon)
					std::cout << "Largest error was :" << largest << std::endl;
				Assert::IsTrue(diff < epsilon);
			}
			std::cout<< "Largest error was :" << largest << std::endl;

			moveCompressed.Set(mov);
			auto retMov = moveCompressed.Get();
			Assert::IsTrue(abs(movX - retMov.x) < epsilon);
			Assert::IsTrue(abs(movX - retMov.x) < epsilon);
			Assert::IsTrue(abs(movX - retMov.x) < epsilon);
		}
	};

	TEST_CLASS(UnitTestBasicNetworking_BoilerplateRewrite)
	{
		TEST_METHOD(BasicBasePacketTest)
		{
			FactoryMock* mock = new FactoryMock(); // just to initialize
			U8 buffer[100];

			BasePacket bp;
			bp.packetSubType = BasePacket::SubType::BasePacket_Type;
			bp.gameProductId = 13;
			bp.versionNumberMajor = 56;
			bp.versionNumberMinor = 11;
			bp.gameInstanceId = 01;
			Assert::AreEqual((U8)bp.Type(), (U8)PacketType::PacketType_Base);
			Assert::AreEqual(bp.GetName(), string("BasePacket"));
			Assert::AreEqual(bp.GetSubType(), (U8)BasePacket::SubType::BasePacket_Type);
			Assert::IsTrue(bp.s_typeRegistered);
			

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

			delete mock;
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

		TEST_METHOD(BasicBasePacketTestPointer)
		{
			U8 buffer[100];

			BasePacket* bp = new BasePacket();
			bp->packetSubType = BasePacket::SubType::BasePacket_Hello;
			bp->gameProductId = 13;
			bp->versionNumberMajor = 56;
			bp->versionNumberMinor = 11;
			bp->gameInstanceId = 01;

			IPacketSerializable* ptr = bp;
			int outOffset = 0;
			Serialize::Out(buffer, outOffset, *ptr, 1);

			int inOffset = 0;
			BasePacket inPacket;
			Serialize::In(buffer, inOffset, inPacket, 1);

			Assert::AreEqual(inOffset, outOffset);
			Assert::AreEqual(bp->packetType, inPacket.packetType);
			Assert::AreEqual(bp->packetSubType, inPacket.packetSubType);
			Assert::AreEqual(bp->gameProductId, inPacket.gameProductId);
			Assert::AreEqual(bp->versionNumberMajor, inPacket.versionNumberMajor);
			Assert::AreEqual(bp->versionNumberMinor, inPacket.versionNumberMinor);
			Assert::AreEqual((int)bp->gameInstanceId, (int)inPacket.gameInstanceId);
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
			float rotEpsilon = 0.71f;
			inPacket.Get(resultPos, resultRot, resultMov);
			Assert::IsTrue(abs(posX - resultPos.x) < episilon);
			Assert::IsTrue(abs(posY - resultPos.y) < episilon);
			Assert::IsTrue(abs(posZ - resultPos.z) < episilon);
			Assert::IsTrue(abs(x - resultRot.x) < rotEpsilon);// abs(Wrap(retRot.z, 360.0f) - Wrap(rotationTest2.z, 360.0f));
			Assert::IsTrue(abs(y - resultRot.y) < rotEpsilon);
			Assert::IsTrue(abs(z - resultRot.z) < rotEpsilon);
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

			float rotEpsilon = 0.36f;
			Vector3 resultPos = inPositionPacket.positionCompressed.Get();
			Assert::IsTrue(abs(posX - resultPos.x) < episilon);
			Assert::IsTrue(abs(posY - resultPos.y) < episilon);
			Assert::IsTrue(abs(posZ - resultPos.z) < episilon);
			Vector3 result = inPositionPacket.rotationCompressed.Get();
			Assert::IsTrue(abs(x - result.x) < rotEpsilon);
			Assert::IsTrue(abs(y - result.y) < rotEpsilon);
			Assert::IsTrue(abs(z - result.z) < rotEpsilon);


			Assert::AreEqual(mp->packetType, inMovementPacket.packetType);
			Assert::AreEqual(mp->packetSubType, inMovementPacket.packetSubType);
			Assert::AreEqual(mp->gameProductId, inMovementPacket.gameProductId);
			Assert::AreEqual(mp->versionNumberMajor, inMovementPacket.versionNumberMajor);
			resultPos = inMovementPacket.positionCompressed.Get();
			Assert::IsTrue(abs(posX - resultPos.x) < episilon);
			Assert::IsTrue(abs(posY - resultPos.y) < episilon);
			Assert::IsTrue(abs(posZ - resultPos.z) < episilon);
			result = inMovementPacket.rotationCompressed.Get();
			Assert::IsTrue(abs(x - result.x) < rotEpsilon);
			Assert::IsTrue(abs(y - result.y) < rotEpsilon);
			Vector3 dirResult = inMovementPacket.movementDirCompressed.Get();
			Assert::IsTrue(abs(x - dirResult.x) < episilon);
			Assert::IsTrue(abs(y - dirResult.y) < episilon);


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
			float rotEpsilon = 0.36f;
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
				Assert::IsTrue(abs(x - result.x) < rotEpsilon);
				Assert::IsTrue(abs(y - result.y) < rotEpsilon);
				Assert::IsTrue(abs(z - result.z) < rotEpsilon);
				index++;
			}
		}
	};

	TEST_CLASS(UnitTestBasicNetworking_PacketFactory)
	{
		FactoryMock* mock;
		
		TEST_METHOD(FactoryTest_Basics)
		{
			mock = new FactoryMock();
			shared_ptr<IPacketSerializable> pack = PacketMethodFactory::Create("BasePacket");
			pack.get()->GetName();

			Assert::AreEqual((string)("BasePacket"), pack.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_Base, pack.get()->GetType());
			Assert::AreEqual((U8)BasePacket::BasePacket_Type, pack.get()->GetSubType());

			PacketMethodFactory::Release(pack);
			delete mock;
		}
		TEST_METHOD(FactoryTest_Basics2)
		{
			mock = new FactoryMock();
			shared_ptr<IPacketSerializable> pack = PacketMethodFactory::Create("BasePacket");

			Assert::AreEqual((string)("BasePacket"), pack.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_Base, pack.get()->GetType());
			Assert::AreEqual((U8)BasePacket::BasePacket_Type, pack.get()->GetSubType());

			shared_ptr<IPacketSerializable> pack3 = PacketMethodFactory::Create("PositionPacket");
			Assert::AreEqual((string)("PositionPacket"), pack3.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_ServerTick, pack3.get()->GetType());
			Assert::AreEqual((U8)ServerTickPacket::ServerTick_Position, pack3.get()->GetSubType());

			shared_ptr<IPacketSerializable> pack2 = PacketMethodFactory::Create("MovementPacket");
			Assert::AreEqual((string)("MovementPacket"), pack2.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_ServerTick, pack2.get()->GetType());
			Assert::AreEqual((U8)ServerTickPacket::ServerTick_Movement, pack2.get()->GetSubType());

			PacketMethodFactory::Release(pack3);
			PacketMethodFactory::Release(pack2);
			PacketMethodFactory::Release(pack);
			delete mock;
		}
		TEST_METHOD(FactoryTest_FactoryTest)
		{
			mock = new FactoryMock();
			shared_ptr<IPacketSerializable> pack = PacketMethodFactory::Create("BasePacket");

			Assert::AreEqual((string)("BasePacket"), pack.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_Base, pack.get()->GetType());
			Assert::AreEqual((U8)BasePacket::BasePacket_Type, pack.get()->GetSubType());

			shared_ptr<IPacketSerializable> pack2 = PacketMethodFactory::Create("PositionPacket");
			Assert::AreEqual((string)("PositionPacket"), pack2.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_ServerTick, pack2.get()->GetType());
			Assert::AreEqual((U8)ServerTickPacket::ServerTick_Position, pack2.get()->GetSubType());
			PositionPacket* pp = dynamic_cast<PositionPacket*>(pack2.get());

			float episilon = 0.001f;
			
			Vector3 positionTest(1, 20, -3);
			Vector3 rotationTest(20, 17, -20);
			pp->Set(positionTest, rotationTest);

			shared_ptr<IPacketSerializable> pack3 = PacketMethodFactory::Create("MovementPacket");
			Assert::AreEqual((string)("MovementPacket"), pack3.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_ServerTick, pack3.get()->GetType());
			Assert::AreEqual((U8)ServerTickPacket::ServerTick_Movement, pack3.get()->GetSubType());

			U8 buffer[100];
			int outOffset = 0;
			Serialize::Out(buffer, outOffset, *pack, 1);
			Serialize::Out(buffer, outOffset, *pack3.get(), 1);
			Serialize::Out(buffer, outOffset, *pack2.get(), 1);

			int inOffset = 0, sampleOffset = 0;
			BasePacket sampler;
			Serialize::In(buffer, sampleOffset, sampler, 1);
			shared_ptr<IPacketSerializable> unpack1 = PacketMethodFactory::Create(sampler.packetType, sampler.packetSubType);
			Serialize::In(buffer, inOffset, *unpack1, 1);
			sampleOffset = inOffset;
			Serialize::In(buffer, sampleOffset, sampler, 1);
			shared_ptr<IPacketSerializable> unpack2 = PacketMethodFactory::Create(sampler.packetType, sampler.packetSubType);
			Serialize::In(buffer, inOffset, *unpack2, 1);
			sampleOffset = inOffset;
			Serialize::In(buffer, sampleOffset, sampler, 1);
			shared_ptr<IPacketSerializable> unpack3 = PacketMethodFactory::Create(sampler.packetType, sampler.packetSubType);
			Serialize::In(buffer, inOffset, *unpack3, 1);
			sampleOffset = inOffset;

			Assert::AreEqual((string)("BasePacket"), unpack1.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_Base, unpack1.get()->GetType());
			Assert::AreEqual((U8)BasePacket::BasePacket_Type, unpack1.get()->GetSubType());
			Assert::AreEqual((string)("MovementPacket"), unpack2.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_ServerTick, unpack2.get()->GetType());
			Assert::AreEqual((U8)ServerTickPacket::ServerTick_Movement, unpack2.get()->GetSubType());
			Assert::AreEqual((string)("PositionPacket"), unpack3.get()->GetName());
			Assert::AreEqual((U8)PacketType::PacketType_ServerTick, unpack3.get()->GetType());
			Assert::AreEqual((U8)ServerTickPacket::ServerTick_Position, unpack3.get()->GetSubType());
			PositionPacket* pp2 = dynamic_cast<PositionPacket*>(unpack3.get());

			float rotEpsilon = 0.36f;
			Vector3 resultPos, resultRot;// , resultMov;
			pp2->Get(resultPos, resultRot);// , resultMov);
			Assert::IsTrue(abs(positionTest.x - resultPos.x) < episilon);
			Assert::IsTrue(abs(positionTest.y - resultPos.y) < episilon);
			Assert::IsTrue(abs(positionTest.z - resultPos.z) < episilon);
			Assert::IsTrue(AbsDiffWithWrap(rotationTest.x, resultRot.x, 360.0f) < rotEpsilon);
			Assert::IsTrue(AbsDiffWithWrap(rotationTest.y, resultRot.y, 360.0f) < rotEpsilon);
			Assert::IsTrue(AbsDiffWithWrap(rotationTest.z, resultRot.z, 360.0f) < rotEpsilon);
			/*Assert::IsTrue(abs(movX - resultMov.x) < episilon);
			Assert::IsTrue(abs(movY - resultMov.y) < episilon);
			Assert::IsTrue(abs(movZ - resultMov.z) < episilon);*/
			
			PacketMethodFactory::Release(unpack1);
			PacketMethodFactory::Release(unpack2);
			PacketMethodFactory::Release(unpack3);
			PacketMethodFactory::Release(pack3);
			PacketMethodFactory::Release(pack2);
			PacketMethodFactory::Release(pack);
			delete mock;
		}
	};

	TEST_CLASS(UnitTestBasicNetworking_PacketMemoryPools)
	{
		FactoryMock* mock;
		TEST_METHOD(FactoryTest_Basics_ValidateUniquenessAndProperPools)
		{
			mock = new FactoryMock();
			const int num = 100;
			shared_ptr<IPacketSerializable> packets [num];
			for (int i = 0; i < num; i++)
			{
				shared_ptr<IPacketSerializable> pack = PacketMethodFactory::Create("BasePacket");
				
				BasePacket* unpacked = dynamic_cast<BasePacket*>(pack->GetTypePtr());
				packets[i] = pack;
				unpacked->gameInstanceId = i;
			}
			// verfiy that every instance is unique
			std::set<int> instanceIds;
			BasePacket* bpTestInstance;
			for (int i = 0; i < num; i++)
			{
				BasePacket* bp = dynamic_cast<BasePacket*>(packets[i]->GetTypePtr());
				Assert::IsTrue(instanceIds.find(bp->gameInstanceId) == instanceIds.end());
				instanceIds.insert(bp->gameInstanceId);
				bpTestInstance = bp;
			}

			shared_ptr<IPacketSerializable> packets2[num];
			shared_ptr<IPacketSerializable> packets3[num];
			for (int i = 0; i < num; i++)
			{
				shared_ptr<IPacketSerializable> position = PacketMethodFactory::Create("PositionPacket");
				shared_ptr<IPacketSerializable> movement = PacketMethodFactory::Create("MovementPacket");
				PositionPacket* unpackedPosition = dynamic_cast<PositionPacket*>(position.get());
				MovementPacket* unpackedMove = dynamic_cast<MovementPacket*>(movement.get());
				packets2[i] = position;
				unpackedPosition->gameInstanceId = i+200;
				packets3[i] = movement;
				unpackedMove->gameInstanceId = i+400;
			}


			for (int i = 0; i < num; i++)
			{
				PositionPacket* bp = dynamic_cast<PositionPacket*>(packets2[i]->GetTypePtr());
				Assert::IsTrue(instanceIds.find(bp->gameInstanceId) == instanceIds.end());
				instanceIds.insert(bp->gameInstanceId);
				MovementPacket* bp2 = dynamic_cast<MovementPacket*>(packets3[i]->GetTypePtr());
				Assert::IsTrue(instanceIds.find(bp2->gameInstanceId) == instanceIds.end());
				instanceIds.insert(bp2->gameInstanceId);
			}

			for (auto pack : packets)
			{
				PacketMethodFactory::Release(pack);
				pack.reset();
			}
			for (auto pack : packets2)
			{
				PacketMethodFactory::Release(pack);
				pack.reset();
			}
			for (auto pack : packets3)
			{
				PacketMethodFactory::Release(pack);
				pack.reset();
			}

			// check bpTestInstance

			delete mock;
		}

		TEST_METHOD(FactoryTest_Basics_DeleteItemsInMiddleOFPools)
		{
			mock = new FactoryMock();
			const int num = 100;
			shared_ptr<IPacketSerializable> packets[num];
			for (int i = 0; i < num; i++)
			{
				shared_ptr<IPacketSerializable> pack = PacketMethodFactory::Create("BasePacket");

				BasePacket* unpacked = dynamic_cast<BasePacket*>(pack->GetTypePtr());
				packets[i] = pack;
				unpacked->gameInstanceId = i;
			}
			for (int i=45; i<55; i++) // delete the middle
			{
				PacketMethodFactory::Release(packets[i]);
				packets[i].reset();
			}
			for (int i = 45; i < 55; i++) // refil
			{
				shared_ptr<IPacketSerializable> pack = PacketMethodFactory::Create("BasePacket");
				BasePacket* unpacked = dynamic_cast<BasePacket*>(pack->GetTypePtr());
				packets[i] = pack;
				unpacked->gameInstanceId = i+100;
			}

			for (int i = 0; i < 100; i+=2) // delete every other one
			{
				PacketMethodFactory::Release(packets[i]);
				packets[i].reset();
			}
			for (int i = 0; i < 100; i+=2) // refil
			{
				shared_ptr<IPacketSerializable> pack = PacketMethodFactory::Create("BasePacket");
				BasePacket* unpacked = dynamic_cast<BasePacket*>(pack->GetTypePtr());
				packets[i] = pack;
				unpacked->gameInstanceId = i + 200;
			}
			cout << "-------------------------------" << endl;
			cout << "Post delete numbering" << endl;
			for (int i = 0; i < num; i++)
			{
				BasePacket* bp = dynamic_cast<BasePacket*>(packets[i]->GetTypePtr());
				cout << i << ":" << bp->gameInstanceId << endl;

			}

			cout << "-------------------------------" << endl;
		/*	for (int i = 0; i < num; i++)
			{
				shared_ptr<IPacketSerializable> pack = PacketMethodFactory::Create("BasePacket");

				BasePacket* unpacked = dynamic_cast<BasePacket*>(pack->GetTypePtr());
				packets[i] = pack;
				unpacked->gameInstanceId = i;
			}
			// verfiy that every instance is unique
			std::set<int> instanceIds;
			BasePacket* bpTestInstance;
			for (int i = 0; i < num; i++)
			{
				BasePacket* bp = dynamic_cast<BasePacket*>(packets[i]->GetTypePtr());
				Assert::IsTrue(instanceIds.find(bp->gameInstanceId) == instanceIds.end());
				instanceIds.insert(bp->gameInstanceId);
				bpTestInstance = bp;
			}

			shared_ptr<IPacketSerializable> packets2[num];
			shared_ptr<IPacketSerializable> packets3[num];
			for (int i = 0; i < num; i++)
			{
				shared_ptr<IPacketSerializable> position = PacketMethodFactory::Create("PositionPacket");
				shared_ptr<IPacketSerializable> movement = PacketMethodFactory::Create("MovementPacket");
				PositionPacket* unpackedPosition = dynamic_cast<PositionPacket*>(position.get());
				MovementPacket* unpackedMove = dynamic_cast<MovementPacket*>(movement.get());
				packets2[i] = position;
				unpackedPosition->gameInstanceId = i + 200;
				packets3[i] = movement;
				unpackedMove->gameInstanceId = i + 400;
			}


			for (int i = 0; i < num; i++)
			{
				PositionPacket* bp = dynamic_cast<PositionPacket*>(packets2[i]->GetTypePtr());
				Assert::IsTrue(instanceIds.find(bp->gameInstanceId) == instanceIds.end());
				instanceIds.insert(bp->gameInstanceId);
				MovementPacket* bp2 = dynamic_cast<MovementPacket*>(packets3[i]->GetTypePtr());
				Assert::IsTrue(instanceIds.find(bp2->gameInstanceId) == instanceIds.end());
				instanceIds.insert(bp2->gameInstanceId);
			}

			for (auto pack : packets)
			{
				PacketMethodFactory::Release(pack);
				pack.reset();
			}
			for (auto pack : packets2)
			{
				PacketMethodFactory::Release(pack);
				pack.reset();
			}
			for (auto pack : packets3)
			{
				PacketMethodFactory::Release(pack);
				pack.reset();
			}*/

			// check bpTestInstance

			delete mock;
		}
	};	

	TEST_CLASS(UnitTestBasicNetworking_StreamOfPackets)
	{
		FactoryMock* mock;
		TEST_METHOD(FactoryTest_Deserialize_1WrappedPacket)
		{
			mock = new FactoryMock();
			const int num = 100;

			shared_ptr<IPacketSerializable> pack = PacketMethodFactory::Create("BasePacket");
			BasePacket* bp = dynamic_cast<BasePacket*>(pack->GetTypePtr());
			bp->gameInstanceId = 13;
			bp->gameProductId = 51;

			SizePacket sp(pack);

			U8 buffer[100];

			int outOffset = 0;
			Serialize::Out(buffer, outOffset, sp, 1);

			int inOffset = 0;
			SizePacket sp2;
			if(sp2.IsRemainingBufferBigenough(buffer, inOffset, 1, num))
				Serialize::In(buffer, inOffset, sp2, 1);
			BasePacket* bp2 = dynamic_cast<BasePacket*>(sp2.packet->GetTypePtr()); 

			Assert::AreEqual(inOffset, outOffset);
			Assert::AreEqual(bp->packetType, bp2->packetType);
			Assert::AreEqual(bp->packetSubType, bp2->packetSubType);
			Assert::AreEqual(bp->gameProductId, bp2->gameProductId);
			Assert::AreEqual(bp->versionNumberMajor, bp2->versionNumberMajor);
			Assert::AreEqual(bp->versionNumberMinor, bp2->versionNumberMinor);
			Assert::AreEqual((int)bp->gameInstanceId, (int)bp2->gameInstanceId);

			delete mock;
		}
		TEST_METHOD(FactoryTest_Deserialize_WrappedPackets)
		{
			mock = new FactoryMock();
			const int num = 100;

			const int numToSend = 5;
			SizePacket sp[numToSend];
			shared_ptr<IPacketSerializable> pack[numToSend];
			int packetType[numToSend][2] = {
				{PacketType_Base, ServerTickPacket::BasePacket_Type},
				{PacketType_Base, ServerTickPacket::BasePacket_Type},
				{PacketType_ServerTick, ServerTickPacket::ServerTick_Position},
				{PacketType_ServerTick, ServerTickPacket::ServerTick_Movement},
				{PacketType_ServerTick, ServerTickPacket::ServerTick_Position}
			};
			
			for (int i = 0; i < numToSend; i++)
			{
				pack[i] = PacketMethodFactory::Create(packetType[i][0], packetType[i][1]);
				BasePacket* bp = dynamic_cast<BasePacket*>(pack[i]->GetTypePtr());
				bp->gameInstanceId = 13;
				bp->gameProductId = 51;
				sp[i].packet = pack[i];
			}

			U8 buffer[120];

			int outOffset = 0;
			Serialize::Out(buffer, outOffset, sp, 1);

			int inOffset = 0;
			int index = 0;
			SizePacket sp2;
			while (sp2.IsRemainingBufferBigenough(buffer, inOffset, 1, num) )
			{
				Serialize::In(buffer, inOffset, sp2, 1);
				BasePacket* bp2 = dynamic_cast<BasePacket*>(sp2.packet->GetTypePtr());
				BasePacket* bp = dynamic_cast<BasePacket*>(pack[index]->GetTypePtr());
				
				Assert::AreEqual(bp->packetType, bp2->packetType);
				Assert::AreEqual(bp->packetSubType, bp2->packetSubType);
				Assert::AreEqual(bp->gameProductId, bp2->gameProductId);
				Assert::AreEqual(bp->versionNumberMajor, bp2->versionNumberMajor);
				Assert::AreEqual(bp->versionNumberMinor, bp2->versionNumberMinor);
				Assert::AreEqual((int)bp->gameInstanceId, (int)bp2->gameInstanceId);

				
				PacketMethodFactory::Release(sp2.packet);
				sp2.packet.reset();
				index++;
			}
			Assert::AreEqual(inOffset, outOffset);
			for (int i = 0; i < numToSend; i++)
			{
				PacketMethodFactory::Release(pack[i]);
				pack[i].reset();
			}

			delete mock;
		}
	};

	TEST_CLASS(UnitTestBasicNetworking_TCpSocket)
	{
		//  launching a basic server
		TEST_METHOD(SocketTest_CreateServer)
		{
			// create started
			// listens
			// shuts down
		}

		// launching a basic client

		// client and server talk.
	};
}
