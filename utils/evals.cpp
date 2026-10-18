#include<string>
#include "utils.h"
#include<algorithm>
using namespace std;


string evaluateAndRespond(const string& resp){
    if(resp.empty()){
        return "Error: Empty response";
    }
    string command=readCommand(resp).Cmd;
    transform(command.begin(),command.end(),command.begin(),::toupper);
    string arg=readCommand(resp).Arg;
    vector<string> args;
    if(readCommand(resp).Args.has_value()){
        args=readCommand(resp).Args.value();
    }
    if(command=="PING"){
        if(arg.empty()){    
            return "+PONG\r\n";
        }else{
            if(args.size()>0){
                string ans="";
                for(const auto& a:args){
                    ans+=a+" ";
                }
                return "+"+ans+"\r\n";
            }
            return "+"+arg+"\r\n"; 
        }
    }
    else{
        return "-ERR unknown command '"+command+"'\r\n";
    }

}