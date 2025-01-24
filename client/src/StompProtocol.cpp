#include "../include/StompProtocol.h"
#include <sstream>

StompProtocol::StompProtocol() : _isRunning(true), _ch(), _isConnected(false), _isLoggedIn(false) {};

bool StompProtocol::shouldTerminate() const { return !_isRunning; };

void StompProtocol::proccess(const string &command)
{
    string frame = parseCommand(command);

    if (!frame.empty())
    {
        _ch.sendFrameAscii(frame, '\0');
    }
}

void StompProtocol::proccess_response(const string &response)
{
    if (response)
}

void StompProtocol::getResponses()
{
    while (_isConnected)
    {
        string response;
        if (!_ch.getFrameAscii(response, '\0'))
        {
            cout << "Disconnected" << endl;
            _isConnected = false;
            break;
        }

        proccess_response(response);
    }
}

string StompProtocol::parseCommand(const string &command)
{
    stringstream ss(command);
    string mainCommand, frame("");

    try
    {
        ss >> mainCommand;

        if (mainCommand == "login")
        {

            if (this->_isConnected)
            {
                cout << "Already connected" << endl;
                return "";
            }

            string address, username, password;
            ss >> address >> username >> password;

            size_t colon_index = address.find(':');

            string port = address.substr(colon_index + 1, address.length());
            address = address.substr(0, colon_index);

            cout << "Host : " << address << " Port : " << port << endl;

            this->_ch.setHost(address);
            this->_ch.setPort(stoi(port));

            if (!this->_ch.connect())
            {
                cout << "Could not connect to server" << endl;
                return "";
            }

            this->_isConnected = true;

            _responseThread = thread(&StompProtocol::getResponses, this);

            frame += "CONNECT\n";
            frame += "accept-version:1.2\n";
            frame += "host:stomp.cs.bgu.ac.il\n";
            frame += "login:" + username + "\n";
            frame += "passcode:" + password + "\n";
            frame += "\n";

            cout << frame << endl; // Debugging
            return frame;
        }

        else if (mainCommand == "logout")
        {
            // handle join
        }

        else if (mainCommand == "join"){

        }

        else if (mainCommand == "exit"){

            
        }

        else if (mainCommand == "report"){

        }

        else if (mainCommand == "summary"){

        }

        else{

        }
    }

    catch (exception &e)
    {
        cerr << e.what() << endl;
    }

    return frame;
}