all: clean cache_sim

cache_sim:
	g++ -std=c++11 -Wall -g main.cpp -o cache_sim

test:
	./cache_sim 1 AddressTrace_FirstIndex.bin

clean:
	rm cache_sim
