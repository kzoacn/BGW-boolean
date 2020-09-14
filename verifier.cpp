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
  
bool verify(){
    MOD.from_hex("73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001");

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
        cerr<<"checking "<<it<<endl;
        do{
            for(int i=2;i<=n;i++){
                int x=prng.rand_range(i-1)+1;
                swap(perm[i],perm[x]);
            }
        }while(!check_perm(perm));
        
        vector<View<n> >views;
        views.resize(n+1);
        for(int i=1;i<=open_num;i++){
            int x=perm[i];
            static unsigned char tmp[MAX_SIZE];
            int size;
            fread(&size,1,4,fp[x]);
            fread(tmp,1,size,fp[x]);
            views[x].from_bin(tmp);
        }

        for(int i=1;i<=open_num;i++){
            int x=perm[i];

            vector<string>ip(n+1,"");
            MPIO<RepIO,n> *io2=new MPIO<RepIO,n>(x,ip,0,true);

            for(int j=1;j<=n;j++)if(j!=x){
                io2->recv_io[j]->recv_rec=views[x].trans[j];
            }

            BGW<RepIO,n,n/2> *bgw2=new BGW<RepIO,n,n/2>(io2,x,MOD);
            
            bgw2->prng=views[x].prng;
            bgw2->prng.rewind();
            BigInt input=views[x].input;
            BigInt res=compute(x,input,bgw2);
            res.print();

            for(int j=1;j<=open_num;j++)if(i!=j){
                int y=perm[j];
                char tmp1[Hash::DIGEST_SIZE],tmp2[Hash::DIGEST_SIZE];
                io2->send_io[y]->send_hash.digest(tmp1);
                Hash h;
                h.put(views[y].trans[x].data(),views[y].trans[x].size());
                h.digest(tmp2);
                if(memcmp(tmp1,tmp2,Hash::DIGEST_SIZE)!=0){
                    cerr<<"error ! consistent"<<endl;
                    //return false;
                }
            }
            delete io2;
            delete bgw2;
        }
    }

    for(int i=1;i<=n;i++)
        fclose(fp[i]);

    return true;
}


int main(int argc,char **argv){

    
    if(verify()){
        puts("Yes");
    }else{
        puts("No");
    }


    return 0;
}