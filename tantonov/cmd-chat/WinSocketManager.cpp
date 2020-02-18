#include "WinSocketManager.h"

#include <iostream>
#include <sstream>
#include <cassert>

#include <algorithm>
#include <iomanip>
#include <conio.h>

int WinSocketManager::Initialize()
{
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &m_WsaData);
	if (iResult != 0)
	{
		std::cout << "WSAStartup failed:" << iResult << std::endl;
		std::cin.get();
		return 1;
	}

	m_PortRange.resize(30);
	int tempPort = m_DefaultPort;
	std::generate(m_PortRange.begin(), m_PortRange.end(), [&]() { return tempPort++; });

	InitializeAddresses();

	return 0;
}

int WinSocketManager::CreateUDPSocket()
{
	m_ListenerSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (m_ListenerSocket == INVALID_SOCKET)
	{
		std::cout << "Error at socket" << WSAGetLastError() << std::endl;
		WSACleanup();
		std::cin.get();
		return 1;
	}

	m_TalkerSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (m_TalkerSocket == INVALID_SOCKET)
	{
		std::cout << "Error at socket" << WSAGetLastError() << std::endl;
		WSACleanup();
		std::cin.get();
		return 1;
	}

	AllowBroadcast();

	return 0;
}

int WinSocketManager::AllowBroadcast()
{
	const char broadcastOptionValue = '1';

	if (setsockopt(
		m_ListenerSocket,
		SOL_SOCKET,
		SO_BROADCAST,
		&broadcastOptionValue,
		sizeof(broadcastOptionValue))
		< 0)
	{
		std::cout << "Error in setting Broadcast option";
		closesocket(m_ListenerSocket);
		std::cin.get();
		return 1;
	}

	if (setsockopt(
		m_TalkerSocket,
		SOL_SOCKET,
		SO_BROADCAST,
		&broadcastOptionValue,
		sizeof(broadcastOptionValue))
		< 0)
	{
		std::cout << "Error in setting Broadcast option";
		closesocket(m_TalkerSocket);
		std::cin.get();
		return 1;
	}
}

int WinSocketManager::LaunchReceiverThread()
{
	int iResult = 0;
	int chosenPortIndex = 0;
	do 
	{
		iResult = bind(
			m_ListenerSocket,
			(struct sockaddr*) & m_ReceivAddress,
			sizeof(struct sockaddr));

		if (iResult != 0)
		{
			std::cout << "Bind error" << std::endl;
			if (chosenPortIndex > m_PortRange.size())
			{
				int newPortValue = *(m_PortRange.end())++;
				m_PortRange.push_back(newPortValue);
			}
			m_ReceivAddress.sin_port = htons(m_PortRange[++chosenPortIndex]);
			std::cout << "Trying to bind with port: " << m_PortRange[chosenPortIndex] << std::endl;
		}
	} while (iResult != 0);

	std::thread receiverThread([this]()
		{
			int numberOfBytesReceived = 0;
			struct sockaddr_in senderAddress;
			socklen_t addr_len = sizeof(struct sockaddr);
			do
			{
				if (numberOfBytesReceived = recvfrom(
					m_ListenerSocket,
					m_ReceivedDataBuffer,
					m_ReceivedDataBufferLength - 1,
					0,
					(struct sockaddr*) & senderAddress,
					&addr_len) == -1)
				{
					std::cout << "Receive failed: " << WSAGetLastError() << std::endl;
					closesocket(m_ListenerSocket);
					WSACleanup();
					std::cin.get();
					return 1;
				}
				//std::cout << "Just received a massage: " << m_ReceivedDataBuffer << std::endl;
				// add sender to list and print buff to console
				if (m_PeersAddresses.empty())
				{
					USHORT peersPort = (USHORT)std::strtoul(m_ReceivedDataBuffer, nullptr, 0);
					senderAddress.sin_port = peersPort;
					std::lock_guard<std::mutex> guard(m_Mutex);
					m_PeersAddresses.push_back(senderAddress);
				}
				else
				{
					for (struct sockaddr_in peerAddressInfo : m_PeersAddresses)
					{
						if ((peerAddressInfo.sin_addr.s_addr == senderAddress.sin_addr.s_addr)
							&&
							(peerAddressInfo.sin_port == senderAddress.sin_port))
						{
							continue;
						}
						// if unknown peer found
						USHORT peersPort = (USHORT)std::strtoul(m_ReceivedDataBuffer, nullptr, 0);
						senderAddress.sin_port = peersPort;
						std::lock_guard<std::mutex> guard(m_Mutex);
						m_PeersAddresses.push_back(senderAddress);
					}
				}
				//#TODO 1) make it thread safe 2) Add hostname or nickname of the sender
				std::cout << '\r' << m_ReceivedDataBuffer << std::setw(50) << ' ' << std::endl;
			} while (true); // #TODO create a var to control this loop
		});

	receiverThread.detach();

	return 0;
}

int WinSocketManager::SendMessageTo(sockaddr_in const& recepeintAddress, std::string const& message)
{
	int iResult = sendto(
		m_TalkerSocket,
		message.c_str(),
		message.length() + 1,
		0,
		(sockaddr*)&recepeintAddress,
		sizeof(recepeintAddress));

	if (iResult == SOCKET_ERROR)
	{
		std::cout << "Send failed: " << WSAGetLastError() << std::endl;
		closesocket(m_TalkerSocket);
		WSACleanup();
		std::cin.get();
		return 1;
	}
	return 0;
}


int WinSocketManager::SendMessageToPeers(std::string const& message)
{
	//assert(m_SendMode == SendMode::Direct && "Send mode should be set to Direct");
	std::lock_guard<std::mutex> guard(m_Mutex);
	for (sockaddr_in peerAddress : m_PeersAddresses)
	{
		int directSendResult = SendMessageTo(peerAddress, message);
		assert(directSendResult == 0 && "Direct send failed");
		//for (const int port : m_PortRange)
		//{
		//	peerAddress.sin_port = htons(port);

		//}
	}
	return 0;
}


int WinSocketManager::BroadcastHelloMessage()
{
	for (int port : m_PortRange)
	{
		m_BroadcastAddress.sin_port = htons(port);
		int broadcastResult = SendMessageTo(
			m_BroadcastAddress, 
			std::to_string(m_ReceivAddress.sin_port));
		assert(broadcastResult == 0 && "Broadcast failed");
	}

	return 0;
}

int WinSocketManager::InitializeAddresses()
{
	// Init broadcast address
	m_BroadcastAddress.sin_family = AF_INET;
	m_BroadcastAddress.sin_port = htons(m_PortRange[0]);
	m_BroadcastAddress.sin_addr.s_addr = INADDR_BROADCAST;
	memset(&(m_BroadcastAddress.sin_zero), '\0', 8); //#TODO figure out what it does

	// Init receive address
	m_ReceivAddress.sin_family = AF_INET;
	m_ReceivAddress.sin_port = htons(m_PortRange[0]);
	m_ReceivAddress.sin_addr.s_addr = INADDR_ANY; // or inaddr_any, need to think
	//memset(&(m_ReceivAddress.sin_zero), '\0', 8);

	return 0;
}

int WinSocketManager::Deinitialize()
{
	closesocket(m_ListenerSocket);
	closesocket(m_TalkerSocket);
	WSACleanup();
	return 0;
}

int WinSocketManager::ProcessUserInput()
{
	char lastInput = _getch();
	m_UserInput.push_back(lastInput);
	if (lastInput == '\r')
	{
		//lastInput = ' ';
		m_UserSentMessage = true;
		m_UserInput.push_back('\n');
		SendMessageToPeers(m_UserInput);
		//BroadcastHelloMessage(m_UserInput);
		//std::cout << "Tried to send" << m_UserInput << std::endl;
	}

	std::cout << '\r' << m_UserInput << std::setw(50) << ' ';
	if (m_UserSentMessage)
	{
		m_UserSentMessage = false;
		m_UserInput.erase();
	}

	return 0;
}