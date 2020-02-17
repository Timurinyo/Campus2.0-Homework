#include "WinSocketManager.h"

#include <iostream>
#include <sstream>
#include <cassert>

#include <algorithm>
#include <iomanip>
#include <conio.h>

int WinSocketManager::Initialize()
{
	m_WsaData;

	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &m_WsaData);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed:" << iResult << std::endl;
		std::cin.get();
		return 1;
	}

	InitializeAddresses();

	return 0;
}

int WinSocketManager::CreateUDPSocket()
{
	m_ChatSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (m_ChatSocket == INVALID_SOCKET)
	{
		std::cout << "Error at socket" << WSAGetLastError() << std::endl;
		WSACleanup();
		std::cin.get();
		return 1;
	}

	return 0;
}

int WinSocketManager::LaunchReceiverThread()
{
	if (bind(
		m_ChatSocket,
		(struct sockaddr*)&m_ReceivAddress,
		sizeof(struct sockaddr))
		== -1)
	{
		std::cout << "Error at binding" << WSAGetLastError() << std::endl;
		closesocket(m_ChatSocket);
		WSACleanup();
		std::cin.get();
		return 1;
	}

	std::thread receiverThread([this]()
		{
			int numberOfBytesReceived = 0;
			struct sockaddr_in senderAddress;
			socklen_t addr_len = sizeof(struct sockaddr);
			do
			{
				if (numberOfBytesReceived = recvfrom(
					m_ChatSocket,
					m_ReceivedDataBuffer,
					m_ReceivedDataBufferLength - 1,
					0,
					(struct sockaddr*) & senderAddress,
					&addr_len) == -1)
				{
					std::cout << "Receive failed: " << WSAGetLastError() << std::endl;
					closesocket(m_ChatSocket);
					WSACleanup();
					std::cin.get();
					return 1;
				}
				std::cout << "Just received a massage: " << m_ReceivedDataBuffer << std::endl;
				// add sender to list and print buff to console
				for (struct sockaddr_in peerAddressInfo : m_PeersAddresses)
				{
					if ((peerAddressInfo.sin_addr.s_addr == senderAddress.sin_addr.s_addr)
						&&
						(peerAddressInfo.sin_port == senderAddress.sin_port))
					{
						continue;
					}
					// if unknown peer found
					//std::lock_guard<std::mutex> guard(m_Mutex);

					m_PeersAddresses.push_back(senderAddress);
				}
				//#TODO 1) make it thread safe 2) Add hostname or nickname of the sender
				std::cout << '\r' << m_ReceivedDataBuffer << std::setw(50) << ' ' << std::endl;
			} while (numberOfBytesReceived > 0);
		});

	receiverThread.detach();

	return 0;
}

int WinSocketManager::SendMessageTo(sockaddr_in const& recepeintAddress, std::string const& message)
{
	int iResult = sendto(
		m_ChatSocket,
		message.c_str(),
		message.length() + 1,
		0,
		(sockaddr*)&recepeintAddress,
		sizeof(recepeintAddress));

	if (iResult == SOCKET_ERROR)
	{
		std::cout << "Send failed: " << WSAGetLastError() << std::endl;
		closesocket(m_ChatSocket);
		WSACleanup();
		std::cin.get();
		return 1;
	}
	return 0;
}


int WinSocketManager::SendMessageToPeers(std::string const& message)
{
	//assert(m_SendMode == SendMode::Direct && "Send mode should be set to Direct");

	for (const sockaddr_in peerAddress : m_PeersAddresses)
	{
		int directSendResult = SendMessageTo(peerAddress, message);
		assert(directSendResult == 0 && "Direct send failed");
	}
	return 0;
}


int WinSocketManager::BroadcastHelloMessage(std::string const& messageToBroadcast)
{
	SetSocketMode(SendMode::Broadcast);
	assert(m_SendMode == SendMode::Broadcast && "Send mode should be set to Broadcast");

	int broadcastResult =  SendMessageTo(m_BroadcastAddress, messageToBroadcast);
	assert(broadcastResult == 0 && "Broadcast failed");
	return 0;
}

int WinSocketManager::InitializeAddresses()
{
	// Init broadcast address
	m_BroadcastAddress.sin_family = AF_INET;
	m_BroadcastAddress.sin_port = 0;
	m_BroadcastAddress.sin_addr.s_addr = INADDR_BROADCAST;
	memset(&(m_BroadcastAddress.sin_zero), '\0', 8); //#TODO figure out what it does

	// Init receive address
	m_ReceivAddress.sin_family = AF_INET;
	m_ReceivAddress.sin_port = 0;//htons(12512);
	m_ReceivAddress.sin_addr.s_addr = INADDR_ANY; // or inaddr_any, need to think
	//memset(&(m_ReceivAddress.sin_zero), '\0', 8);

	return 0;
}

int WinSocketManager::SetSocketMode(SendMode sendMode)
{
	m_SendMode = sendMode;

	switch (m_SendMode)
	{
	case SendMode::Broadcast:
	{
		const char broadcastOptionValue = '1';

		//if (setsockopt(
		//	m_ChatSocket,
		//	SOL_SOCKET,
		//	IP_RECEIVE_BROADCAST,
		//	&broadcastOptionValue,
		//	sizeof(broadcastOptionValue))
		//	< 0)
		//{
		//	std::cout << "Error in setting receive broadcast option";
		//	closesocket(m_ChatSocket);
		//	std::cin.get();
		//	return 1;
		//}

		if (setsockopt(
			m_ChatSocket, 
			SOL_SOCKET, 
			SO_BROADCAST, 
			&broadcastOptionValue, 
			sizeof(broadcastOptionValue)) 
			< 0)
		{
			std::cout << "Error in setting Broadcast option";
			closesocket(m_ChatSocket);
			std::cin.get();
			return 1;
		}


	}
	break;
	case SendMode::Direct:
	{
		//Currently nothing special is needed
	}
	break;
	default:
		assert(false && "Invalid sendMode specified");
		break;
	}
	return 0;
}

int WinSocketManager::Deinitialize()
{
	closesocket(m_ChatSocket);
	WSACleanup();
	return 0;
}

int WinSocketManager::ProcessUserInput()
{
	char lastInput = _getch();
	m_UserInput.push_back(lastInput);
	if (lastInput == '\r')
	{
		m_UserInput.push_back('\n');
		m_UserSentMessage = true;
		SendMessageToPeers(m_UserInput);
		BroadcastHelloMessage(m_UserInput);
		std::cout << "Tried to send" << m_UserInput << std::endl;
	}

	std::cout << '\r' << m_UserInput << std::setw(50) << ' ';
	if (m_UserSentMessage)
	{
		m_UserSentMessage = false;
		m_UserInput.erase();
	}

	return 0;
}