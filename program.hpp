#ifndef PROGRAM_HPP_
#define PROGRAM_HPP_

#include "bgw.hpp"
#include <iostream>
#include <vector>
#include "constant.h"
#include "GF.hpp"

using namespace std;




vector<Int> adder(vector<Int> a,vector<Int> b,MPC *bgw){
    vector<Int>res;
    assert(a.size()==b.size());
    Int c;
    bgw->set(c,GF(0),0);
    for(int i=0;i<a.size();i++){
        Int p,q,r,t1,t2;

        bgw->oxor(t1,a[i],c);
        bgw->oxor(t2,b[i],c);
        bgw->oxor(r,t1,b[i]);

        bgw->oand(t1,t1,t2);
        bgw->oxor(c,c,t1);
        /*bgw->oxor(p,a[i],b[i]);
        bgw->oand(q,a[i],b[i]);

        bgw->oxor(r,p,c);

        bgw->oand(t1,p,c);
        bgw->oxor(c,q,t1);*/

        res.push_back(r);
    }
    return res;
}

vector<Int> neg(vector<Int> a,MPC *bgw){
    vector<Int>c,one;
    c.resize(a.size());
    Int zero;
    bgw->set(zero,GF(0),0);
    for(int i=0;i<a.size();i++){
        bgw->onot(c[i],a[i]);
        one.push_back(zero);    
    }
    bgw->set(one[0],GF(1),0);
    c=adder(c,one,bgw);
    return c;
}


vector<Int> suber(vector<Int> a,vector<Int> b,MPC *bgw){
    vector<Int>b2;
    b2=neg(b,bgw);
    auto res=adder(a,b2,bgw);
    return res;
}
Int less_than(vector<Int> a,vector<Int> b,MPC *bgw){
    auto res=suber(a,b,bgw);
    return res[res.size()-1];
}


vector<GF> compute(int party,vector<GF> inputs,MPC *bgw){


    

    Int sum;
    bgw->set(sum,GF(0),0);
    Int tmp;
    vector<Int>bits[n+1];
    
    vector<Int>s;
    s.resize(inputs.size());
    for(int i=1;i<=n;i++){
        bits[i].resize(inputs.size());
        for(int j=0;j<inputs.size();j++){
            bgw->set(bits[i][j],inputs[j],i);
        }
        s=adder(s,bits[i],bgw);
    }

    vector<GF>res;
//    for(int i=0;i<s.size();i++){
//        res.push_back(bgw->reveal(s[i]));
//    }

    vector<Int> th;

    unsigned long long t=40;
    for(int i=0;i<(int)s.size();i++){
        Int b;
        bgw->set(b,GF(t>>i&1),0);
        th.push_back(b);
    }

    Int bit;
    bit=less_than(th,s,bgw);
    res.push_back(bgw->reveal(bit));
    

    return res;
}


#endif