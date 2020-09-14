#include "RecIO.hpp"
#include "RepIO.hpp"
#include "bgw.hpp"
#include <iostream>
#include <vector>
#include "constant.h"

using namespace std;
using emp::RecIO;
using emp::RepIO;
using emp::Hash;

int party,port;


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

    FILE *fp[n+1];
    for(int i=1;i<=n;i++){
        string name="view_"+to_string(i)+".bin";
        fp[i]=fopen(name.c_str(),"rb");
    }
    
    vector<vector<char> >view_n;
    vector<vector<vector<char> > >views_hash;


    view_n.resize(n+1);
    views_hash.resize(n+1);
    for(int i=1;i<=n;i++){
        Hash view_all;
        views_hash[i].resize(REP);
        for(int it=0;it<REP;it++){
            views_hash[i][it].resize(Hash::DIGEST_SIZE);
    
            fread(views_hash[i][it].data(),1,Hash::DIGEST_SIZE,fp[i]);
    
            view_all.put(views_hash[i][it].data(),Hash::DIGEST_SIZE);
        }
        view_n[i].resize(Hash::DIGEST_SIZE);
        view_all.digest(view_n[i].data());
    }

    Hash view_all;
    for(int i=1;i<=n;i++)
        view_all.put(view_n[i].data(),view_n[i].size());
    
    char r[Hash::DIGEST_SIZE];
    view_all.digest(r);
    PRNG prng;
    prng.reseed(r,sizeof(r));
    static int perm[n+1];
    for(int i=1;i<=n;i++)
        perm[i]=i;

    for(int it=0;it<REP;it++){
        for(int i=2;i<=n;i++){
            int x=prng.rand_range(i-1)+1;
            swap(perm[i],perm[x]);
        }
        cerr<<"open "<<perm[1]<<" "<<perm[2]<<endl;
        for(int i=1;i<=open_num;i++){
            /*
            int size=views[it].size();
            unsigned char *tmp=new unsigned char[size];
            views[it].to_bin(tmp);
            fwrite(tmp,1,size,fp);*/
        }
    }

    for(int i=1;i<=n;i++)
        fclose(fp[i]);



    return 0;
}