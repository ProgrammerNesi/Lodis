#include <string>
#include <vector>
#include <optional>
#include <iostream>
using namespace std;

struct decodedMessage
{
    string message;
    int length;
    optional<vector<string>> array;
};

struct command
{
    string Cmd;
    string Arg;
    optional<vector<string>> Args;
};

decodedMessage decodeResp(const string &resp);
command readCommand(const string &resp)
{
    command cmd;
    decodedMessage decoded = decodeResp(resp);
    if (decoded.array.has_value())
    {
        if (decoded.array->size() > 0)
        {
            cmd.Cmd = decoded.array->at(0);
        }
        if (decoded.array->size() > 1)
        {

            if (decoded.array->size() == 2)
            {
                cmd.Arg = decoded.array->at(1);
            }
            else
            {
                vector<string> args(decoded.array->begin() + 1, decoded.array->end());
                cmd.Args = args;
                cmd.Arg = "placeholder"; // Placeholder value for Arg when Args is present
            }
        }
    }
    return cmd;
}

// Eg: +OK\r\n
decodedMessage decodeSimpleString(const string &resp)
{
    size_t pos = resp.find("\r\n");
    if (pos == string::npos)
    {
        return {"", 0};
    }
    decodedMessage decoded;
    decoded.message = resp.substr(1, pos - 1);
    decoded.length = pos + 2;
    return decoded;
}

// Eg: :1000\r\n
decodedMessage decodeInteger(const string &resp)
{
    size_t pos = resp.find("\r\n");
    if (pos == string::npos)
    {
        return {"", 0};
    }
    decodedMessage decoded;
    decoded.message = resp.substr(1, pos - 1);
    decoded.length = pos + 2;
    return decoded;
}

// Eg: $6\r\nfoobar\r\n
decodedMessage decodeBulkString(const string &resp)
{
    size_t pos = resp.find("\r\n");
    if (pos == string::npos)
    {
        return {"", 0};
    }
    int length = 0;
    try
    {
        length = stoi(resp.substr(1, pos - 1));
    }
    catch (...)
    {
        return {"", 0};
    }
    if (length == -1)
    {
        return {"", (int)pos + 2};
    }
    if (length < 0)
    {
        return {"", 0};
    }
    size_t start = pos + 2;
    decodedMessage decoded;
    decoded.message = resp.substr(start, length);
    decoded.length = length + start + 2;
    return decoded;
}

//Eg: *3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n
decodedMessage decodeArray(const string &resp)
{
    size_t pos = resp.find("\r\n");
    if (pos == string::npos)
    {
        return {"", 0};
    }
    int len;
    try
    {
        len = stoi(resp.substr(1, pos - 1));
    }
    catch (...)
    {
        return {"", 0};
    }
    if (len < 0)
    {
        return {"", 0};
    }

    pos += 2; //Skip the array length's \r\n (\r => 1, \n => 1)

    vector<string> ans;
    for (int i = 0; i < len; i++)
    {
        decodedMessage decoded = decodeResp(resp.substr(pos));

        if (decoded.length == 0)
        {
            return {"", 0};
        }
        ans.push_back(decoded.message);
        pos += decoded.length;
    }

    return {"placeholder", int(pos), ans};
}

decodedMessage decodeResp(const string &resp)
{

    if (resp.empty())
    {
        return {"", 0};
    }

    decodedMessage decoded;

    switch (resp[0])
    {
    case '+':
        return decodeSimpleString(resp);

    case ':':
        return decodeInteger(resp);

    case '$':
        return decodeBulkString(resp);
    case '*':
        return decodeArray(resp);
    default:
        return {"", 0};
    }
}
