#ifndef PROGRAM_HPP_
#define PROGRAM_HPP_

#include "bgw.hpp"
#include <iostream>
#include <vector>
#include "constant.h"
#include "bigint.hpp"
#include "poseidon_constant.h" 

using namespace std;

//#define RSA_IMPL

//#define HASH_IMPL

#define COMP_IMPL



#ifdef RSA_IMPL

#define MOD_STR "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784406918290641249515082189298559149176184502808489120072844992687392807287776735971418347270261896375014971824691165077613379859095700097330459748808428401797429100642458691817195118746121515172654632282216869987549182422433637259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133844143603833904414952634432190114657544454178424020924616515723350778707749817125772467962926386356373289912154831438167899885040445364023527381951378636564391212010397122822120720357"

BigInt compute(int party,BigInt input,MPC *bgw){

    Int sum,s2;
    bgw->set(sum,BigInt(0),0);
    Int tmp;
    for(int i=1;i<=n;i++){
        bgw->set(tmp,input,i);
        bgw->add(sum,sum,tmp);
    }

    bgw->mul(s2,sum,sum);
    bgw->mul(sum,sum,s2);
    
    return bgw->reveal(sum);
}


#endif

#ifdef HASH_IMPL

#define MOD_STR "52435875175126190479447740508185965837690552500527637822603658699938581184513"


const int NROUNDSF = 8;
int NROUNDSP[] = {56, 57, 56, 60, 60, 63, 64, 63};

Int zero;
Int five;
//Int MOD;
int len;

void ark(Int *state, vector<Int> c, int it,MPC *bgw) {
	for(int i = 0; i < len; i++) {
		//state[i]=state[i].add_mod(c[it+i],MOD);
        bgw->add(state[i],state[i],c[it+i]);
    }
}

void exp5(Int &a,MPC *bgw) {
	//a=a.pow_mod(five,MOD);
    Int a2,a4;
    bgw->mul(a2,a,a);
    bgw->mul(a4,a2,a2);
    bgw->mul(a,a,a4);
}

void sbox(int nRoundsF, int nRoundsP , Int *state,int i,MPC *bgw) {
	if (i < nRoundsF/2 || (i >= nRoundsF/2+nRoundsP) ){
		for (int j = 0; j < len; j++) {
			exp5(state[j],bgw);
		}
	} else {
		exp5(state[0],bgw);
	}
}

void mix(Int *state,Int *newState, vector<vector<Int> >m,MPC *bgw) {
	Int mul;
    bgw->set(mul,BigInt(0),0);
	for(int i = 0; i < len; i++) {
		bgw->set(newState[i],BigInt(0),0);
		for(int j = 0; j < len; j++) {
			//mul=m[j][i].mul_mod(state[j],MOD);
			bgw->mul(mul,m[j][i],state[j]);
            //newState[i]=newState[i].add_mod(mul,MOD);
            bgw->add(newState[i],newState[i],mul);
		}
	}
}
 

vector<vector<Int> >c;
vector<vector<vector<Int> > >m;

void poseidon_hash(vector<Int> input,Int &output,MPC *bgw) {
    len=input.size();
	int t = len + 1;
	if(len == 0 || len >= 8-1 ){
        puts("invalid inputs length");
        return ;
	}
    
    Int *state=new Int[t];
    for(int i=0;i<len;i++)
        state[i]=input[i];
    state[len-1]=zero;

	int nRoundsF = NROUNDSF;
	int nRoundsP = NROUNDSP[t-2];


    Int *newState=new Int[t]; 
    for(int i=0;i<t;i++)
        newState[i]=zero;

	// ARK --> SBox --> M, https://eprint.iacr.org/2019/458.pdf pag.5
	for(int i = 0; i < nRoundsF+nRoundsP; i++ ){
		ark(state, c[t-2], i*t,bgw);
		sbox(nRoundsF, nRoundsP, state, i,bgw); 
        if(i < nRoundsF+nRoundsP-1) {
			mix(state, newState, m[t-2],bgw);
			swap(state, newState);
		}
        
	}
	output = state[0];
}


    //MOD.from_hex("73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001");



BigInt compute(int party,BigInt input,MPC *bgw){

    bgw->set(zero,BigInt(0),0);
    bgw->set(five,BigInt(5),0);


    c.resize(_C.size());
    for(int i=0;i<(int)_C.size();i++){
        c[i].resize(_C[i].size());
        for(int j=0;j<(int)_C[i].size();j++){
            BigInt tmp;
            tmp.from_dec(_C[i][j].c_str());
            bgw->set(c[i][j],tmp,0);
        }
    }

    m.resize(_M.size());
    for(int i=0;i<(int)_M.size();i++){
        m[i].resize(_M[i].size());
        for(int j=0;j<(int)_M[i].size();j++){
            m[i][j].resize(_M[i][j].size());
            for(int k=0;k<(int)_M[i][j].size();k++){
                BigInt tmp;
                tmp.from_dec(_M[i][j][k].c_str());
                bgw->set(m[i][j][k],tmp,0);
            }
        }
    }

    Int sum;
    bgw->set(sum,BigInt(0),0);
    Int tmp;
    for(int i=1;i<=n;i++){
        bgw->set(tmp,input,i);
        bgw->add(sum,sum,tmp);
    }

    vector<Int>inputs;
    inputs.push_back(sum);
    Int output;
    poseidon_hash(inputs,output,bgw);

    return bgw->reveal(output);
}

#endif

#ifdef COMP_IMPL

#define MOD_STR "13"


vector<Int> adder(vector<Int> a,vector<Int> b,MPC *bgw){
    vector<Int>res;
    assert(a.size()==b.size());
    Int c;
    bgw->set(c,BigInt(0),0);
    for(int i=0;i<a.size();i++){
        Int p,q,r,t1,t2;
        bgw->oxor(p,a[i],b[i]);
        bgw->oand(q,a[i],b[i]);

        bgw->oxor(r,p,c);

        bgw->oand(t1,p,c);
        bgw->oxor(c,q,t1);

        res.push_back(r);
    }
    return res;
}

vector<Int> neg(vector<Int> a,MPC *bgw){
    vector<Int>c,one;
    c.resize(a.size());
    Int zero;
    bgw->set(zero,BigInt(0),0);
    for(int i=0;i<a.size();i++){
        bgw->onot(c[i],a[i]);
        one.push_back(zero);    
    }
    bgw->set(one[0],BigInt(1),0);
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


vector<BigInt> compute(int party,vector<BigInt> inputs,MPC *bgw){


    Int sum,s2;
    bgw->set(sum,BigInt(0),0);
    Int tmp;
    for(int i=1;i<=n;i++){
        BigInt in(party);
        bgw->set(tmp,in,i);
        bgw->add(sum,sum,tmp);
    }
    vector<BigInt>outputs;
    outputs.push_back(bgw->reveal(sum));
    return outputs;



    

    /*Int sum;
    bgw->set(sum,BigInt(0),0);
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

    vector<BigInt>res;
    for(int i=0;i<s.size();i++){
        res.push_back(bgw->reveal(s[i]));
    }

    return res;*/
}

#endif


#endif