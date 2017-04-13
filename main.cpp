/**
* @author Dustin Wendt
* @author John McCain
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cstdint>
#include <random>
#include <iostream>
#include <ctime>
#define NUM_ENTRIES 67108864 //2^26
#define CACHE_SIZE 32768 //32 * 2^10, in bytes
#define CACHE_ENTRIES 512 // 32 * 2^10 / 2^6
#define BLOCK_BITS 6 //given in prompt

#define DEBUG 0
#define DEBUG_CACHE 0

using namespace std;

int set_assoc;
int index_bits;

//used for changing endianness
union Swapperoo {
	uint32_t i;
	uint8_t b[4];
};

typedef struct entry
{
	uint32_t address;
	bool valid;
} entry_t;

uint32_t change_endianness(uint32_t bytes) {
	union Swapperoo swap;
	swap.i = bytes;
	uint8_t tmp = swap.b[0];
	swap.b[0] = swap.b[3];
	swap.b[3] = tmp;
	tmp = swap.b[1];
	swap.b[1] = swap.b[2];
	swap.b[2] = tmp;
	return swap.i;
}

uint32_t get_tag(uint32_t address) {
	uint32_t tag = address >> (BLOCK_BITS + index_bits);
	uint32_t mask = (int) pow(2, BLOCK_BITS + index_bits) - 1;
	tag = tag & mask;

	#if DEBUG
	printf("tag(%u) = tag0(%u) & mask(%u)\n", tag, address >> (BLOCK_BITS + index_bits), mask);
	#endif
	return tag;
}

uint32_t get_index(uint32_t address) {
	uint32_t mask = (int) pow(2, index_bits) - 1;
	uint32_t index = (address >> BLOCK_BITS) & mask;

	#if DEBUG
	printf("index(%u) = index0(%u) & mask(%u)\n", index, address >> (BLOCK_BITS), mask);
	#endif
	return index;
}

uint32_t get_block(uint32_t address) {
	uint32_t mask = (int) pow(2, BLOCK_BITS) - 1;
	uint32_t block = address & mask;

	#if DEBUG
	printf("block(%u) = address(%u) & mask(%u)\n", block, address, mask);
	#endif
	return block;
}

//for debugging
void show_cache_line(entry_t *line) {
	for(int j = 0; j < set_assoc; ++j) {
		printf("%d:\n\t%d | %u  %u  %u\n", j, line[j].valid, get_tag(line[j].address), get_index(line[j].address), get_block(line[j].address));
	}
}

int main(int argc, char** argv)
{
	uint32_t hits = 0;
	uint32_t misses = 0;
	bool endian_alt = false;

	//Check if arguments are validhits
	if(argc != 3 && argc !=4) {
		printf("Usage: ./cache_sim <set_assoc (1 or 2)> <binary_file_name> [-e]\n");
		return 1;
	}

	set_assoc = argv[1][0] - '0';

	if(set_assoc < 1) {
		printf("Err: set associativity must be at least 1\n");
		return 1;
	}

	if(argc == 4 && strcmp(argv[3], "-e\0") == 0) {
		endian_alt = true;
	}

	index_bits = log2(CACHE_SIZE / (pow(2, BLOCK_BITS) * pow(2, set_assoc - 1)));

	char* file_name = argv[2];
	FILE *input_file = fopen(file_name, "rb");


	//allocate space for entries on the heap. Sizeof(char) is a null terminator, because malloc is a C function
	uint32_t *data = (uint32_t *) malloc((sizeof(uint32_t) * NUM_ENTRIES) + sizeof(char));
	fread(data, (NUM_ENTRIES * 4), 1, input_file);
	fclose(input_file);

	printf("Simulating a %d-way set associative L1 cache.  \nUsing address data from the file \"%s\"\n", set_assoc, file_name);

	//create the cache
	int cache_lines = CACHE_ENTRIES / set_assoc;
	int cache_blocks = set_assoc;
	entry_t **cache = new entry_t*[cache_lines];
	for(int i = 0; i < cache_lines; ++i) {
		cache[i] = new entry_t[cache_blocks];
		for(int j = 0; j < cache_blocks; ++j) {
			cache[i][j].valid = false;
		}
	}

	//change byte-level endianness if endian flag (-e) was set
	for(int i = 0; endian_alt && i < NUM_ENTRIES; ++i) {
		data[i] = change_endianness(data[i]);

		#if DEBUG
		int tag = get_tag(data[i]);
		int index = get_index(data[i]);
		int block = get_block(data[i]);
		printf("%032u\ntag\t%032u\nindex\t%032u\nblock\t%032u\n", data[i], tag, index, block);
		#endif
	}

	if(endian_alt) {
		printf("Changed endianness of addresses\n");
	}

	clock_t start;
	double duration;
	start = clock();

	//loop through each address and check for hits, add to cache if miss
	for(int i = 0; i < NUM_ENTRIES; ++i) {
		int index = get_index(data[i]);

		int addr_line = index % cache_lines;
		bool hit = false;

		//for each block in the corresponding cache set
		for(int j = 0; j < cache_blocks; ++j) {
			//compare the tag associated with that block to the tag from the memory address
			if(cache[addr_line][j].valid && get_tag(cache[addr_line][j].address) == get_tag(data[i])) {
				hit = true;
				break;
			}
		}

		if(hit) {
			//cache hit
			hits++;
			#if DEBUG_CACHE
			printf("HIT\n");
			printf("\n------------------------\n");
			#endif
		} else {
			//cache miss, replace
			misses++;
			bool stored = false;
			//loop through to check for an invalid block to replace
			for(int j = 0; j < cache_blocks; ++j) {
				//compare the tag associated with that block to the tag from the memory address
				if(!cache[addr_line][j].valid) {
					//found an invalid cache location, place there
					cache[addr_line][j].address = data[i];
					cache[addr_line][j].valid = true;
					stored = true;
					break;
				}
			}

			if(!stored) {
				//random replacement
				int set_num = rand() % set_assoc;
				cache[addr_line][set_num].address = data[i];
				cache[addr_line][set_num].valid = true;
			}
			#if DEBUG_CACHE
			printf("MISS\n");
			show_cache_line(cache[addr_line]);
			printf("\n------------------------\n");
			#endif
		}
	}
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;

	//deallocate dynamic memory
	free(data);
	for(int i = 0; i < cache_lines; ++i) {
		delete [] cache[i];
	}
	delete [] cache;

	printf("hits: %u\n", hits);
	printf("misses: %u\n", misses);
	printf("hit ratio: %f%%\n", ((double) hits * 100) / NUM_ENTRIES);
	printf("time taken (s): %f\n\n", duration);

	return 0;
}
