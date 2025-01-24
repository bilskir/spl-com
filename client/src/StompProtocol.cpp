#include "../include/StompProtocol.h"
#include <sstream>

StompProtocol::StompProtocol() : _isRunning(true), _ch(), _isConnected(false), _isLoggedIn(false), receiptId(139) {};

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
    cout<< response << endl;
    size_t index = response.find('\n');
    string command = response.substr(0, index);
    if (command == "CONNECTED")
    {
      
        this->_isLoggedIn = true;

    }

    else if (command == "ERROR")
    {
        this->_isConnected = false;
    }

    else if (command == "RECEIPT")
    {
       
       

        size_t headerIndex = response.substr(index + 1).find('\n');
        string receiptHeader("");


        size_t idStart = response.find("receipt-id:") + 11;
        if (idStart != std::string::npos){
            int idEnd = response.find('\n', idStart); 
            receiptHeader = response.substr(idStart, idEnd - idStart);
        }

        // if the recipt is a recipt recieved from a DISCONNECT frame
        if(std::stoi(receiptHeader) == logoutReceiptId){
            _isLoggedIn = false;
        }
    }
}

void StompProtocol::getResponses()
{
    while(this->_isConnected)
    {   
        cout << "waiting for next message" << endl;
        string response;
        cout << response << endl;
        if (!_ch.getFrameAscii(response, '\0'))
        {
            cout << "STAMP 5" << endl;
            cout << "Disconnected" << endl;
            _isConnected = false;
            //_isRunning = false;
            break;
        }
 
        proccess_response(response);
        cout << "STAMP 6" << endl;
    }

    this->_ch.close();
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
            cout << "STAMP 1" << endl;
            if(this-> _isLoggedIn){
                    cout << "Already logged in" << endl;
                    return "";
            }

            string address, username, password;
            ss >> address >> username >> password;


            cout << "STAMP 2" << endl;
            if(!this->_isConnected){
               
                size_t colon_index = address.find(':');
                string port = address.substr(colon_index + 1, address.length());
                address = address.substr(0, colon_index);
                this->_ch.setHost(address);
                this->_ch.setPort(stoi(port));

                if (!this->_ch.connect())
                {
                    cout << "Could not connect to server" << endl;
                    return "";
                }

                this->_isConnected = true;
                _responseThread = thread(&StompProtocol::getResponses, this);
            }
            cout << "STAMP 3" << endl;

            frame += "CONNECT\n";
            frame += "accept-version:1.2\n";
            frame += "host:stomp.cs.bgu.ac.il\n";
            frame += "login:" + username + "\n";
            frame += "passcode:" + password + "\n";
            frame += "\n";

            cout << "STAMP 4" << endl;
            return frame; 
        }

        else if (this->_isLoggedIn)
        {
            if (mainCommand == "logout")
            {
                frame += "DISCONNECT\n";
                frame += "receipt:" + std::to_string(receiptId)+ '\n';

                logoutReceiptId = receiptId;
                receiptId++;
                cout << frame << endl; // Debugging

                return frame; 
                
            }

            else if (mainCommand == "join")
            {
            }

            else if (mainCommand == "exit")
            {
            }

            else if (mainCommand == "report")
            {
            }

            else if (mainCommand == "summary")
            {
            }
        }
        else
        {
            cout << "Please login first" << endl;
        }
    }
    catch (exception &e)
    {
        cerr << e.what() << endl;
    }

    return frame;
}
