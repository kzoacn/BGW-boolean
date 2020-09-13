main: main.cpp
	g++ main.cpp -o main
test_io: test_io.cpp 
	g++ test_io.cpp -o test_io
clean:
	rm test_io
all: test_io
