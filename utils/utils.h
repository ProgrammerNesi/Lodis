#pragma once
#include <string>
#include <vector>
#include<unordered_map>

using namespace std;
struct command{
    string Cmd;
    vector<string> Args;
};
struct Obj
{
    string value;
    long long expiresAt = -1;
};

command readCommand(const string& resp);
string evaluateAndRespond(const string& resp);
string encode(string&data,bool isString);
void putKey(string& key,string& value,long long expiresAt=-1);
string getKey(string& key);
int getTTL(string& key);
bool delKey(string& key);
bool setEXP(string key, int t);
void deleteExpKeys();
extern std::unordered_map<std::string, Obj> mpp;
void evict();
void dumpAllAOF();