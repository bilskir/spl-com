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






/*
//From here we will see the rest of the ehco client implementation:
    while (1) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::String command = cin >> endl;
		std::String
 
        // We can use one of three options to read data from the server:
        // 1. Read a fixed number of characters
        // 2. Read a line (up to the newline character using the getline() buffered reader
        // 3. Read up to the null character
        std::string answer;
        // Get back an answer: by using the expected number of bytes (len bytes + newline delimiter)
        // We could also use: connectionHandler.getline(answer) and then get the answer without the newline char at the end
        if (!connectionHandler.getLine(answer)) {
            std::cout << "Disconnected. Exiting...\n" << std::endl;
            break;
        }
        
		len=answer.length();
		// A C string must end with a 0 char delimiter.  When we filled the answer buffer from the socket
		// we filled up to the \n char - we must make sure now that a 0 char is also present. So we truncate last character.
        answer.resize(len-1);
        std::cout << "Reply: " << answer << " " << len << " bytes " << std::endl << std::endl;
        if (answer == "bye") {
            std::cout << "Exiting...\n" << std::endl;
            break;
        }
    }*/