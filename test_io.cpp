#include "io.hpp"
#include "mpio.hpp"
#include<iostream>
#include <vector>
#include <string>
using namespace std;


int party,port;

const int n=3;

int main(int argc,char **argv){
    if(argc!=3){
        puts("./test_io <party> <port>");
        return 0;
    }
    sscanf(argv[1],"%d",&party);
    sscanf(argv[2],"%d",&port);

    vector<string>ip;
    for(int i=0;i<=n;i++)
        ip.push_back(string("127.0.0.1"));
    

    /*if(party==1){
        NetIO *io=new NetIO("127.0.0.1",port);
        io->send_data("hi2",3);
    
    }
    if(party==2){
        NetIO *io=new NetIO(NULL,port);
        io->accepting();
        char tmp[4];memset(tmp,0,sizeof(tmp));
        io->recv_data(tmp,3);
        puts(tmp);
    }*/
    
    

    MPIO<NetIO,n> *io=new MPIO<NetIO,n>(party,ip,port);

    
    if(party==1){
        io->send_data(2,"hi2",3);
        io->send_data(3,"hi3",3);
    }
    if(party==2){
        char tmp[4];memset(tmp,0,sizeof(tmp));
        io->recv_data(1,tmp,3);
        puts(tmp);
    }
    if(party==3){
        char tmp[4];memset(tmp,0,sizeof(tmp));
        io->recv_data(1,tmp,3);
        puts(tmp);
    }
}