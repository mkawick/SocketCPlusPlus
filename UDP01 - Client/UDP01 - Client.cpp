// UDP01 - Client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include<iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include<sys/types.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
using namespace std;
#include<stdio.h>

#include "../Common/Connection.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable:4996)

void error(char* msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void Init()
{
	WSADATA wsa;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");
}

SOCKET CreateSocket()
{	
	SOCKET socketCreated;
	if ((socketCreated = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	return socketCreated;
}
int main()
{
	//const int BUFLEN = 256;
	struct sockaddr_in si_other;
	int slen = sizeof(si_other);
	Init();

	//create socket
	SOCKET socketCreated = CreateSocket();

	//setup address structure
	memset((char*)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(5150);
	si_other.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	CommsStateMachine sm(false);

	const int bufferSize = 1024;
	char buffer[bufferSize];

	while(1)
	{
		while (sm.ReadyToSend() == false)
			Sleep(10);
		
		int bytesUsed = 0;
		bool success = sm.DataSend(buffer, bufferSize, bytesUsed);
		//strcpy_s(message, "hello there");

		if (sendto(socketCreated, buffer, bytesUsed, 0, (struct sockaddr*) & si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//memset(buf, '\0', BUFLEN);
		//try to receive some data, this is a blocking call
		if (sm.IsExpectingReceive())
		{
			// we need a select here
			if (recvfrom(socketCreated, buffer, bufferSize, 0, (struct sockaddr*) & si_other, &slen) == SOCKET_ERROR)
			{
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			sm.DataReceive(buffer, slen);
		}
	}
		
	//start communication
	/*while (1)
	{
		//printf("Enter message : ");
		//gets(message);

		//send the message
		

		//receive a reply and print it
		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		//try to receive some data, this is a blocking call
		if (recvfrom(socketCreated, buf, BUFLEN, 0, (struct sockaddr*) & si_other, &slen) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		puts(buf);
	}*/

	closesocket(socketCreated);
	WSACleanup();

	return 0;
}
