
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

void StompProtocol::printEvents(const std::unordered_map<std::string, std::unordered_map<std::string, std::vector<Event>>> &_events)
{
    // Iterate through the outer map (channels)
    for (const auto &channelEntry : _events)
    {
        const std::unordered_map<std::string, std::vector<Event>> &userMap = channelEntry.second;

        // Iterate through the inner map (users)
        for (const auto &userEntry : userMap)
        {
            const std::string &user = userEntry.first; // User name
            const std::vector<Event> &events = userEntry.second;

            std::cout << "  User: " << user << std::endl;

            // Iterate through the vector of events for this user
            for (const Event &event : events)
            {
                std::cout << "    Event Name: " << event.get_name() << std::endl;
                std::cout << "    Date Time: " << event.get_date_time() << std::endl;
                std::cout << "    Description: " << event.get_description() << std::endl;
                std::cout << "    -----------------------------" << std::endl;
            }
        }
    }
}

StompProtocol::StompProtocol()
    : _responseThread(),
    _isRunning(true),
    _ch(),
    _isConnected(false),
    _isLoggedIn(false),
    receiptId(0),
    logoutId(0),
    _subscriptions(),
    _events(),
    userName(""),
    tempUserName(""),
    currentChannel(""){}

    bool StompProtocol::shouldTerminate() const
{
    return !_isRunning;
}

void StompProtocol::proccess(const std::string &command)
{
    std::string frame = parseCommand(command);

    if (!frame.empty())
    {
        if (!_ch.sendFrameAscii(frame, '\0'))
        {
            std::cerr << "[ERROR] Failed to send frame in process()" << std::endl;
        }
    }
}

void StompProtocol::proccess_response(const std::string &response)
{
    size_t index = response.find('\n');
    size_t bodyIndex = response.find("\n\n");
    std::string command = response.substr(0, index);
    std::string body = response.substr(bodyIndex + 1);

    if (command == "CONNECTED")
    {
        _isConnected = true;
        _isLoggedIn = true;
        userName = tempUserName;
        std::cout << "Login successful" << std::endl;
    }
    else if (command == "RECEIPT")
    {
        size_t idStart = response.find("receipt-id:") + 11;
        if (idStart != std::string::npos)
        {
            size_t idEnd = response.find('\n', idStart);
            std::string receiptHeader = response.substr(idStart, idEnd - idStart);
            std::cout << "Received receipt: " << receiptHeader << " receiptId: " << receiptId << std::endl;

            if (logoutId == stoi(receiptHeader))
            {
                std::cout << "Closing Connection.\n"
                          << std::endl;
                _isConnected = false;
                _isLoggedIn = false;
                userName = "";
                _ch.close();
                _responseThread.detach();
                return;
            }
        }
    }
    else if (command == "MESSAGE")
    {
        std::vector<Event> eventsRecieved = formatRecievedEvents(body, currentChannel);
        for (Event event : eventsRecieved)
        {
            std::string channel = event.get_channel_name();
            std::string user = event.getEventOwnerUser();
            // Ensure the channel exists
            if (_events.find(channel) == _events.end())
            {
                // If channel doesn't exist, create a new unordered_map for it
                _events[channel] = std::unordered_map<std::string, std::vector<Event>>();
            }

            if (_events[channel].find(user) == _events[channel].end())
            {
                // If user doesn't exist, create a new vector for this user
                _events[channel][user] = std::vector<Event>();
            }

            _events[channel][user].push_back(event);
        }

        StompProtocol::printEvents(_events);
        std::cout << "Message received: " << response.substr(index + 1) << std::endl;
    }
    else if (command == "ERROR")
    {
        std::cout << body << std::endl;
        std::cerr << "Error received: " << response.substr(index + 1) << std::endl;
        _isConnected = false;
        _isRunning = false;
    }
}

void StompProtocol::getResponses()
{
    while (_isConnected)
    {
        std::string response;
        if (!_ch.getFrameAscii(response, '\0'))
        {
            std::cerr << "[ERROR] Disconnected from server or failed to read frame." << std::endl;
            _isConnected = false;
            break;
        }
        if (!response.empty())
        {
            std::cout << "[DEBUG] Full Response received:\n"
                      << response << std::endl;
            proccess_response(response);
        }
        else
        {
            std::cout << "[DEBUG] Received empty response. Continuing..." << std::endl;
        }
    }
    std::cout << "[DEBUG] Exiting response thread..." << std::endl;
}

std::string StompProtocol::parseCommand(const std::string &command)
{
    std::stringstream ss(command);
    std::string mainCommand, frame("");

    try
    {
        ss >> mainCommand;

        if (mainCommand == "login")
        {
            if (_isConnected)
            {
                std::cout << "The client is already logged in, log out before trying again" << std::endl;
                return "";
            }

            std::string address, username, password;

            if (!(ss >> address >> username >> password))
            {
                std::cout << "invalid login command, Error: missing variables - expected format is address:port username password" << std::endl;
                return "";
            }

            // check if extra input is added to the command
            std::string extra;
            if (ss >> extra)
            {
                std::cout << "invalid login command, Error: extra variables were added after the expected amount" << std::endl;
                return "";
            }
            // ss >> address >> username >> password;

            size_t colon_index = address.find(':');
            if (colon_index == std::string::npos)
            {
                std::cerr << "[ERROR] Invalid address format. Expected format: host:port" << std::endl;
                return "";
            }

            std::string port = address.substr(colon_index + 1);
            address = address.substr(0, colon_index);

            _ch.setHost(address);
            _ch.setPort(std::stoi(port));

            if (!_ch.connect())
            {
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
            // frame += "\0"; // Null terminator
            tempUserName = username;
            return frame;
        }

        if (!_isConnected)
        {
            std::cout << "Please login first" << std::endl;
            return "";
        }

        if (mainCommand == "logout")
        {
            std::string extra;
            if (ss >> extra)
            {
                std::cout << "invalid login command, Error: extra variables were added after the expected amount" << std::endl;
                return "";
            }

            logoutId = receiptId;
            frame += "DISCONNECT\n";
            frame += "receipt:" + std::to_string(receiptId++) + '\n';
            frame += "\0";

            return frame;
        }

        if (mainCommand == "join")
        {
            std::string channel;
            if (!(ss >> channel))
            {
                std::cout << "Invalid join command. Expected format: join channels" << std::endl;
                return "";
            }

            std::string extra;
            if (ss >> extra)
            {
                std::cout << "Too many arguments provided in the join command. Expected format: join channels" << std::endl;
                return "";
            }

            int subscriptionID;
            try
            {
                subscriptionID = _subscriptions.at(channel);
                std::cout << "User is already subscribed to this channel" << std::endl;
                return frame;
            }
            catch (const std::exception &e)
            {
                subscriptionID = receiptId++;
                _subscriptions[channel] = subscriptionID;
            }

            frame += "SUBSCRIBE\n";
            frame += "destination:" + channel + "\n";
            frame += "id:" + std::to_string(subscriptionID) + "\n";
            frame += "receipt:" + std::to_string(receiptId++) + "\n";
            // frame += "\0";
            std::cout << "[DEBUG] State Change: _isConnected=false, _isLoggedIn=false" << std::endl;
            return frame;
        }

        if (mainCommand == "exit")
        {
            std::string channel;
            if (!(ss >> channel))
            {
                std::cout << "Invalid exit command. Expected format: exit channel" << std::endl;
                return "";
            }

            std::string extra;
            if (ss >> extra)
            {
                std::cout << "Too many arguments provided in the exit command. Expected format: exit channel" << std::endl;
                return "";
            }

            if (_subscriptions.find(channel) == _subscriptions.end())
            {
                std::cerr << "Error: Not subscribed to channel: " << channel << std::endl;
                return "";
            }

            int subscriptionId = _subscriptions[channel];
            _subscriptions.erase(channel);

            frame += "UNSUBSCRIBE\n";
            frame += "id:" + std::to_string(subscriptionId) + "\n";
            frame += "receipt:" + std::to_string(receiptId++) + "\n";
            // frame += "\0";

            return frame;
        }

        if (mainCommand == "report")
        {
            std::string filename;
            if (!(ss >> filename))
            {
                std::cout << "Invalid report command. Expected format: report filename" << std::endl;
                return "";
            }
            std::string extra;
            if (ss >> extra)
            {
                std::cout << "Too many arguments provided in the report command" << std::endl;
                return "";
            }

            names_and_events data = parseEventsFile(filename);
            frame += "SEND\n";
            frame += "destination:" + data.channel_name + '\n';
            frame += "\n";
            for (const Event &event : data.events)
            {
                frame += "user:" + userName + "\n";
                frame += "city:" + event.get_city() + "\n";
                frame += "event name:" + event.get_name() + "\n";
                frame += "date time:" + std::to_string(event.get_date_time()) + "\n";
                frame += "general information:\n";
                frame += "active:" + event.get_general_information().at("active") + "\n";
                frame += "forces_arrival_at_scene:" + event.get_general_information().at("forces_arrival_at_scene") + "\n";
                frame += "description:" + event.get_description() + "\n";
            }
            currentChannel = data.channel_name;
            return frame;
        }

        if (mainCommand == "summary")
        {
            std::string channel, user, filename;
            if (!(ss >> channel >> user >> filename))
            {
                std::cout << "Invalid summary command. Expected format: summary channel user outputFileName" << std::endl;
            }
            std::string extra;
            if (ss >> extra)
            {
                std::cout << "Too many arguments in summary" << std::endl;
            }

            saveSummary(channel, user, filename);
            return "";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[ERROR] Exception in parseCommand: " << e.what() << std::endl;
    }

    return frame;
}

void StompProtocol::saveSummary(const std::string &channel, const std::string &user, const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "[ERROR] Could not open file: " << filename << std::endl;
        return;
    }

    // Check if the user has events in the specified channel
    if (_events.find(channel) == _events.end() || _events[channel].find(user) == _events[channel].end())
    {
        file << "No events found for user: " << user << " in channel: " << channel << std::endl;
        file.close();
        return;
    }

    const auto &events = _events[channel][user];
    // Sort events by date and time
    std::vector<Event> sorted_events(events);
    std::sort(sorted_events.begin(), sorted_events.end(), [](const Event &a, const Event &b)
              { return a.get_date_time() < b.get_date_time(); });

    // Calculate statistics
    int total_events = sorted_events.size();
    int active_count = 0;
    int forces_arrival_count = 0;

    
    for (Event event : sorted_events)
    {
        const auto &info = event.get_general_information();
        if (info.find("active") != info.end() && info.at("active") == "true")
        {
            active_count++;
        }
        if (info.find("forces_arrival_at_scene") != info.end() && info.at("forces_arrival_at_scene") == "true")
        {
            forces_arrival_count++;
        }
        
    }
    // Write the summary to the file
    file << "Channel " << channel << "\n";
    file << "Stats:\n";
    file << "Total: " << total_events << "\n";
    file << "active: " << active_count << "\n";
    file << "forces arrival at scene: " << forces_arrival_count << "\n\n";

    file << "Event Reports:\n\n";
    // Write event details
    for (const auto &event : sorted_events)
    {
        file << epochToDate(event.get_date_time()) << " - "
             << event.get_name() << " - "
             << event.get_city() << ":\n";
        file << event.get_description() << "\n\n";
    }
    file.close();
}

std::string StompProtocol::epochToDate(long long epoch_time)
{
    std::time_t time = static_cast<std::time_t>(epoch_time);
    std::tm *tm = std::localtime(&time);
    if (!tm)
    {
        return "Invalid date";
    }
    std::ostringstream oss;
    oss << std::put_time(tm, "%d/%m/%Y %H:%M:%S");
    return oss.str();
}

std::vector<Event> StompProtocol::formatRecievedEvents(const std::string &body, const std::string &channel)
{
    std::istringstream inputStream(body);
    std::string line;
    std::vector<Event> events; // Vector to store the parsed events

    std::string user, city, eventName, dateTime, description;
    std::map<std::string, std::string> general_information;

    // Lambda function to add the current event to the events vector
    auto addEvent = [&]()
    {
        if (!eventName.empty() && !city.empty())
        {
            // Convert dateTime to int (assuming it is a valid integer in string form)
            int dateTimeInt = std::stoi(dateTime);

            // Create an Event object using the constructor, including general information
            Event event(channel, city, eventName, dateTimeInt, description, general_information);
            event.setEventOwnerUser(user); // Set the event owner user

            events.push_back(event); // Add the created Event to the vector
        }
    };

    while (std::getline(inputStream, line))
    {
        // Trim leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        if (line.rfind("user:", 0) == 0)
        {
            addEvent();            // Add the previous event before starting a new one
            user = line.substr(5); // Get the user (now used for event owner user)
        }
        else if (line.rfind("city:", 0) == 0)
        {
            city = line.substr(5); // Get the city
        }
        else if (line.rfind("event name:", 0) == 0)
        {
            eventName = line.substr(11); // Get the event name
        }
        else if (line.rfind("date time:", 0) == 0)
        {
            dateTime = line.substr(10); // Get the date time
        }
        else if (line.rfind("description:", 0) == 0)
        {
            description = line.substr(12); // Get the description
        }
        else if (line.rfind("general information:", 0) == 0)
        {
            // We can add additional general information into the map if necessary
            std::string key_value = line.substr(20);
            size_t pos = key_value.find(":");
            if (pos != std::string::npos)
            {
                std::string key = key_value.substr(0, pos);
                std::string value = key_value.substr(pos + 1);

                value.erase(0, value.find_first_not_of(" \t"));

                // Insert "active" and "forces_arrival_at_scene" without spaces after the colon
                if (key == "active" || key == "forces_arrival_at_scene")
                {
                    general_information[key] = value; // Insert it as-is, without trimming spaces
                }
            }
        }
    }

    // Add the last event after the loop ends
    addEvent();

    return events; // Return the vector of Event objects
}
