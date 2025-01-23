#pragma once

#include "StompFrame.h"
// TODO: implement the STOMP protocol
class StompProtocol
{
    private:
        bool _isRunning;
    public:
        StompProtocol();
        bool shouldTerminate() const;
        void proccess(const StompFrame& message);

};
