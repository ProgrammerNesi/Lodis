#include<iostream>
#include<fstream>
#include "../config/config.h"
#include "utils.h"
using namespace std;

void dumpAllAOF(){
    ofstream file(AOFFIle);
    if(!file.is_open()){
        cout<<"Error opening the file";
        return;
    }
    cout<<"writing to AOF file"<<endl;

    //Currently we are implementing the complete file log dump when command is triggered for it
    //Also here we writing the plain command and data without encoding them
    //In acutal implementation the commands log are written in the aof file encoded in RESP
    for(auto it:mpp){
        string key=it.first;
        string val =it.second.value;
        string cmd="SET " + key +  " "  + val;
        file<<cmd;
    }
    file.close();
    cout<<"Written to AOF file"<<endl;

}