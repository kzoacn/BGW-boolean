prover: prover.cpp program.hpp constant.h
	g++ $< -o $@ -lcrypto -O2
verifier: verifier.cpp program.hpp constant.h
	g++ $< -o $@ -lcrypto -O2
clean:
	rm prover verifier
all: prover verifier
