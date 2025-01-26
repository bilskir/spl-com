#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/StompProtocol.h"
#include <thread>
#include <mutex>
#include <string>
#include <condition_variable>


class StompClient {
private:
    ConnectionHandler *connectionHandler = nullptr;// Handles server communication
    StompProtocol stompProtocol;         // Handles STOMP protocol logic
    std::thread listenerThread;          // Thread for server responses
    std::mutex mtx;                      // Mutex for thread safety
    std::condition_variable cv;
    bool isRunning;                      // Client running state
    bool isConnected;                    // Connection state
    int connection_id;

    void handle_login(const std::string &host, const std::string &username, const std::string &password); 
    void handle_logout();
    void listenToServer();               // Server listener logic
    void stop();                         // Graceful shutdown logic

public:
    StompClient();
    StompClient(const StompClient&) = default;
    ~StompClient();
    StompClient& operator=(const StompClient&) = default;

    void run();                          // Starts the client
    bool connect(const std::string &host, short port);                      // Connects to the server
};