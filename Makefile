prover: prover.cpp program.hpp constant.h
	g++ $< -o $@ -lcrypto
verifier: verifier.cpp program.hpp constant.h
	g++ $< -o $@ -lcrypto
clean:
	rm prover verifier
all: prover verifier
