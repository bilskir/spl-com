#include "../include/StompProtocol.h"
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <mutex>
#include <thread>

StompProtocol::StompProtocol()
    : _isRunning(true),
      _ch(),
      _isConnected(false),
      _isLoggedIn(false),
      receiptId(0),
      _responseThread(),
      _subscriptions(),
      _events() {
    std::cout << "[DEBUG] StompProtocol instance initialized." << std::endl;
}

bool StompProtocol::shouldTerminate() const {
    return !_isRunning;
}

void StompProtocol::proccess(const std::string &command) {
    std::cout << "[DEBUG] Received Command: " << command << std::endl;
    std::string frame = parseCommand(command);

    if (!frame.empty()) {
        std::cout << "[DEBUG] Sending Frame:\n" << frame << std::endl;
        if (!_ch.sendFrameAscii(frame, '\0')) {
            std::cerr << "[ERROR] Failed to send frame in process()" << std::endl;
        }
        else{
            std::cout << "[DEBUG] Frame sent successfully." << std::endl;
        }
    }
    else{
        std::cout << "[DEBUG] No frame generated for command: " << command << std::endl;
    }
}

void StompProtocol::proccess_response(const std::string &response) {
    size_t index = response.find('\n');
    std::string command = response.substr(0, index);
    std::cout << "[DEBUG] Full Response received:\n" << response << std::endl;

    if (command == "CONNECTED") {
        _isConnected = true;
        _isLoggedIn = true;
        std::cout << "[DEBUG] State Change: _isConnected=true, _isLoggedIn=true" << std::endl;
        std::cout << "Login successful" << std::endl;
    } else if (command == "RECEIPT") {
        size_t idStart = response.find("receipt-id:") + 11;
        if (idStart != std::string::npos) {
            size_t idEnd = response.find('\n', idStart);
            std::string receiptHeader = response.substr(idStart, idEnd - idStart);
            std::cout << "Received receipt: " << receiptHeader << " receiptId: " << receiptId<<std::endl;

            if (logoutId == stoi(receiptHeader)){
                std::cout << "Closing Connection.\n" << std::endl;
                _isConnected = false;
                _isLoggedIn = false;
                _ch.close();
                _responseThread.detach();
                return;
            }
        }

    } else if (command == "MESSAGE") {
        std::cout << "Message received: " << response.substr(index + 1) << std::endl;
    } else if (command == "ERROR") {
        std::cerr << "Error received: " << response.substr(index + 1) << std::endl;
    }
    else{
        std::cerr << "[ERROR] Unrecognized command in response: " << command << std::endl;
    }
}

void StompProtocol::getResponses() {
    std::cout << "[DEBUG] Response thread started." << std::endl;
    while (_isConnected) {
        std::string response;
        std::cout << "[DEBUG] Waiting to receive a frame..." << std::endl;
        if (!_ch.getFrameAscii(response, '\0')) {
            std::cerr << "[ERROR] Disconnected from server or failed to read frame." << std::endl;
            _isConnected = false;
            break;
        }
        if (!response.empty()) {
            std::cout << "[DEBUG] Full Response received:\n" << response << std::endl;
            proccess_response(response);
        } else {
            std::cout << "[DEBUG] Received empty response. Continuing..." << std::endl;
        }
    }
    std::cout << "[DEBUG] Exiting response thread..." << std::endl;
}



std::string StompProtocol::parseCommand(const std::string &command) {
    std::stringstream ss(command);
    std::string mainCommand, frame("");

    try {
        ss >> mainCommand;

        if (mainCommand == "login") {
            if (_isConnected) {
                std::cout << "The client is already logged in, log out before trying again" << std::endl;
                return "";
            }

            std::string address, username, password;
            ss >> address >> username >> password;

            size_t colon_index = address.find(':');
            if (colon_index == std::string::npos) {
                std::cerr << "[ERROR] Invalid address format. Expected format: host:port" << std::endl;
                return "";
            }

            std::string port = address.substr(colon_index + 1);
            address = address.substr(0, colon_index);

            _ch.setHost(address);
            _ch.setPort(std::stoi(port));

            if (!_ch.connect()) {
                std::cerr << "Could not connect to server" << std::endl;
                return "";
            }

            _isConnected = true;
            _isRunning = true;

            _responseThread = std::thread(&StompProtocol::getResponses, this);
            std::cout << "[DEBUG] Response thread started successfully after login." << std::endl;

            frame += "CONNECT\n";
            frame += "accept-version:1.2\n";
            frame += "host:stomp.cs.bgu.ac.il\n";
            frame += "login:" + username + "\n";
            frame += "passcode:" + password + "\n";
            //frame += "\0"; // Null terminator

            return frame;
        }

        if (!_isConnected) {
            std::cout << "Please login first" << std::endl;
            return "";
        }

        if (mainCommand == "logout") {
            logoutId = receiptId;
            frame += "DISCONNECT\n";
            frame += "receipt:" + std::to_string(receiptId++) + '\n';
            frame += "\0";
            return frame;
    // std::cout << "[DEBUG] Generated Logout Frame:\n" << frame << std::endl;
    // _isRunning = false;
    // std::cout << "[DEBUG] Stopping response thread..." << std::endl;
    // if (_responseThread.joinable()) {
    //     std::cout << "[DEBUG] Waiting for response thread to join..." << std::endl;
    //     _responseThread.join();
    //     std::cout << "[DEBUG] Response thread joined successfully." << std::endl;
    // } else {
    //     std::cerr << "[ERROR] Response thread was not joinable." << std::endl;
    // }
    // _ch.close();
    // _isConnected = false;
    // _isLoggedIn = false;
    // std::cout << "[DEBUG] Closing connection handler..." << std::endl;
    // std::cout << "[DEBUG] State Change: _isConnected=false, _isLoggedIn=false" << std::endl;
    }

        if (mainCommand == "join") {
            std::string channel;
            ss >> channel;

            int subscriptionId = receiptId++;
            _subscriptions[channel] = subscriptionId;

            frame += "SUBSCRIBE\n";
            frame += "destination:/" + channel + "\n";
            frame += "id:" + std::to_string(subscriptionId) + "\n";
            frame += "receipt:" + std::to_string(receiptId++) + "\n";
            //frame += "\0";
             std::cout << "[DEBUG] State Change: _isConnected=false, _isLoggedIn=false" << std::endl;
            return frame;
        }

        if (mainCommand == "exit") {
            std::string channel;
            ss >> channel;

            if (_subscriptions.find(channel) == _subscriptions.end()) {
                std::cerr << "Error: Not subscribed to channel: " << channel << std::endl;
                return "";
            }

            int subscriptionId = _subscriptions[channel];
            _subscriptions.erase(channel);

            frame += "UNSUBSCRIBE\n";
            frame += "id:" + std::to_string(subscriptionId) + "\n";
            frame += "receipt:" + std::to_string(receiptId++) + "\n";
            //frame += "\0";

            return frame;
        }

        if (mainCommand == "report") {
            std::string filename;
            ss >> filename;

            names_and_events data = parseEventsFile(filename);

            for (const Event &event : data.events) {
                frame += "SEND\n";
                frame += "destination:/" + data.channel_name + "\n";
                frame += "user:current_user\n";
                frame += "city:" + event.get_city() + "\n";
                frame += "event name:" + event.get_name() + "\n";
                frame += "date time:" + std::to_string(event.get_date_time()) + "\n";
                frame += "general information:\n";
                frame += "active:" + event.get_general_information().at("active") + "\n";
                frame += "forces_arrival_at_scene:" + event.get_general_information().at("forces_arrival_at_scene") + "\n";
                frame += "description:" + event.get_description() + "\n";
                //frame += "\0";

                _events[data.channel_name]["current_user"].push_back(event);
            }

            return frame;
        }

        if (mainCommand == "summary") {
            std::string channel, user, filename;
            ss >> channel >> user >> filename;

            saveSummary(channel, user, filename);
            return "";
        }
    } catch (const std::exception &e) {
        std::cerr << "[ERROR] Exception in parseCommand: " << e.what() << std::endl;
    }

    return frame;
}

void StompProtocol::saveSummary(const std::string &channel, const std::string &user, const std::string &filename) {
    std::cout << "[DEBUG] Saving summary for channel: " << channel 
              << ", user: " << user << ", filename: " << filename << std::endl;

    std::ofstream file(filename);
    if (!file) {
        std::cerr << "[ERROR] Error opening file: " << filename << std::endl;
        return;
    }

    if (_subscriptions.find(channel) == _subscriptions.end()) {
        std::cerr << "[DEBUG] No subscription to channel: " << channel << std::endl;
        file << "No subscription to channel: " << channel << std::endl;
        return;
    }

    if (_events.find(channel) == _events.end() || _events[channel].find(user) == _events[channel].end()) {
        std::cerr << "[DEBUG] No events found for user: " << user 
                  << " in channel: " << channel << std::endl;
        file << "No events found for user: " << user << " in channel: " << channel << std::endl;
        return;
    }

    const auto &events = _events[channel][user];

    std::cout << "[DEBUG] Preparing summary for " << events.size() << " events." << std::endl;
    auto sorted_events = events;
    std::sort(sorted_events.begin(), sorted_events.end(), [](const Event &a, const Event &b) {
        if (a.get_date_time() != b.get_date_time())
            return a.get_date_time() < b.get_date_time();
        return a.get_name() < b.get_name();
    });

    int active_count = 0;
    int forces_count = 0;

    for (const auto &event : sorted_events) {
        if (event.get_general_information().at("active") == "true")
            active_count++;
        if (event.get_general_information().at("forces_arrival_at_scene") == "true")
            forces_count++;
    }

    file << "Channel: " << channel << "\nStats:\n"
         << "Total: " << sorted_events.size() << "\n"
         << "active: " << active_count << "\n"
         << "forces arrival at scene: " << forces_count << "\n\n";

    file << "Event Reports:\n";
    for (size_t i = 0; i < sorted_events.size(); ++i) {
        const auto &event = sorted_events[i];
        std::string description = event.get_description().substr(0, 27);
        if (event.get_description().size() > 27)
            description += "...";

        file << "Report_" << i + 1 << ":\n"
             << "city: " << event.get_city() << "\n"
             << "date time: " << epochToDate(event.get_date_time()) << "\n"
             << "event name: " << event.get_name() << "\n"
             << "summary: " << description << "\n\n";
    }

    std::cout << "[DEBUG] Summary saved successfully to " << filename << std::endl;
    file.close();
}

std::string StompProtocol::epochToDate(long long epoch_time) {
    std::time_t time = static_cast<std::time_t>(epoch_time);
    std::tm *tm = std::localtime(&time);

    if (tm == nullptr) {
        std::cerr << "[ERROR] Failed to convert epoch time to date." << std::endl;
        return "";
    }

    std::stringstream ss;
    ss << std::put_time(tm, "%d/%m/%Y %H:%M");
    std::cout << "[DEBUG] Converted epoch time: " << epoch_time 
              << " to date: " << ss.str() << std::endl;
    return ss.str();
}
