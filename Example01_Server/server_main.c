
#include "Server.h"

int main(int argc,char * argv [] )
{

	//CheakArgs(argc, 2);
	if (argc == 2)
	{
		if (CheakIsAnumber(argv[1]) != SUCCESS)
		{
			printf(" port is not a valid number , exit!\n ");
			exit(-1);
		}
		int port = atoi(argv[1]);
		MainServer(port);
		return 1;
	}
	else// defualt value in case port is not given by the user. 
	{
		MainServer(SERVER_PORT);
		return 1; 
	}
	return 0; 
}