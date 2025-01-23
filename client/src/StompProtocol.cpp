#include "../include/StompProtocol.h"

StompProtocol::StompProtocol():_isRunning(true){}


bool StompProtocol::shouldTerminate() const {return !_isRunning;}

void StompProtocol::proccess(const StompFrame& frame){
    
}