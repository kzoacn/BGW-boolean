#include "bgw.hpp"
#include <iostream>
#include <vector>

using namespace std;

int party,port;

const int n=3;

int main(int argc,char **argv){
    if(argc!=3){
        puts("./main <party> <port>");
        return 0;
    }
    sscanf(argv[1],"%d",&party);
    sscanf(argv[2],"%d",&port);

    vector<string>ip;
    for(int i=0;i<=n;i++)
        ip.push_back(string("127.0.0.1"));
    
    MPIO<NetIO,n> *io=new MPIO<NetIO,n>(party,ip,port);

    BigInt MOD;
    MOD.from_ulong(233);

    BigInt one;one.from_ulong(123);


    BGW<NetIO,n,n/2> *bgw=new BGW<NetIO,n,n/2>(io,party,MOD);

    BigInt x;
    x.from_ulong(party);

    Int c[4];

    for(int i=1;i<=3;i++){
        bgw->set(c[i],x,i);
    }

    Int res;
    bgw->mul(res,c[2],c[3]);

    BigInt ret=bgw->reveal(res);

    ret.print();

    return 0;
}