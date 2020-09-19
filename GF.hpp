#ifndef _BIG_INT__
#define _BIG_INT__

#include <string>
#include <iostream>
#include <cstring>
#include "emp-tool/utils/hash.h"


unsigned int mod=283;
class GF { public:
	//BIGNUM *n = nullptr;
	unsigned char n;
	GF();
	GF(const GF &oth);
	GF(unsigned long long oth);
	GF &operator=(GF oth);
	bool operator==(GF oth)const;

	int size()const;
	void to_bin(unsigned char * in);
	void from_bin(const unsigned char * in);
    
	void print()const;

	GF add(const GF &oth);
	GF sub(const GF &oth);
	GF mul(const GF &oth);
	GF inv();
};	
GF inv_tab[256];

inline GF::GF() {
	n = 0;
}
inline GF::GF(const GF &oth) {
	n = oth.n;
}
inline GF::GF(unsigned long long num) {
	n = num;
}

inline GF& GF::operator=(GF oth) {
	n=oth.n;
	return *this;
}
inline bool GF::operator==(GF oth)const{
	return n==oth.n;
}

inline int GF::size() const{
	return 1;
}

inline void GF::to_bin(unsigned char * in) {
	memcpy(in,&n,sizeof(n));
}

inline void GF::from_bin(const unsigned char * in) {
	memcpy(&n,in,sizeof(n));
}


inline GF GF::add(const GF &oth) {
	return GF(n^oth.n);
}
inline GF GF::sub(const GF &oth) {
	return GF(n^oth.n);
}

inline GF GF::mul(const GF & b) {
	GF ret(0);
	unsigned int cur=n;
	for(int i=0;i<8;i++){
		if(b.n>>i&1)
			ret=ret.add(cur);
		cur=cur<<1;
		cur=std::min(cur,cur^mod);
	}
	return ret;
}

inline void GF::print()const{
	printf("%d\n",n);
}


inline GF GF::inv() {
	if(n==0)throw;

	if(inv_tab[0].n==0){
		inv_tab[0]=GF(1);
		for(int i=0;i<256;i++)
		for(int j=0;j<256;j++){
			GF x(i),y(j);
			if(x.mul(y)==GF(1))
				inv_tab[i]=j;
		}
	}
	return inv_tab[n];
}


class PRNG{
	emp::Hash hash;
	int counter=0;
	char dig[emp::Hash::DIGEST_SIZE*8];
public:
	unsigned char seed[256];
	PRNG(){
		FILE *fp=fopen("/dev/urandom","rb");
		fread(seed,1,256,fp);
		fclose(fp);
	}
	void rewind(){
		counter=0;
	}
	void reseed(const void *data,int len){
		memset(seed,0,sizeof(seed));
		memcpy(seed,data,std::min(len,256));
	}
	int rand_range(int m){
        hash.reset();
		hash.put(seed,256);
		hash.put(&counter,4);
		counter++;
		hash.digest(dig);
		int ret=*(int*)dig;
        ret=(ret%m+m)%m;
        return ret;
	}
    GF rand_GF(){
        GF ret;
		
			hash.reset();
			hash.put(seed,256);
			hash.put(&counter,4);
			counter++;
			hash.digest(dig);
		
		memcpy(&ret,dig,sizeof(ret));
        return ret;
    }
};



#endif