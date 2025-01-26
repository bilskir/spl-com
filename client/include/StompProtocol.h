#ifndef STOMP_PROTOCOL_H
#define STOMP_PROTOCOL_H

#include "ConnectionHandler.h"
#include "Event.h"
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class StompProtocol {
public:
    // Constructor
    StompProtocol();

    // Public methods
    bool shouldTerminate() const;                          // Check if the protocol should terminate
    void proccess(const std::string &command);             // Process user input commands
    void proccess_response(const std::string &response);   // Handle server responses
    void getResponses();                                   // Thread function to read server responses
    void saveSummary(const std::string &channel, const std::string &user, const std::string &filename);

private:
    // Private methods
    std::string parseCommand(const std::string &command); // Parse commands and generate STOMP frames
    std::string epochToDate(long long epoch_time);        // Convert epoch time to human-readable string 

    // Private members
    bool _isRunning;                                      // Flag to signal the thread to run or stop
    ConnectionHandler _ch;                                // Handles the connection to the server
    bool _isConnected;                                    // Indicates if the client is connected
    bool _isLoggedIn;                                     // Indicates if the client is logged in
    int receiptId;                                        // Counter for receipt IDs
    int logoutId;                                         // ID to keep logout receipt id
    std::thread _responseThread;                          // Thread to handle server responses
    std::unordered_map<std::string, int> _subscriptions;  // Map of channel name to subscription ID
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>> _events; // Map of events by channel and user
};

#endif // STOMP_PROTOCOL_H
