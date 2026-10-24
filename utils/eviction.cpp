#include "utils.h"

//Evicts the first key which is accessed by looping into the store
void evictFirst(){
    for(auto it : mpp){
        string k=it.first;
        delKey(k);
        return;
    }
}

//Calling any (now, evict first) Eviction strategy algorithm like LFU, LRU etc.
void evict(){
    evictFirst();
}