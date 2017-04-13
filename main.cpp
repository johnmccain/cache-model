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

#define DEBUG 0
#define DEBUG_CACHE 1

using namespace std;

int set_assoc;
int index_bits;

//used for fixing endianness
union Swapperoo {
	uint32_t i;
	uint8_t b[4];
};

typedef struct entry
{
	uint32_t address;
	bool valid;
} entry_t;

uint32_t little_endian_to_big_endian(uint32_t bytes) {
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

//for testing
uint32_t dec_to_bin(uint32_t val) {

	return (val == 0) ? 0 : (val % 2) + (10 * dec_to_bin(val / 2));
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

	index_bits = log2(CACHE_SIZE / (pow(2, BLOCK_BITS) * pow(2, set_assoc - 1)));
	char* file_name = argv[2];
	FILE *input_file = fopen(file_name, "rb");

	printf("index_bits = %d\n", index_bits);

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

	//fix byte-level endianness
	//we do this because the data is little endian, and our cache likes big endian addresses
	for(int i = 0; i < 10; ++i) {
		data[i] = little_endian_to_big_endian(data[i]);

		#if DEBUG
		int tag = get_tag(data[i]);
		int index = get_index(data[i]);
		int block = get_block(data[i]);
		printf("%032u\ntag\t%032u\nind\t%032u\nblo\t%032u\n", dec_to_bin(data[i]), dec_to_bin(tag), dec_to_bin(index), dec_to_bin(block));
		#endif
	}

	printf("fixed byte-level endianness\n");

	for(int i = 0; i < 32; ++i) {
		// printf("%032u\t", data[i]);
		int tag = get_tag(data[i]);
		int index = get_index(data[i]);
		int block = get_block(data[i]);
		// printf("%032u\t", data[i]);

		int addr_line = get_index(data[i]) % cache_lines;

		#if DEBUG_CACHE
		printf("\n------------------------\n");

		printf("ADDR %d: %u: %u  %u  %u\n", i, data[i], get_tag(data[i]), get_index(data[i]), get_block(data[i]));
		show_cache_line(cache[addr_line]);
		#endif

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
			#if DEBUG_CACHE
			printf("HIT\n");
			printf("\n------------------------\n");
			#endif
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
				cache[addr_line][set_num].valid = true;
			}
			#if DEBUG_CACHE
			printf("MISS\n");
			show_cache_line(cache[addr_line]);
			printf("\n------------------------\n");

			#endif
		}
	}

	free(data);

	printf("hits: %u\n", hits);
	printf("hit ratio: %f%%\n", ((double) hits) / NUM_ENTRIES);

	return 0;
}
