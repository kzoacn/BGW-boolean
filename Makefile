prover: prover.cpp program.hpp
	g++ $< -o $@ -lcrypto
verifier: verifier.cpp program.hpp
	g++ $< -o $@ -lcrypto
clean:
	rm prover verifier
all: prover verifier
