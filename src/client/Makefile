STD_FLAG:=-std=c++20

LD_FLAGS:=-lspdlog -lfmt

all: make_dir ../../build/client

make_dir:
	mkdir -p ../../build

../../build/client:../../build/client_main.o ../client_main.cc ../../build/net.o ../utils/net/net.cc ../client_main.cc client.cc ../../build/client.o 
	g++ $(STD_FLAG) ../../build/client_main.o ../../build/client.o  ../../build/net.o -o ../../build/client $(LD_FLAGS) 
../../build/client_main.o:
	g++ $(STD_FLAG) -c ../client_main.cc -o ../../build/client_main.o 
../../build/net.o:../utils/net/net.cc
	g++ $(STD_FLAG) -c ../utils/net/net.cc -o ../../build/net.o

../../build/client.o:client.cc
	g++ $(STD_FLAG) -c client.cc -o ../../build/client.o 

clean:
	rm -rf ../../build