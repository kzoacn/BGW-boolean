#ifndef PROGRAM_HPP_
#define PROGRAM_HPP_

#include "bgw.hpp"
#include <iostream>
#include <vector>
#include "constant.h"
#include "bigint.hpp"
#include "poseidon_constant.h" 

using namespace std;


const int NROUNDSF = 8;
int NROUNDSP[] = {56, 57, 56, 60, 60, 63, 64, 63};

Int zero;
Int five;
//Int MOD;
//int len;
/*
void ark(Int *state, vector<Int> c, int it) {
	for(int i = 0; i < len; i++) {
		state[i]=state[i].add_mod(c[it+i],MOD);
	}
}

void exp5(Int &a) {
	a=a.pow_mod(five,MOD);
}

void sbox(int nRoundsF, int nRoundsP , Int *state,int i) {
	if (i < nRoundsF/2 || (i >= nRoundsF/2+nRoundsP) ){
		for (int j = 0; j < len; j++) {
			exp5(state[j]);
		}
	} else {
		exp5(state[0]);
	}
}

void mix(Int *state,Int *newState, vector<vector<Int> >m) {
	Int mul(0);
	for(int i = 0; i < len; i++) {
		newState[i]=0;
		for(int j = 0; j < len; j++) {
			mul=m[j][i].mul_mod(state[j],MOD);
			newState[i]=newState[i].add_mod(mul,MOD);
		}
	}
}
 

vector<vector<Int> >c;
vector<vector<vector<Int> > >m;

void Hash(vector<Int> input,Int &output) {
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
		ark(state, c[t-2], i*t);
		sbox(nRoundsF, nRoundsP, state, i); 
        if(i < nRoundsF+nRoundsP-1) {
			mix(state, newState, m[t-2]);
			swap(state, newState);
		}
        
	}
	output = state[0];
}*/
/*
int main(int argc,char **argv){ 

    MOD.from_hex("73eda753299d7d483339d80809a1d80553bda402fffe5bfeffffffff00000001");

    c.resize(_C.size());
    for(int i=0;i<(int)_C.size();i++){
        c[i].resize(_C[i].size());
        for(int j=0;j<(int)_C[i].size();j++)
            c[i][j].from_dec(_C[i][j].c_str());
    }

    m.resize(_M.size());
    for(int i=0;i<(int)_M.size();i++){
        m[i].resize(_M[i].size());
        for(int j=0;j<(int)_M[i].size();j++){
            m[i][j].resize(_M[i][j].size());
            for(int k=0;k<(int)_M[i][j].size();k++)
                m[i][j][k].from_dec(_M[i][j][k].c_str());
        }
    }

    Int one(1);
    vector<Int>input;
    input.push_back(one);
    Int output;
    Hash(input,output);
    output.print();
 
    return 0;
}*/



BigInt compute(int party,BigInt input,MPC *bgw){

    bgw->set(zero,BigInt(0),0);
    bgw->set(five,BigInt(5),0);

    return bgw->reveal(five);
}

#endif