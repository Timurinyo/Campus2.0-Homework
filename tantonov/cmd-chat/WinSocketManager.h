#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>

enum class SendMode
{
    Broadcast,
    Direct,
    Invalid
};

class WinSocketManager
{
public:
    static WinSocketManager& GetInstance()
    {
        static WinSocketManager* instance = new WinSocketManager();
        return *instance;
    }

    WinSocketManager(WinSocketManager const&) = delete;
    void operator=(WinSocketManager const&) = delete;

    int Initialize();
    int Deinitialize();

    int CreateUDPSocket();

    int LaunchReceiverThread();

    int BroadcastHelloMessage();

    int SendMessageToPeers(std::string const& message);

    int ProcessUserInput();

private:

    WinSocketManager() {};

    int InitializeAddresses();

    int SendMessageTo(sockaddr_in const& recepeintAddress, std::string const& message);

    int AllowBroadcast();


    WSADATA m_WsaData;

    SOCKET m_ListenerSocket = INVALID_SOCKET;
    SOCKET m_TalkerSocket = INVALID_SOCKET;

    struct sockaddr_in m_BroadcastAddress;
    struct sockaddr_in m_ReceivAddress;

    std::vector<struct sockaddr_in> m_PeersAddresses;

    int m_ReceivedDataBufferLength = 1024;
    char m_ReceivedDataBuffer[1024];

    std::mutex m_Mutex;

    std::string m_UserInput;
    bool m_UserSentMessage = false;

    const int m_DefaultPort = 9009;
    std::vector<int> m_PortRange;
};

