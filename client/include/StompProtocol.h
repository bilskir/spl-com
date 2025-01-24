#pragma once

#include "StompFrame.h"
#include "ConnectionHandler.h"

// TODO: implement the STOMP protocol
class StompProtocol
{
    private:
        bool _isRunning;
        thread _responseThread;
        ConnectionHandler _ch;
        bool _isConnected;
        bool _isLoggedIn;
        int receiptId;
        int logoutReceiptId;

        
    public:
        StompProtocol();
        bool shouldTerminate() const;
        void proccess(const string& command);
        string parseCommand(const string& command);
        void proccess_response(const string& response);
        void getResponses();

};
