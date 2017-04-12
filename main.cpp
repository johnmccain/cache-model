/**
* @author Dustin Wendt
* @author John McCain
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#define NUM_ENTRIES 67108864 //2^26

int set_assoc;

int main(int argc, char** argv)
{
	//Check if arguments are valid
	if(argc != 3) {
		printf("Usage: ./cache_sim <set_assoc (1 or 2)> <binary_file_name>\n");
		return 1;
	}

	set_assoc = argv[1][0] - '0';

	//Program only allows for 1 and 2-way set associative
	if(set_assoc != 1 && set_assoc != 2) {
		printf("Err: supported set associativity includes: 1, 2\n");
		return 1;
	}

	char* file_name = argv[2];
	FILE *input_file = fopen(file_name, "rb");

	//allocate space for entries on the heap. Sizeof(char) is a null terminator, because malloc is a C function
	uint32_t *data = (uint32_t *) malloc((sizeof(uint32_t) * NUM_ENTRIES) + sizeof(char));
	fread(data, (NUM_ENTRIES * 4), 1, input_file);
	fclose(input_file);

	//print out data read in
	for(int i = 0; i < NUM_ENTRIES; ++i) {
		printf("0x%08x\t", data[i]);
		if(i % 4 == 0) printf("\n");
	}


	free(data);

	return 0;
}
