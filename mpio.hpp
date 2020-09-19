#ifndef MP_IO_HPP
#define MP_IO_HPP

#include "RecIO.hpp"
#include "GF.hpp"
#include "constant.h"
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
    
        void send_GF(int i,GF x){
            send_io[i]->send_data(&x,sizeof(x));
            send_io[i]->flush();
        }
        void recv_GF(int i,GF &x){
            recv_io[i]->recv_data(&x,sizeof(x));
        }
    
};



#endif