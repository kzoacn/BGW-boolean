#include "RecIO.hpp"
#include "RepIO.hpp"
#include "bgw.hpp"
#include <iostream>
#include <vector>

using namespace std;
using emp::RecIO;
using emp::RepIO;
using emp::Hash;

int party,port;

const int n=9;
const int open_num=2;

const int REP=1;



BigInt compute(int party,BigInt input,MPC *bgw){

    BigInt x=input;
    Int c[n+1];
    for(int i=1;i<=n;i++){
        bgw->set(c[i],x,i);
    }
    Int r;
    bgw->add(r,c[1],c[2]);;
    bgw->mul(r,r,c[3]);

    return bgw->reveal(r);
}


struct View{
    BigInt input;
    PRNG prng;
    vector<vector<char> >trans;
    void from_bin(unsigned char *in){
        int size;
        memcpy(&size,in,4);
        size+=4;
        input.from_bin(in+4,size);
        
        memcpy(prng.seed,in+size,sizeof(prng.seed));
        size+=sizeof(prng.seed);
        trans.resize(n+1);
        for(int i=1;i<=n;i++){
            int sz=0;
            memcpy(&sz,in+size,4);
            size+=4;
            
            trans[i].resize(sz);
            
            memcpy(trans[i].data(),in+size,sz);
            size+=sz;
        }
    }
    void to_bin(unsigned char *out){
        int size=input.size();
        memcpy(out,&size,4);
        size+=4;
        input.to_bin(out+4);
        memcpy(out+size,prng.seed,sizeof(prng.seed));
        size+=sizeof(prng.seed);
        for(int i=1;i<=n;i++){
            int sz=trans[i].size();
            memcpy(out+size,&sz,4);
            size+=4;
            memcpy(out+size,trans[i].data(),sz);
            size+=sz;
        }
    }
    int size(){
        int size=0;
        size+=4;
        size+=input.size();
        size+=sizeof(prng.seed);
        for(int i=1;i<=n;i++){
            int sz=trans[i].size();
            size+=4;
            size+=sz;
        }
        return size;
    }
    void digest(char *out){
        Hash view_hash;
        unsigned char *tmp=new unsigned char[size()];
        to_bin(tmp);
        view_hash.put(tmp,size());
        delete []tmp;
        view_hash.digest(out);
    }
};

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
    
    BigInt MOD(233);

    Hash view_all;
    vector<vector<char> >view_n;

    vector<vector<char> >views_hash;
    vector<View>views;
    views.resize(REP);

    for(int it=0;it<REP;it++){
        MPIO<RecIO,n> *io=new MPIO<RecIO,n>(party,ip,port,true);
        BGW<RecIO,n,n/2> *bgw=new BGW<RecIO,n,n/2>(io,party,MOD);
        BigInt input(party);
        BigInt res=compute(party,input,bgw);
        res.print();

        
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
        for(int i=2;i<=n;i++){
            int x=prng.rand_range(i-1)+1;
            swap(perm[i],perm[x]);
        }
        for(int i=1;i<=open_num;i++)if(party==perm[i]){
            int size=views[it].size();
            unsigned char *tmp=new unsigned char[size];
            views[it].to_bin(tmp);
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