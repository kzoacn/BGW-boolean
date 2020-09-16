#ifndef _BGW__HPP
#define _BGW__HPP

#include "bigint.hpp"
#include "mpio.hpp"
#include <cassert>
#include "constant.h"
class Int{
public:
    BigInt val;
    
};


class MPC{
public:
    virtual void set(Int &c,BigInt a,int p)=0;
    virtual void add(Int &c,const Int &a,const Int &b)=0;
    virtual void mul(Int &c,const Int &a,const Int &b)=0;
    virtual BigInt reveal(const Int &a)=0;
};

template<class IO,int n,int t>
class BGW : public MPC{

public:
    BigInt MOD;
    int party;
    MPIO<IO,n> *io;
    PRNG prng;

    BigInt lambda[n+1];    
    BigInt lambda_t[n+1];    
    int add_cnt;
    int mul_cnt;

    BGW(MPIO<IO,n> *io,int party,BigInt MOD){
        add_cnt=mul_cnt=0;
        this->io=io;
        this->party=party;
        this->MOD=MOD;
        //2*t+1==n
        //cerr<<"hi"<<endl;

        assert(2*t+1==n);
        BigInt zero;zero.from_ulong(0);
        BigInt two;two.from_ulong(2);
        for(int i=1;i<=n;i++){
            BigInt num,den,xi,xj,tmp;
            num.from_ulong(1);
            den.from_ulong(1);
            xi.from_ulong(i);
            for(int j=1;j<=n;j++)if(i!=j){
                xj.from_ulong(j);
                tmp=zero.sub_mod(xj,MOD);
                num=num.mul_mod(tmp,MOD);
                den=den.mul_mod(xi.sub_mod(xj,MOD),MOD);
            }
            den=den.inv_mod(MOD);
            lambda[i]=num.mul_mod(den,MOD);
        }

        for(int i=1;i<=t+1;i++){
            BigInt num,den,xi,xj,tmp;
            num.from_ulong(1);
            den.from_ulong(1);
            xi.from_ulong(i);
            for(int j=1;j<=t+1;j++)if(i!=j){
                xj.from_ulong(j);
                tmp=zero.sub_mod(xj,MOD);
                num=num.mul_mod(tmp,MOD);
                den=den.mul_mod(xi.sub_mod(xj,MOD),MOD);
            }
            den=den.inv_mod(MOD);
            lambda_t[i]=num.mul_mod(den,MOD);
        }
    }

    ~BGW(){

    }

    void set(Int &c,BigInt a,int p){
        c=share(a,p);
    }

    void add(Int &c,const Int &a,const Int &b){
        c.val=a.val;
        c.val=c.val.add_mod(b.val,MOD);
        add_cnt++;
    }

    Int share(BigInt a,int p){
        Int c;
        if(p==0){
            c.val.from_ulong(1);
            BigInt P;
            P.from_ulong(party);
            for(int i=1;i<=t;i++)
                c.val=c.val.mul_mod(P,MOD);
            c.val=c.val.add_mod(a,MOD);
            // f(x)=x^t+a
        }else{
            BigInt cof[t+1];
            cof[0]=a;
            
            BigInt bound(10);
            for(int i=1;i<=t;i++)
                cof[i]=prng.rand_range(bound);
            
            if(p==party){
                BigInt sum(0);
                
                for(int i=1;i<=n;i++){
                    BigInt r=cof[t];
                    BigInt x;x.from_ulong(i);
                    for(int j=t-1;j>=0;j--){
                        r=r.mul_mod(x,MOD);
                        r=r.add_mod(cof[j],MOD);
                    }
                    sum=sum.add_mod(lambda[i].add(r),MOD);
                    

                    if(i==party){
                        c.val=r;
                    }else{
                        io->send_bigint(i,r);
                        
                    }
                }

            }else{
                io->recv_bigint(p,c.val);
            }
        }    
        return c;    
    }

    void mul(Int &c,const Int &a,const Int &b){
        mul_cnt++;
        BigInt ab=a.val;
        ab=ab.mul_mod(b.val,MOD);
        c.val.from_ulong(0);
        for(int i=1;i<=n;i++){
            Int tmp=share(ab,i);
            c.val=c.val.add_mod(lambda[i].mul_mod(tmp.val,MOD),MOD);
        }
    }

    BigInt reveal(const Int &a){
        BigInt point[n+1];
        point[party]=a.val;

        for(int i=1;i<=n;i++)
        for(int j=1;j<=n;j++)if(i!=j){
            if(i==party){
                io->send_bigint(j,a.val);
            }
            if(j==party){
                io->recv_bigint(i,point[i]);
            }
        } 
        BigInt ret(0);
        for(int i=1;i<=t+1;i++){
            BigInt tmp;
            tmp=lambda_t[i].mul_mod(point[i],MOD);
            ret=ret.add_mod(tmp,MOD);
        }
        return ret;
    }

};



using emp::Hash;
template<int n>
struct View{
    BigInt input;
    PRNG prng;
    vector<vector<char> >trans;
    void from_bin(unsigned char *in){
        int size;
        memcpy(&size,in,4);
        input.from_bin(in+4,size);
        size+=4;
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

bool check_perm(int *perm){
    
    int cnt[n/3];
    memset(cnt,0,sizeof(cnt));
    for(int i=1;i<=open_num;i++){
        cnt[perm[i]%3]++;
    }
    for(int i=0;i<n/3;i++)
        if(cnt[i]==3)
            return false;
    return true;
}


#endif