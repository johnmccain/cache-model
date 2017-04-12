/**
* @author Dustin Wendt
* @author John McCain
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstdint>
#include <random>
#include <iostream>
#define NUM_ENTRIES 67108864 //2^26
#define CACHE_SIZE 32768 //32 * 2^10, in bytes
#define CACHE_ENTRIES 512 // 32 * 2^10 / 2^6
#define BLOCK_BITS 6

using namespace std;

int set_assoc;
int index_bits;

typedef struct entry
{
	uint32_t address;
	bool valid;
} entry_t;

uint32_t get_tag(uint32_t address) {
	return (address >> (BLOCK_BITS + index_bits));
}

uint32_t get_index(uint32_t address) {
	return (address >> BLOCK_BITS) % (int) pow(2, index_bits);
}

uint32_t get_block(uint32_t address) {
	return address % (int) pow(2, BLOCK_BITS);
}

int main(int argc, char** argv)
{
	double hits = 0.0;

	//Check if arguments are valid
	if(argc != 3) {
		printf("Usage: ./cache_sim <set_assoc (1 or 2)> <binary_file_name>\n");
		return 1;
	}

	set_assoc = argv[1][0] - '0';

	if(set_assoc % 2 && set_assoc != 1) {
		printf("Err: set associativity must be a power of 2\n");
		return 1;
	}

	if(set_assoc < 1) {
		printf("Err: set associativity must be at least 1\n");
		return 1;
	}

	index_bits = CACHE_SIZE / (pow(2, BLOCK_BITS) * pow(2, set_assoc - 1));
	char* file_name = argv[2];
	FILE *input_file = fopen(file_name, "rb");

	//allocate space for entries on the heap. Sizeof(char) is a null terminator, because malloc is a C function
	uint32_t *data = (uint32_t *) malloc((sizeof(uint32_t) * NUM_ENTRIES) + sizeof(char));
	fread(data, (NUM_ENTRIES * 4), 1, input_file);
	fclose(input_file);

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

	printf("spoopy\n");

	//print out data read in
	for(int i = 0; i < NUM_ENTRIES; ++i) {
		int addr_line = get_index(data[i]) % cache_lines;

		bool hit = false;

		//for each block in the corresponding cache set
		for(int j = 0; j < set_assoc; ++j) {
			//compare the tag associated with that block to the tag from the memory address
			if(get_tag(cache[addr_line][j].address) == get_tag(data[i]) && cache[addr_line][j].valid) {
				hit = true;
				break;
			}
		}

		if(hit) {
			hits++;
		} else {
			//cache miss, replace
			bool stored = false;
			//loop through to check for an invalid block to replace
			for(int j = 0; j < set_assoc; ++j) {
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
			}
		}

		//printf("0x%08x\t", data[i]);
		//if(i % 4 == 0) printf("\n");
	}

	free(data);

	cout << hits / (double)NUM_ENTRIES << "\n";

	return 0;
}
