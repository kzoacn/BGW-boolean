#include "RecIO.hpp"
#include "RepIO.hpp"
#include "bgw.hpp"
#include <iostream>
#include <vector>
#include "constant.h"
#include "program.hpp"

using namespace std;
using emp::RecIO;
using emp::RepIO;
using emp::Hash;

int party,port;



int main(int argc,char **argv){
    if(argc!=3){
        puts("./main <party> <port>");
        return 0;
    }
    sscanf(argv[1],"%d",&party);
    sscanf(argv[2],"%d",&port);

    vector<string>ip;
    for(int i=0;i<=n+1;i++)
        ip.push_back(string("127.0.0.1"));
    

    MOD.from_hex("73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001");
    Hash view_all;
    vector<vector<char> >view_n;

    vector<vector<char> >views_hash;
    vector<View<n> >views;
    views.resize(REP);

    for(int it=0;it<REP;it++){
        cerr<<"proveing "<<it<<endl;
        MPIO<RecIO,n> *io=new MPIO<RecIO,n>(party,ip,port,true);
        BGW<RecIO,n,n/2> *bgw=new BGW<RecIO,n,n/2>(io,party,MOD);
        BigInt input(party==1?1:0);
        BigInt res=compute(party,input,bgw);
        if(party==1)res.print();

        
        views[it].input=input;
        views[it].prng=bgw->prng;
        views[it].trans.resize(n+1);
        
        for(int i=1;i<=n;i++)if(i!=party){
            auto &vec=bgw->io->recv_io[i]->recv_rec; 
            views[it].trans[i]=vec;
        }



        views_hash.push_back(vector<char>());
        views_hash[it].resize(Hash::DIGEST_SIZE);
        views[it].digest(views_hash[it].data());
        
        view_all.put(views_hash[it].data(),views_hash[it].size());

        delete io;
        delete bgw;
    }
    view_n.resize(n+1);
    for(int i=1;i<=n;i++)
        view_n[i].resize(Hash::DIGEST_SIZE);
    view_all.digest(view_n[party].data());
    
    MPIO<RecIO,n> *io=new MPIO<RecIO,n>(party,ip,port);
    for(int i=1;i<=n;i++)
    for(int j=1;j<=n;j++)if(i!=j){
        if(i==party){
            io->send_data(j,view_n[i].data(),view_n[i].size());
        
        }
        if(j==party){
            io->recv_data(i,view_n[i].data(),view_n[i].size());
        }
    }
    delete io;

    view_all.reset();
    for(int i=1;i<=n;i++)
        view_all.put(view_n[i].data(),view_n[i].size());
    
    char r[Hash::DIGEST_SIZE];
    view_all.digest(r);
    PRNG prng;
    prng.reseed(r,sizeof(r));
    static int perm[n+1];
    for(int i=1;i<=n;i++)
        perm[i]=i;


    string name="view_"+to_string(party)+".bin";
    FILE *fp=fopen(name.c_str(),"wb");
    for(int it=0;it<REP;it++){
        fwrite(views_hash[it].data(),1,views_hash[it].size(),fp);
    }

    for(int it=0;it<REP;it++){
        do{
            for(int i=2;i<=n;i++){
                int x=prng.rand_range(i-1)+1;
                swap(perm[i],perm[x]);
            }
        }while(!check_perm(perm));
        
        for(int i=1;i<=open_num;i++)if(party==perm[i]){
            int size=views[it].size();
            unsigned char *tmp=new unsigned char[size];
            views[it].to_bin(tmp);
            fwrite(&size,1,4,fp);
            fwrite(tmp,1,size,fp);
        }
    }
    fclose(fp);

    //FS , random




/*
    MPIO<RepIO,n> *io2=new MPIO<RepIO,n>(party,ip,port);
    for(int i=1;i<=n;i++)if(i!=party){
        io2->recv_io[i]->recv_rec=io->recv_io[i]->recv_rec;
    }

    BGW<RepIO,n,n/2> *bgw2=new BGW<RepIO,n,n/2>(io2,party,MOD);
    
    bgw2->prng=bgw->prng;
    bgw2->prng.rewind();

    res=compute(party,bgw2);
    res.print();

*/


    return 0;
}