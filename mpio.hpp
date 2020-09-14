#ifndef MP_IO_HPP
#define MP_IO_HPP

#include "RecIO.hpp"
#include "bigint.hpp"
#include <vector>
#include <string>
using namespace std;

template<class IO,int n>
class MPIO{
public:
    int party;
    IO* send_io[n+1];
    IO* recv_io[n+1];

    MPIO(int party,vector<string> ip,int port,bool quiet=false){
        this->party=party;
        for(int i=1;i<=n;i++)
        for(int j=1;j<=n;j++)if(i!=j){
            if(i==party){
                send_io[j]=new IO(ip[j].c_str(),port+(i-1)*n+j,quiet);
            }
            if(j==party){
                recv_io[i]=new IO(NULL,port+(i-1)*n+j,quiet);
            }
        }
    }

    ~MPIO(){
        for(int i=1;i<=n;i++)if(i!=party){
            delete send_io[i];
            delete recv_io[i];
        }
    }

    void send_data(int i,const void *data,int len){
        send_io[i]->send_data(data,len);
        send_io[i]->flush();
    }
    void recv_data(int i,void *data,int len){
        recv_io[i]->recv_data(data,len);
    }
    
    void send_bigint(int i,BigInt x){
        int size=x.size();
        unsigned char *tmp=new unsigned char[size];
        x.to_bin(tmp);
        send_io[i]->send_data(&size,4);
        send_io[i]->send_data(tmp,size);
        send_io[i]->flush();
    }
    void recv_bigint(int i,BigInt &x){
        int size;
        recv_io[i]->recv_data(&size,4);
        unsigned char *tmp=new unsigned char[size];
        recv_io[i]->recv_data(tmp,size);
        x.from_bin(tmp,size);
    }

};



#endif