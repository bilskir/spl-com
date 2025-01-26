#pragma once

#include <map>
#include <string>

using namespace std;

class StompFrame
{
private:
    const string command;
    const string body;
    map<string, string> headers;

public:
    StompFrame(const string command, const string body);
    StompFrame(const StompFrame &frame);
    void addHeader(const string key, const string value);

    const string &getCommand() const;
    const string &getBody() const;
    const map<string, string> &getHeaders() const;
    string toString() const;
};