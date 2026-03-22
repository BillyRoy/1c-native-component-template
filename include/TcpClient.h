#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <optional>


class TcpClient {
public:
    TcpClient();
    ~TcpClient();

    bool connectTo(const std::string& host, int port, std::string& err);
    bool disconnect(std::string& err);
    bool sendBytes(const std::string& data, std::string& err);

    bool isConnected() const;
    bool hasMessage() const;
    size_t queueSize() const;

    std::optional<std::string> popMessage();
    std::optional<std::string> receive(int timeoutMs);

private:
    void recvLoop();
    void closeSocket();

private:
#ifdef _WIN32
    SOCKET m_socket = INVALID_SOCKET;
#endif
    std::atomic<bool> m_connected{false};
    std::atomic<bool> m_running{false};
    std::thread m_thread;

    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    std::deque<std::string> m_queue;
};