#include <stdio.h>
#include <stdlib.h>

#include "src/ml/network.h"

int main()
{
	char* file_name = "data/networks/xor.json";

	struct Network net = LoadNetworkFromJSON(file_name);

	for (double i = 0; i <= 1; i++)
	{
		for (double j = 0; j <= 1; j++)
		{
			double dataInput[2] = {i,j};
			printf("XOR(%lf,%lf) => %lf\n",i,j,*CalculateNetworkOutput(net, dataInput));
		}		
	}
	
}