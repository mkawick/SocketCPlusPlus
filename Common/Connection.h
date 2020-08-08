//////// Connection.h

#include "../UDP01/OldCode/Packets/CommonTypes.h"
#include "../UDP01/OldCode/DataTypes.h"
////////////////////////////////////////////////////

class ConnectionCommunicationsPolicy
{};

////////////////////////////////////////////////////

class CommsStateMachine
{
public:
    enum states {
        SendHandshake, 
        SendConnectionId,
        SendConnectionPolicies,
        SendChannels,
        SendTopology,
        NormalSend,
        ConnectionFailed
    };

    const U8 serverId = 0x08;
    const U8 clientId = 0x04;
    const U8 sentinel = 0xAA;
    const char* uniqueLibraryId = "IOIC";
public :
    states  currentState;
    bool    iniates;
    bool    hasSentState;
    U16     connId;
public:
    CommsStateMachine(bool isServer): currentState(SendHandshake), iniates(!isServer), hasSentState(false)
    {}

    bool DataSend(char* buffer, int numBytes, int& bytesUsed )
    {
        const char* ptr = buffer;
        bytesUsed = 0;
        if (currentState == states::SendHandshake)// we will need retry
        {
            if (numBytes < 6)
            {
                currentState = states::ConnectionFailed;
                return false;
            }
            WriteProtocolSentinel(buffer);
            strcpy_s(buffer, numBytes-2, uniqueLibraryId);// We want to send a unique string
            hasSentState = true;
            bytesUsed = 6;
            return true;
        }
        if (currentState == SendConnectionId)// we will need retry
        {

        }

        return true;
    }
    bool DataReceive(const char* buffer, int numBytes)
    {
        if (currentState == states::SendHandshake)
        {
            if (numBytes < 6)
            {
                currentState = states::ConnectionFailed;
                return false;
            }
            if (ReadProtocolSentinel(buffer) == false)
                return false;
            if (strncmp(uniqueLibraryId, buffer, strlen(uniqueLibraryId)) != 0)
            {
                currentState = states::ConnectionFailed;
                return false;
            }
            // if I am the server, then we can move onto the next state
            if (iniates)// client round trip complete. wait for connectionId
            {
                currentState = states::SendConnectionId;
            }
            else
            {
                currentState = states::SendConnectionId;
                hasSentState = false;
            }
            /*  if (val2 != serverId)
                currentState = ConnectionFailed;
            return false;**/
        }

        return true;
    }

private:
    void WriteProtocolSentinel(char*& buffer)
    {
        *buffer++ = sentinel;
        if (iniates == true)
        {
            *buffer++ = clientId;
        }
        else // this will be a response 
        {
            *buffer++ = serverId;
        }
    }

    bool ReadProtocolSentinel(const char*& buffer)
    {
        U8 val1 = *buffer++;
        U8 val2 = *buffer++;
        if (val1 != sentinel)
        {
            currentState = states::ConnectionFailed;
            return false;
        }

        if (iniates)// client
        {
            if (val2 != serverId)
            {
                currentState = states::ConnectionFailed;
                return false;
            }
        }
        else // this will be a response 
        {
            if (val2 != clientId)
            {
                currentState = states::ConnectionFailed;
                return false;
            }
        }

        return true;
    }
    

public:
    //-------------------------------
    void Update(){}
    bool ReadyToSend() { return !hasSentState; }

    bool IsExpectingReceive() { return true; }
};

////////////////////////////////////////////////////

class Connection
{
public:
    Connection(SOCKET socket, SOCKADDR_IN S_addr, U16 port) : Socket(socket), SenderAddr(S_addr), Port(port), stateMachine(true)
    {
        testWrite = false;
    }
    void SetPolicy(ConnectionCommunicationsPolicy policy)
    {

    }
    void    HandleDataIn(const char* dataIn, int bufferLen)
    {
        stateMachine.DataReceive(dataIn, bufferLen);
        //testWrite = true;
    }
    bool    NeedsService() 
    {
        if (stateMachine.ReadyToSend() || stateMachine.IsExpectingReceive())
            return true;
        return false; 
    }
    void    RunService()
    {
        /* if (testWrite == false)
             return;*/
        const int bufferSize = 1024;
        char buffer[bufferSize];

        if (stateMachine.ReadyToSend())
        {
            int bytesUsed = 0;
            stateMachine.DataSend(buffer, bufferSize, bytesUsed);
            int errorType = sendto(Socket, buffer, bufferSize, 0, (const SOCKADDR*)&SenderAddr, sizeof(SenderAddr));
        }

     /*   if (stateMachine.IsExpectingReceive())
        {

        }*/

        // testWrite = false;
    }
    bool HasExpired() {
        return false;
    }
    void Cleanup() {
        // release pending outgoing messages
    }

private:
    SOCKADDR_IN         SenderAddr;
    U16                 Port;
    SOCKET              Socket;

    bool                testWrite;
    bool                initiatesComms;
    CommsStateMachine   stateMachine;
};