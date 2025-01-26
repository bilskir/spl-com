#include <stdlib.h>
#include <bits/stdc++.h>
#include "../include/ConnectionHandler.h"
#include <thread>
#include <mutex>
#include <vector>
#include "../include/StompProtocol.h"

int main(int argc, char *argv[]) {
    StompProtocol protocol;
    std::string command;
    while(!protocol.shouldTerminate()){
        std::getline(std::cin, command);
        if(!command.empty()){
            protocol.proccess(command);
        }
    }

    return 0;
}