// cmd-chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <string>
#include <iostream>
#include <sstream>
//#include <string>
//#include <stdlib.h>
//#define DEFAULT_PORT 27015
#define DEFAULT_BUFLEN 512


#pragma comment(lib, "Ws2_32.lib")

int main()
{
	// At home you need to put the listening and typing into different threads. 
	// You need to also figure out how to check if the port is already taken.
	// You also need to create some kind of peers list to send them messages directly. 
	//It should be easy to implement with recvFrom Sendto functionality. 

	// Start the app
	// Create a socket on free port that you will use later
	// Start listen to who? everybody? 
	// Broadcast a message to network that new member with IP:port added to the chat
	// On main thread listen to keyboard and send messages on enter
	// On separate thread listen for messages that arrive on your socket
	// 
    //std::cout << "Chat started running!\n";
	std::cout << ("Chat started running!\n");

	WSADATA wsaData;

	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed:" << iResult << std::endl;
		return 1;
	}

	struct addrinfo* getAddrInfoResult = nullptr;
	struct addrinfo* ptr = nullptr;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;

	//#define SOCK_STREAM     1               /* stream socket */
	//#define SOCK_DGRAM      2               /* datagram socket */
	hints.ai_socktype = SOCK_DGRAM; // #MAYLEADTOPROBLEMS
	hints.ai_protocol = IPPROTO_UDP; // #MAYLEADTOPROBLEMS
	//hints.ai_flags = AI_PASSIVE; // #MAYLEADTOPROBLEMS

	//std::string nicknameResponse;
	//std::cout << "Please enter your nickname before entering the chat" << std::endl;
	//std::getline(std::cin, nicknameResponse); // #TODO Use this later 

	u_short defaultPort = 27154;

	do
	{
		std::ostringstream oss;
		oss << defaultPort;
		iResult = getaddrinfo(NULL, oss.str().c_str(), &hints, &getAddrInfoResult);
		if (iResult != 0)
		{
			defaultPort++;
		}
	} while (iResult != 0);
	 // #TODO 1) Seek for first available port 2) I will need another way to get the ip.
	//if (iResult != 0) 
	//{
	//	std::cout << "getaddrinfo failed:" << iResult << std::endl;
	//	WSACleanup(); // #TODO Transfer such function into destructor of the winSocket wrapper
	//	return 1;
	//}

	SOCKET BroadcastingSocket = INVALID_SOCKET;

	// Attempt to connect to the first address returned by
	// the call to getaddrinfo
	ptr = getAddrInfoResult;

	// Create a socket for connecting to server
	BroadcastingSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (BroadcastingSocket == INVALID_SOCKET)
	{
		std::cout << "Error at socket" << WSAGetLastError() << std::endl;
		freeaddrinfo(getAddrInfoResult);
		WSACleanup();
		std::cin.get();
		return 1;
	}

	const char broadcastOptionValue = '1';

	if (setsockopt(BroadcastingSocket, SOL_SOCKET, SO_BROADCAST, &broadcastOptionValue, sizeof(broadcastOptionValue)) < 0)
	{
		std::cout << "Error in setting Broadcast option";
		closesocket(BroadcastingSocket);
		std::cin.get();
		return 1;
	}


	std::string sendMSG;// = "Hello msg";
	std::getline(std::cin, sendMSG);

	const int recvBufferLength = 50;
	char recvBuffer[recvBufferLength] = "";

	struct sockaddr_in RecvAddress;
	RecvAddress.sin_family = ptr->ai_family;
	RecvAddress.sin_port = htons(defaultPort);
	RecvAddress.sin_addr.s_addr = INADDR_ANY; // OR INADDR_ANY, NEED to think

	if (bind(BroadcastingSocket, (sockaddr*)&RecvAddress, sizeof(RecvAddress)) < 0)
	{
		std::cout << "Error in BINDING" << WSAGetLastError();
		std::cout << "Chosen port is " << defaultPort;


		closesocket(BroadcastingSocket);
		std::cin.get();
		return 1;
	}

	struct sockaddr_in BroadcastAddress;
	BroadcastAddress.sin_family = ptr->ai_family;
	BroadcastAddress.sin_port = htons(defaultPort);
	BroadcastAddress.sin_addr.s_addr = INADDR_BROADCAST; // OR INADDR_ANY, NEED to think

	iResult = sendto(BroadcastingSocket, sendMSG.c_str(), sendMSG.length() + 1, 0, (sockaddr*)&BroadcastAddress, sizeof(BroadcastAddress));
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "Send failed: " << WSAGetLastError() << std::endl;
		closesocket(BroadcastingSocket);
		WSACleanup();
		std::cin.get();
		return 1;
	}

	struct sockaddr_in SenderAddress;
	int len = sizeof(struct sockaddr_in);

	//do
	//{
	{
		iResult = recvfrom(BroadcastingSocket, recvBuffer, recvBufferLength, 0, (sockaddr*)&SenderAddress, &len);
		if (iResult > 0)
		{
			std::cout << "Bytes received: " << iResult << std::endl;
		}
		else if (iResult == 0)
		{
			std::cout << "Connection closed" << iResult << std::endl;
		}
		else
		{
			std::cout << "recv failed:: " << WSAGetLastError() << std::endl;
		}
	}
	//} while (iResult > 0);

	std::cout << "\n\n\tReceived message from Reader is =>  " << recvBuffer;

	std::cout << "\n\n\tpress any key to CONTINUE...";

	//std::string answer;
	std::cin.get();// >> answer;

	closesocket(BroadcastingSocket);
	WSACleanup();
	//// Connect to "server"
	//iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	//if (iResult == SOCKET_ERROR)
	//{
	//	closesocket(ConnectSocket);
	//	ConnectSocket = INVALID_SOCKET;
	//}

	/*
	#TODO
	#MAYLEADTOPROBLEMS
	The getaddrinfo function is used to determine the values in the sockaddr structure. In this example, the first IP address returned by the getaddrinfo function is used to specify the sockaddr structure passed to the connect. If the connect call fails to the first IP address, then try the next addrinfo structure in the linked list returned from the getaddrinfo function.

	The information specified in the sockaddr structure includes:

	the IP address of the server that the client will try to connect to.
	the port number on the server that the client will connect to. 
	This port was specified as port 27015 when the client called the getaddrinfo function.
	*/

	//freeaddrinfo(result);


	//if (ConnectSocket == INVALID_SOCKET)
	//{
	//	std::cout << "Unable to connect to server!" << std::endl;
	//	WSACleanup();
	//	return 1;
	//}

	////////////////////////////////////////////////////////////
	//// Sending and Receiving Data on the Client
	////////////////////////////////////////////////////////////
	//int recvBufferLength = DEFAULT_BUFLEN;

	//const char* sendBuffer = "This is a test";

	//char recvBuffer[DEFAULT_BUFLEN];

	//iResult = send(ConnectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
	//if (iResult == SOCKET_ERROR)
	//{
	//	std::cout << "Send failed: " << WSAGetLastError() << std::endl;
	//	closesocket(ConnectSocket);
	//	WSACleanup();
	//	return 1;
	//}

	//std::cout << "Bytes sent: " << iResult;

	//// shutdown the connection for sending since no more data will be sent
	//// the client can still use the ConnectSocket for receiving data
	//iResult = shutdown(ConnectSocket, SD_SEND);
	//if (iResult == SOCKET_ERROR)
	//{
	//	std::cout << "Shutdown failed: " << WSAGetLastError() << std::endl;
	//	closesocket(ConnectSocket);
	//	WSACleanup();
	//	return 1;
	//}

	//do
	//{
	//	iResult = recv(ConnectSocket, recvBuffer, recvBufferLength, 0);
	//	if (iResult > 0)
	//	{
	//		std::cout << "Bytes received: " << iResult << std::endl;
	//	}
	//	else if (iResult == 0)
	//	{
	//		std::cout << "Connection closed" << iResult << std::endl;
	//	}
	//	else
	//	{
	//		std::cout << "recv failed:: " << WSAGetLastError() << std::endl;
	//	}
	//} while (iResult > 0);

	/////////////////////////////////////////////
	//// Disconnecting the Client
	////////////////////////////////////////////
	//// cleanup
	//closesocket(ConnectSocket);
	//WSACleanup();

	//return 0; // #MAYLEADTOPROBLEMS

	/////////////////////////////////////////////
	//// Server
	/////////////////////////////////////////////


	return 0;

	// Disconnect on app close
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
