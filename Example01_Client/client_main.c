
#include "Client.h"
#include "Functions.h"
#include "../Shared/HardCodedData.h"
int main(int argc ,  char* argv [] )
{
	//CheakArgs(argc, 4); 
	//if (CheakIsAnumber(argv[2] )!= SUCCESS)
	//{
	//	printf( " port is not a valid number , 
	//! ");
	//}

	if (argc != 4)
	{
		printf("invalid number of arguments,please enter argumants in format : ip port username");
		exit(-1);
	}
	else
	{
		if (CheakIsAnumber(argv[2]) != SUCCESS)
		{
			printf(" port is not a valid number , exit!\n ");
			exit(-1);
		}
		int port = atoi(argv[2]);
		char* ip = argv[1];
		char* username= argv[3];
		MainClient(port, ip, username);
	}
	return 0;
}