#include "stdafx.h"
#include "TcpClient.h"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

#include <chrono>

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
        err.clear();
        return true;
    }

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    std::string portStr = std::to_string(port);

    int rc = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &result);
    if (rc != 0) {
        err = "getaddrinfo failed";
        return false;
    }

    m_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_socket == INVALID_SOCKET) {
        freeaddrinfo(result);
        err = "socket failed";
        return false;
    }

    rc = ::connect(m_socket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);

    if (rc == SOCKET_ERROR) {
        closeSocket();
        err = "connect failed";
        return false;
    }

    m_connected = true;
    m_running = true;
    //m_thread = std::thread(&TcpClient::recvLoop, this);
    err.clear();
    return true;
#else
    err = "platform not implemented";
    return false;
#endif
}

bool TcpClient::disconnect(std::string& err) {
    err.clear();
    m_running = false;
    m_connected = false;

#ifdef _WIN32
    if (m_socket != INVALID_SOCKET)
        shutdown(m_socket, SD_BOTH);
#endif

    if (m_thread.joinable())
        m_thread.join();

    closeSocket();
    m_cv.notify_all();
    return true;
}

bool TcpClient::sendBytes(const std::string& data, std::string& err) {
#ifdef _WIN32
    if (!m_connected) {
        err = "socket is not connected";
        return false;
    }

    int sentTotal = 0;
    int total = (int)data.size();

    while (sentTotal < total) {
        int n = ::send(m_socket, data.data() + sentTotal, total - sentTotal, 0);
        if (n <= 0) {
            m_connected = false;
            err = "send failed";
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

bool TcpClient::isConnected() const {
    return m_connected;
}

bool TcpClient::hasMessage() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return !m_queue.empty();
}

size_t TcpClient::queueSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}

std::optional<std::string> TcpClient::popMessage() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty())
        return std::nullopt;

    std::string msg = std::move(m_queue.front());
    m_queue.pop_front();
    return msg;
}

std::optional<std::string> TcpClient::receive(int timeoutMs) {
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_queue.empty() && timeoutMs > 0) {
        m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this]() {
            return !m_queue.empty() || !m_running;
        });
    }

    if (m_queue.empty())
        return std::nullopt;

    std::string msg = std::move(m_queue.front());
    m_queue.pop_front();
    return msg;
}

void TcpClient::recvLoop() {
#ifdef _WIN32
    char buffer[4096];

    while (m_running) {
        int n = recv(m_socket, buffer, sizeof(buffer), 0);
        if (n > 0) {
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.emplace_back(buffer, buffer + n);
            }
            m_cv.notify_one();
        } else {
            m_connected = false;
            m_running = false;
            break;
        }
    }

    m_cv.notify_all();
#endif
}

void TcpClient::closeSocket() {
#ifdef _WIN32
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
#endif
}