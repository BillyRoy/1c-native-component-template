#include "stdafx.h"
#include "TcpClient.h"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

TcpClient::TcpClient() {
#ifdef _WIN32
    WSADATA wsa{};
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

TcpClient::~TcpClient() {
    std::string err;
    disconnect(err);
#ifdef _WIN32
    WSACleanup();
#endif
}

bool TcpClient::connectTo(const std::string& host, int port, std::string& err) {
#ifdef _WIN32
    if (m_connected) {
        disconnect(err);
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    const std::string portStr = std::to_string(port);

    const int rc = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
    if (rc != 0 || result == nullptr) {
        err = "getaddrinfo failed";
        return false;
    }

    m_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_socket == INVALID_SOCKET) {
        freeaddrinfo(result);
        err = "socket failed";
        return false;
    }

    if (::connect(m_socket, result->ai_addr, static_cast<int>(result->ai_addrlen)) == SOCKET_ERROR) {
        freeaddrinfo(result);
        closeSocket();
        err = "connect failed: " + std::to_string(WSAGetLastError());
        return false;
    }

    freeaddrinfo(result);
    m_connected = true;
    err.clear();
    return true;
#else
    err = "platform not implemented";
    return false;
#endif
}

bool TcpClient::disconnect(std::string& err) {
    err.clear();

#ifdef _WIN32
    if (m_socket != INVALID_SOCKET) {
        shutdown(m_socket, SD_BOTH);
    }
#endif

    closeSocket();
    m_connected = false;
    return true;
}

bool TcpClient::sendBytes(const std::string& data, std::string& err) {
#ifdef _WIN32
    if (!m_connected) {
        err = "socket is not connected";
        return false;
    }

    int sentTotal = 0;
    const int total = static_cast<int>(data.size());

    while (sentTotal < total) {
        const int n = ::send(m_socket, data.data() + sentTotal, total - sentTotal, 0);
        if (n == SOCKET_ERROR) {
            const int wsaErr = WSAGetLastError();
            m_connected = false;
            closeSocket();
            err = "send failed: " + std::to_string(wsaErr);
            return false;
        }

        if (n == 0) {
            m_connected = false;
            closeSocket();
            err = "send returned 0";
            return false;
        }

        sentTotal += n;
    }

    err.clear();
    return true;
#else
    err = "platform not implemented";
    return false;
#endif
}

std::optional<std::string> TcpClient::receive(int timeoutMs, std::string& err) {
#ifdef _WIN32
    if (!m_connected) {
        err = "socket is not connected";
        return std::nullopt;
    }

    if (timeoutMs < 0) {
        timeoutMs = 0;
    }

    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(m_socket, &readSet);

    timeval tv{};
    timeval* tvPtr = nullptr;

    if (timeoutMs > 0) {
        tv.tv_sec = timeoutMs / 1000;
        tv.tv_usec = (timeoutMs % 1000) * 1000;
        tvPtr = &tv;
    }

    const int sel = select(0, &readSet, nullptr, nullptr, tvPtr);

    if (sel == 0) {
        // Таймаут: данных нет, соединение остается открытым
        err.clear();
        return std::nullopt;
    }

    if (sel == SOCKET_ERROR) {
        const int wsaErr = WSAGetLastError();
        m_connected = false;
        closeSocket();
        err = "select failed: " + std::to_string(wsaErr);
        return std::nullopt;
    }

    char buffer[4096];
    const int n = recv(m_socket, buffer, static_cast<int>(sizeof(buffer)), 0);

    if (n > 0) {
        err.clear();
        return std::string(buffer, n);
    }

    if (n == 0) {
        // Сервер закрыл соединение
        m_connected = false;
        closeSocket();
        err = "connection closed by peer";
        return std::nullopt;
    }

    const int wsaErr = WSAGetLastError();

    if (wsaErr == WSAEWOULDBLOCK) {
        // Для неблокирующего сокета это не критическая ошибка
        err.clear();
        return std::nullopt;
    }

    m_connected = false;
    closeSocket();
    err = "recv failed: " + std::to_string(wsaErr);
    return std::nullopt;
#else
    err = "platform not implemented";
    return std::nullopt;
#endif
}

bool TcpClient::isConnected() const {
    return m_connected;
}

void TcpClient::closeSocket() {
#ifdef _WIN32
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
#endif
}