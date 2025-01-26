#include "../include/ConnectionHandler.h"
#include <boost/asio.hpp>
#include <iostream>

ConnectionHandler::ConnectionHandler() : host_(), port_(-1), io_service_(), socket_(io_service_) {}
ConnectionHandler::ConnectionHandler(std::string host, short port)
    : host_(host), port_(port), io_service_(), socket_(io_service_) {}

ConnectionHandler::~ConnectionHandler() {
    close();
}

bool ConnectionHandler::connect() {
    std::cout << "Starting connect to " << host_ << ":" << port_ << std::endl;
    try {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(host_), port_);
        boost::system::error_code error;
        socket_.connect(endpoint, error);
        if (error)
            throw boost::system::system_error(error);
    } catch (std::exception &e) {
        std::cerr << "Connection failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
    size_t tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToRead > tmp) {
            tmp += socket_.read_some(boost::asio::buffer(bytes + tmp, bytesToRead - tmp), error);
        }
        if (error) {
            std::cerr << "[ERROR] getBytes failed: " << error.message() << std::endl;
            return false;
        }
    } catch (std::exception &e) {
        std::cerr << "[ERROR] Exception in getBytes: " << e.what() << std::endl;
        return false;
    }
    return true;
}


bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
    std::cout << "[DEBUG] Preparing to send " << bytesToWrite << " bytes." << std::endl;
    int tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToWrite > tmp) {
            tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
            std::cout << "[DEBUG] Sent " << tmp << "/" << bytesToWrite << " bytes." << std::endl;
        }
        if (error) {
            std::cerr << "[ERROR] sendBytes failed: " << error.message() << std::endl;
            return false;
        }
    } catch (std::exception &e) {
        std::cerr << "[ERROR] Exception in sendBytes: " << e.what() << std::endl;
        return false;
    }
    std::cout << "[DEBUG] Frame sent successfully." << std::endl;
    return true;
}


bool ConnectionHandler::getFrameAscii(std::string &frame, char delimiter) {
    char ch;
    try {
        do {
            if (!getBytes(&ch, 1)) {
                return false;
            }
            if (ch != '\0')
                frame.append(1, ch);
        } while (delimiter != ch);
    } catch (std::exception &e) {
        std::cerr << "recv failed2 (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::sendFrameAscii(const std::string &frame, char delimiter) {
    std::cout << "Attempting to send frame: " << frame << std::endl;
    bool result = sendBytes(frame.c_str(), frame.length());
    if (!result) {
        std::cerr << "Failed to send frame data" << std::endl;
        return false;
    }
    return sendBytes(&delimiter, 1);
}

void ConnectionHandler::close() {
    try {
        if (socket_.is_open()) {
            socket_.close();
        }
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}

void ConnectionHandler::setHost(const std::string &host) {
    this->host_ = host;
}

void ConnectionHandler::setPort(short port) {
    this->port_ = port;
}

bool ConnectionHandler::sendLine(std::string &line) {
    return sendFrameAscii(line, '\n');  // Use sendFrameAscii with newline as the delimiter
}

bool ConnectionHandler::getLine(std::string &line) {
    return getFrameAscii(line, '\n');  // Use getFrameAscii with newline as the delimiter
}


