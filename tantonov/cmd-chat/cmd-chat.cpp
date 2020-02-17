// cmd-chat.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// #TODO remove includes
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <string>
#include <iostream>
#include <sstream>

#include "WinSocketManager.h"


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

	WinSocketManager& socketManager = WinSocketManager::GetInstance();

	socketManager.Initialize();

	socketManager.CreateUDPSocket();

	std::string helloMessage = "tantonov-chat";
	socketManager.LaunchReceiverThread();
	socketManager.BroadcastHelloMessage(helloMessage);

	while (true)
	{
		socketManager.ProcessUserInput();
	}

	//const unsigned int hostNameBufferLength = 50;
	//char hostNameBuffer[hostNameBufferLength];
	//iResult = gethostname(hostNameBuffer, hostNameBufferLength);
	//if (iResult != 0)
	//{
	//	std::cout << "gethostname failed: " << iResult << std::endl;
	//	WSACleanup();
	//	std::cin.get();
	//	return 1;
	//}
	//std::string messageToSend = hostNameBuffer;// = "Hello msg";
	//messageToSend += " connected";
	//std::getline(std::cin, messageToSend);
	//std::string messageToSend = "Hello chat!";



	// Now I need to do two things
	// As a first thought I need to start a thread that will bind a socket to the ip port. and recvfrom any addr for incoming connections. It will remember all peers IP+port that sent info to me, and also display messages to the console. 
	// My main thread will be blocked by std::cin and on enter will send the packet to list of peers listed in the listeners list. 
	
	//struct sockaddr_in SenderAddress;
	//int len = sizeof(struct sockaddr_in);

	//const int recvbufferlength = 50;
	//char recvbuffer[recvbufferlength] = "";
	


	//do
	//{
	//Add super lambda that will capture everyting that it needs and will just recvFrom. the only problem stays is mutexes and adding a peers to the list of peers
	//{
	//	iResult = recvfrom(ChatSocket, recvBuffer, recvBufferLength, 0, (sockaddr*)&SenderAddress, &len);
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
	//}
	////} while (iResult > 0);

	//std::cout << "\n\n\tReceived message from Reader is =>  " << recvBuffer;

	//std::cout << "\n\n\tpress any key to CONTINUE...";

	//std::string answer;
	//std::cin.get();// >> answer;

	socketManager.Deinitialize();
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