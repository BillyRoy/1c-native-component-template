#pragma once

#include <optional>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

class TcpClient {
public:
    TcpClient();
    ~TcpClient();

    bool connectTo(const std::string& host, int port, std::string& err);
    bool disconnect(std::string& err);
    bool sendBytes(const std::string& data, std::string& err);

    std::optional<std::string> receive(int timeoutMs, std::string& err);

    bool isConnected() const;

private:
    void closeSocket();

private:
#ifdef _WIN32
    SOCKET m_socket = INVALID_SOCKET;
#else
    int m_socket = -1;
#endif
    bool m_connected = false;
};