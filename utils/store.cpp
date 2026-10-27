#include<string>
#include<unordered_map>
#include<iostream>
#include<vector>
#include "utils.h"
#include "../config/config.h"
using namespace std;

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
void putKey(string& key,string& value,long long expiresAt){
    //Eviction is done when limit reaches
    if(mpp.size()>=keyLimit){
        evict();
    }
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

//Randomly(assuming unordered_map store would store randomly) selecting 20 of keys whose expiry is set 
//and deleting then with calculating the score
double expireSample(){
    int limit = 20;
    int sampled = 0, expiredCount = 0;
    long long now = time(nullptr);
    vector<string> keysToDelete;
    for (const auto& it : mpp)
    {
        if (it.second.expiresAt == -1)
            continue;

        limit--;
        sampled++;

        if (it.second.expiresAt <= now)
        {
            keysToDelete.push_back(it.first);
            expiredCount++;
        }

        if (limit == 0)
            break;
    }

    for (string key : keysToDelete)
        delKey(key);

    return expiredCount/20.0;
}


void deleteExpKeys(){
    //Continously deleting the keys till the fraction score gets to less than 25%
    //Which means we are continuing till we have less than 25% of keys with set expiry are not expiring currently
    while(true){
        double frac=expireSample();
        if(frac<0.25){
            break;
        }
    }
    cout<<"Keys Deleted"<<"Total keys"<<mpp.size()<<endl;
}