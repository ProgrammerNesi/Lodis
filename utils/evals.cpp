#include <string>
#include "utils.h"
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <ctime>
#include "../config/config.h"

using namespace std;

// SET k y EX 5 (in seconds)
string evalSET(vector<string> &args)
{
    long long expiresAt = -1;
    if (args.size() >= 4)
    {
        long long seconds = stoll(args[3]);
        expiresAt = static_cast<long long>(time(nullptr)) + seconds;
    }
    else if (args.size() != 2)
    {
        return "-ERR syntax error\r\n";
    }
    string key = args[0];
    string value = args[1];
    putKey(key, value, expiresAt);
    return "+OK\r\n";
}

string evalGET(vector<string> &args)
{

    string value = getKey(args[0]);
    if(value=="$-1\r\n"){
        return value;
    }
    return encode(value, true);
}

string evalTTL(vector<string> &args)
{
    string key = args[0];
    int ttl = getTTL(key);
    string ttls = to_string(ttl);
    // since we are taking it as alias(&) in the encode function,
    // we need to pass it as a variable explicilty
    return encode(ttls, false);
}


//DEL k1 k2 k3
string evalDEL(vector<string>&args){
    int count=0;
    for(string s:args){
        if(delKey(s)){
            count++;
        }
    }
    string temp=to_string(count);
    return encode(temp, false);
}

//EXPIRE k 10
string evalEXP(vector<string>&args){
    string temp;
    if(setEXP(args[0],stoi(args[1]))){
        temp="1";
        return encode(temp,false);
    }
    temp="0";
    return encode(temp,false);
}


string evalBGREWRITEAOF(){
    dumpAllAOF();
    return RESP_OK;
}


string evaluateAndRespond(const string &resp)
{
    if (resp.empty())
    {
        return "Error: Empty response";
    }
    string command = readCommand(resp).Cmd;
    transform(command.begin(), command.end(), command.begin(), ::toupper);
    vector<string> args;

    if (readCommand(resp).Args.size()>0)
    {
        args = readCommand(resp).Args;
    }
    cout << "Command: " << command
         << ", Args count: " << args.size() << endl;
    if (command == "PING")
    {
        if (args.empty())
        {
            return "+PONG\r\n";
        }
        else
        {
            string ans = "";
            for (const auto &a : args)
            {
                ans += a + " ";
            }
            return "+" + ans + "\r\n";
        }
    }

    else if (command == "SET")
    {
        if (args.size() > 1)
        {
            return evalSET(args);
        }
        return "-ERR wrong number of arguments for 'set' command\r\n";
    }

    else if (command == "GET")
    {
        if (args.size() > 0)
        {
            return evalGET(args);
        }
        return "-ERR wrong number of arguments for 'get' command\r\n";
    }

    else if(command=="TTL"){
        if(args.size()>0){
            return evalTTL(args);
        }
        return "-ERR wrong number of arguments for 'ttl' command\r\n";
    }
    else if(command=="DEL"){
        if(args.size()>0){
            return evalDEL(args);
        }
        return "-ERR wrong number of arguments for 'ttl' command\r\n";
    }
    else if(command == "EXPIRE"){
        if(args.size()>1){
            return evalEXP(args);
        }
        return "-ERR wrong number of arguments for 'expire' command\r\n";
    }
    else if(command=="BGREWRITEAOF"){
        return evalBGREWRITEAOF();
    }
    else
    {
        return "-ERR unknown command '" + command + "'\r\n";
    }
}