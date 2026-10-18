#pragma once
#include <string>
#include <vector>
#include <optional>
using namespace std;
struct command{
    string Cmd;
    string Arg;
    optional<vector<string>> Args;
};


command readCommand(const string& resp);
string evaluateAndRespond(const string& resp);
