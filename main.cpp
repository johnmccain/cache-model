/**
* @authors: John McCain & Dustin Wendt
*
*/
#include <iostream>
#include <fstream>
#include <vector>

int main()
{
	uint32_t a;
	std::vector<uint32_t> data;
	//uint32_t data [64000000];
	int offset = 0;
	
	std::ifstream file("AddressTrace_FirstIndex.bin",std::ifstream::binary);

	if(file.is_open())
	{
		while(!file.eof())
		{
			file.read((char*)&a, sizeof(a));
		}
	}
	file.close();
	return 0;
}
