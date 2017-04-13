all: cache_sim

cache_sim:
	g++ -std=c++11 -Wall -g main.cpp -o cache_sim

test:
	./cache_sim 1 AddressTrace_FirstIndex.bin

full_test:
	./cache_sim 1 AddressTrace_FirstIndex.bin
	./cache_sim 1 AddressTrace_LastIndex.bin
	./cache_sim 1 AddressTrace_RandomIndex.bin
	./cache_sim 2 AddressTrace_FirstIndex.bin
	./cache_sim 2 AddressTrace_LastIndex.bin
	./cache_sim 2 AddressTrace_RandomIndex.bin

clean:
	rm cache_sim
