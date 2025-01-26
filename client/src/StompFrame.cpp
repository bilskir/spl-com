#include "../include/StompFrame.h"


StompFrame::StompFrame(const string command, const string body): command(command), body(body){headers = map<string,string>();}
StompFrame::StompFrame(const StompFrame& other): command(other.command), body(other.body), headers(other.headers){}

void StompFrame::addHeader(const string key, const string value){headers.insert(key,value);}

const string& StompFrame::getCommand() const{return command;}
const string& StompFrame::getBody() const{return body;}
const map<string,string>& StompFrame::getHeaders() const{return headers;}

string StompFrame::toString() const{
    string out = command + "\n";

    map<string, string>::iterator it;
    for(it = headers.begin(); it != headers.end(); it++){
        out = out + it -> first + ":" + it -> second + "\n";
    }
    out = out + "\n";
    out = out + body + "\n";

    return out;
}