#ifndef _BIG_INT__
#define _BIG_INT__

#include <openssl/bn.h>
#include <openssl/rand.h>
#include <string>
#include <iostream>
#include <cstring>
#include "emp-tool/utils/hash.h"
using std::string;

BN_CTX *default_ctx=BN_CTX_new();

class BigInt { public:
	BIGNUM *n = nullptr;
	BigInt();
	BigInt(const BigInt &oth);
	BigInt(unsigned long long oth);
	BigInt &operator=(BigInt oth);
	~BigInt();

	int size();
	void to_bin(unsigned char * in);
	void from_bin(const unsigned char * in, int length);
    void from_dec(const char *str);
    void from_hex(const char *str);
    void from_ulong(unsigned long long x);
	void print()const;

	BigInt add(const BigInt &oth);
	BigInt sub(const BigInt &oth);
	BigInt mul(const BigInt &oth, BN_CTX *ctx = default_ctx);
	BigInt mod(const BigInt &oth, BN_CTX *ctx = default_ctx);
	BigInt add_mod(const BigInt & b, const BigInt& m, BN_CTX *ctx = default_ctx);
	BigInt sub_mod(const BigInt & b, const BigInt& m, BN_CTX *ctx = default_ctx);
	BigInt mul_mod(const BigInt & b, const BigInt& m, BN_CTX *ctx = default_ctx);
	BigInt pow_mod(const BigInt &k, const BigInt& m, BN_CTX *ctx = default_ctx);
};	


inline BigInt::BigInt() {
	n = BN_new();
}
inline BigInt::BigInt(const BigInt &oth) {
	n = BN_new();
	BN_copy(n, oth.n);
}
inline BigInt::BigInt(unsigned long long oth) {
	from_ulong(oth);
}
inline BigInt& BigInt::operator=(BigInt oth) {
	std::swap(n, oth.n);
	return *this;
}
inline BigInt::~BigInt() {
	if (n != nullptr)
		BN_free(n);
}

inline int BigInt::size() {
	return BN_num_bytes(n);
}

inline void BigInt::to_bin(unsigned char * in) {
	BN_bn2bin(n, in);
}

inline void BigInt::from_bin(const unsigned char * in, int length) {
	BN_free(n);
	n = BN_bin2bn(in, length, nullptr);
}
inline void BigInt::from_dec(const char *str) {
	BN_free(n);
	n=NULL;
    BN_dec2bn(&n,str);
}
inline void BigInt::from_hex(const char *str) {
	BN_free(n);
	n=NULL;
    BN_hex2bn(&n,str);
}
inline void BigInt::from_ulong(unsigned long long x) {
	string s=std::to_string(x);
    from_dec(s.c_str());
}

inline void BigInt::print() const{// memory leak ?
	puts(BN_bn2dec(n));
}

inline BigInt BigInt::add(const BigInt &oth) {
	BigInt ret;
	BN_add(ret.n, n, oth.n);
	return ret;
}
inline BigInt BigInt::sub(const BigInt &oth) {
	BigInt ret;
	BN_sub(ret.n, n, oth.n);
	return ret;
}

inline BigInt BigInt::mul_mod(const BigInt & b, const BigInt &m,  BN_CTX *ctx) {
	BigInt ret;
	BN_mod_mul(ret.n, n, b.n, m.n, ctx);
	return ret;
}

inline BigInt BigInt::add_mod(const BigInt & b, const BigInt &m,  BN_CTX *ctx) {
	BigInt ret;
	BN_mod_add(ret.n, n, b.n, m.n, ctx);
	return ret;
}
inline BigInt BigInt::sub_mod(const BigInt & b, const BigInt &m,  BN_CTX *ctx) {
	BigInt ret;
	BN_mod_sub(ret.n, n, b.n, m.n, ctx);
	return ret;
}
inline BigInt BigInt::pow_mod(const BigInt & b, const BigInt &m,  BN_CTX *ctx) {
	BigInt ret;
	BN_mod_exp(ret.n,n,b.n,m.n,ctx);
	return ret;
}

inline BigInt BigInt::mul(const BigInt &oth, BN_CTX *ctx) {
	BigInt ret;
	BN_mul(ret.n, n, oth.n, ctx);
	return ret;
}

inline BigInt BigInt::mod(const BigInt &oth, BN_CTX *ctx) {
	BigInt ret;
	BN_mod(ret.n, n, oth.n, ctx);
	return ret;
}

class PRNG{
	emp::Hash hash;
	int counter=0;
	char dig[emp::Hash::DIGEST_SIZE];
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
    BigInt rand_range(const BigInt &m){
        BigInt ret;
		hash.reset();
		hash.put(seed,256);
		hash.put(&counter,4);
		counter++;
		hash.digest(dig);
		BN_bin2bn((unsigned char*)dig,sizeof(dig),ret.n);
        ret=ret.mod(m);
        return ret;
    }
};



#endif