#pragma once
#include <string>
#include <vector>
#include <optional>
using namespace std;
struct command{
    string Cmd;
    vector<string> Args;
};


command readCommand(const string& resp);
string evaluateAndRespond(const string& resp);
string encode(string&data,bool isString);
void putKey(string& key,string& value,long long expiresAt=-1);
string getKey(string& key);
int getTTL(string& key);
bool delKey(string& key);
bool setEXP(string key, int t);