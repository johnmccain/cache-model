/**
* @authors: John McCain & Dustin Wendt
*
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>

using namespace std;
int main()
{
	uint32_t a;
	clock_t start;
	double duration;
	vector<uint32_t> data;
	//int offset = 0;

	start = std::clock();
	ifstream file("AddressTrace_FirstIndex.bin",std::ifstream::binary);

	if(file.is_open())
	{
		while(!file.eof())
		{
			file.read((char*)&a, sizeof(a));
		}
	}
	file.close();
	duration = (clock() - start) / (double) CLOCKS_PER_SEC;
	cout << duration << "\n";




	return 0;
}
