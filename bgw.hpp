#ifndef _BGW__HPP
#define _BGW__HPP

#include "GF.hpp"
#include "mpio.hpp"
#include <cassert>
#include "constant.h"
class Int{
public:
    GF val;
    
};


class MPC{
public:
    virtual void set(Int &c,GF a,int p)=0;

    virtual void onot(Int &c,const Int &a)=0;
    virtual void oxor(Int &c,const Int &a,const Int &b)=0;
    virtual void oand(Int &c,const Int &a,const Int &b)=0;
    
    virtual GF reveal(const Int &a)=0;
};

template<class IO,int n,int t>
class BGW : public MPC{

public:
    int party;
    MPIO<IO,n> *io;
    PRNG prng;

    GF lambda[n+1];    
    GF lambda_t[n+1];    
    int xor_cnt;
    int and_cnt;

    BGW(MPIO<IO,n> *io,int party){
        xor_cnt=and_cnt=0;
        this->io=io;
        this->party=party;
        //2*t+1==n
        //cerr<<"hi"<<endl;

        assert(2*t+1==n);
        GF zero(0);

        for(int i=1;i<=n;i++){
            GF num,den,xi,xj,tmp;
            num=1;
            den=1;
            xi=i;
            for(int j=1;j<=n;j++)if(i!=j){
                xj=j;
                tmp=zero.sub(xj);
                num=num.mul(tmp);
                den=den.mul(xi.sub(xj));
            }
            den=den.inv();
            lambda[i]=num.mul(den);
        }

        for(int i=1;i<=t+1;i++){
            GF num,den,xi,xj,tmp;
            num=1;
            den=1;
            xi=i;
            for(int j=1;j<=t+1;j++)if(i!=j){
                xj=j;
                tmp=zero.sub(xj);
                num=num.mul(tmp);
                den=den.mul(xi.sub(xj));
            }
            den=den.inv();
            lambda_t[i]=num.mul(den);
        }
    }

    ~BGW(){

    }

    void set(Int &c,GF a,int p){
        c=share(a,p);
    }

    Int share(GF a,int p){
        Int c;
        if(p==0){
            c.val=1;
            GF P=party;
            for(int i=1;i<=t;i++)
                c.val=c.val.mul(P);
            c.val=c.val.add(a);
            // f(x)=x^t+a
        }else{
            GF cof[t+1];
            cof[0]=a;
            
            for(int i=1;i<=t;i++)
                cof[i]=prng.rand_GF();
            
            if(p==party){
                GF sum(0);
                
                for(int i=1;i<=n;i++){
                    GF r=cof[t];
                    GF x=i;
                    for(int j=t-1;j>=0;j--){
                        r=r.mul(x);
                        r=r.add(cof[j]);
                    }
                    sum=sum.add(lambda[i].add(r));
                    
                    if(i==party){
                        c.val=r;
                    }else{
                        io->send_GF(i,r);    
                    }
                }

            }else{
                io->recv_GF(p,c.val);
            }
        }    
        return c;    
    }


    void oxor(Int &c,const Int &a,const Int &b){
       c.val=a.val;
       c.val=c.val.add(b.val);
       xor_cnt++;
    }

    void onot(Int &c,const Int &a){
        //(1-a)
        Int one;
        set(one,GF(1),0);

        oxor(c,a,one);
    }
    void oand(Int &c,const Int &a,const Int &b){
        and_cnt++;
        GF ab=a.val;
        ab=ab.mul(b.val);
        c.val=0;
        for(int i=1;i<=n;i++){
            Int tmp=share(ab,i);
            c.val=c.val.add(lambda[i].mul(tmp.val));
        }
    }
    

    GF reveal(const Int &a){
        GF point[n+1];
        point[party]=a.val;

        for(int i=1;i<=n;i++)
        for(int j=1;j<=n;j++)if(i!=j){
            if(i==party){
                io->send_GF(j,a.val);
            }
            if(j==party){
                io->recv_GF(i,point[i]);
            }
        } 
        GF ret(0);
        for(int i=1;i<=t+1;i++){
            GF tmp;
            tmp=lambda_t[i].mul(point[i]);
            ret=ret.add(tmp);
        }
        return ret;
    }

};



using emp::Hash;
template<int n>
struct View{
    vector<GF> inputs;
    PRNG prng;
    vector<vector<char> >trans;
    void from_bin(unsigned char *in){
        int size=0;
        int sz;
        memcpy(&sz,in,4);
        size+=4;
        inputs.resize(sz);
        for(int i=0;i<sz;i++){
            int sz;
            memcpy(&sz,in+size,4);
            size+=4;
            inputs[i].from_bin(in+size);
            size+=sz;    
        }

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
        int size=0;
        int sz=inputs.size();
        memcpy(out,&sz,4);
        size+=4;
        for(int i=0;i<inputs.size();i++){
            int sz=inputs[i].size();
            memcpy(out+size,&sz,4);
            size+=4; 
            inputs[i].to_bin(out+size);
            size+=sz;    
        }
        
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
        for(int i=0;i<inputs.size();i++){
            int sz=inputs[i].size();
            size+=4; 
            size+=sz;    
        }  
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
        memset(tmp,0,size());
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