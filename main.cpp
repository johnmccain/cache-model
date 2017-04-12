/**
* @author Dustin Wendt
* @author John McCain
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstdint>
#define NUM_ENTRIES 67108864 //2^26
#define CACHE_SIZE 32768 //32 * 2^10, in bytes
#define CACHE_ENTRIES 512 // 32 * 2^10 / 2^6
#define BLOCK_BITS 6

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
	return (address >> BLOCK_BITS) % (uint32_t) pow(2, index_bits);
}

uint32_t get_block(uint32_t address) {
	return address % (uint32_t) pow(2, BLOCK_BITS);
}

int main(int argc, char** argv)
{
	int hits = 0;

	//Check if arguments are valid
	if(argc != 3) {
		printf("Usage: ./cache_sim <set_assoc (1 or 2)> <binary_file_name>\n");
		return 1;
	}

	set_assoc = argv[1][0] - '0';

	if(set_assoc % 2) {
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

	//print out data read in
	for(int i = 0; i < NUM_ENTRIES; ++i) {
		int addr_line = get_index(data[i]) % cache_lines;
		int addr_block = get_block(data[i]) % cache_blocks;
		if(cache[addr_line][addr_block].address == data[i] && cache[addr_line][addr_block].valid) {
			
		}

		printf("0x%08x\t", data[i]);
		if(i % 4 == 0) printf("\n");
	}

	free(data);

	return 0;
}
