#include<string>
#include<unordered_map>
using namespace std;
struct Obj
{
    string value;
    long long expiresAt = -1;
};

//The global store
unordered_map<string, Obj> mpp;


bool delKey(string& key){
    if(mpp.find(key)!=mpp.end()){
        mpp.erase(key);
        return true;
    }
    return false;
}
//Doing put operation on the global map
void putKey(string& key,string& value,long long expiresAt=-1){
    Obj obj;
    obj.value=value;
    obj.expiresAt=expiresAt;
    mpp[key]=obj;
}

string getKey(string& key){
    if (mpp.find(key) == mpp.end())
    {
        return "$-1\r\n";
    }
    auto it = mpp.find(key);
    long long now = time(nullptr);
    if (it->second.expiresAt != -1 &&
        it->second.expiresAt <= now)
    {
        delKey(key);
        return "$-1\r\n";
    }
    return it->second.value;
}


int getTTL(string& key){
    auto it = mpp.find(key);
    if (it == mpp.end()) {
        return -2;
    }
    if (it->second.expiresAt == -1) {
        return -1;
    }
    long long now = time(nullptr);
    if (it->second.expiresAt <= now) {
        mpp.erase(it);
        return -1;
    }
    return static_cast<int>(it->second.expiresAt - now);
}

bool setEXP(string key, int t){
    if(mpp.find(key)==mpp.end()){
        return false;
    }
    long long expiresAt = static_cast<long long>(time(nullptr)) + t;
    mpp[key].expiresAt=expiresAt;
    return true;
}