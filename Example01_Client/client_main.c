
#include "Client.h"
#include "Functions.h"
#include "../Shared/HardCodedData.h"

SOCKET m_socket;
SOCKADDR_IN clientService;
int main(int argc ,  char* argv [] )
{
	//CheakArgs(argc, 4); 
	//if (CheakIsAnumber(argv[2] )!= SUCCESS)
	//{
	//	printf( " port is not a valid number , 
	//! ");
	//}

	
	
	//MainClient( argc, argv);
	int ret_val = 0;
	HANDLE hThread[NumOfClientThreads];
	// Initialize Winsock.
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	ret_val = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret_val != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		goto clean1;
	}
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		ret_val = m_socket;
		printf("Error at socket(): %ld\n", WSAGetLastError());
		goto clean1;
	}
	//Create a sockaddr_in object clientService and set  values.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); //Setting the IP address to connect to
	clientService.sin_port = htons(SERVER_PORT); //Setting the port to connect to.
	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.

	ret_val = ConnectToServerWithUI(&m_socket, (SOCKADDR*)(&clientService), sizeof(clientService));
	if (ret_val != SUCCESS)
		goto clean1;
	printf("Connected to server on %s : %d\n", SERVER_ADDRESS_STR, SERVER_PORT);
	//printf("un here");
	hThread[0] = CreateThreadSimple((LPTHREAD_START_ROUTINE)SendDataThread, NULL, NULL);
	if (hThread[0] == NULL)
	{
		printf("cannot open send Thread\n");
		WSACleanup();
		goto clean2;
	}
	hThread[1] = CreateThreadSimple((LPTHREAD_START_ROUTINE)RecvDataThread, NULL, NULL);
	if (hThread[1] == NULL)
	{
		printf("cannot open recv Thread");
		goto clean2;
	}
	//hThread[2] = CreateThreadSimple((LPTHREAD_START_ROUTINE)UIThread, NULL, NULL);
	//if (hThread[2] == NULL)
	//{
	//	printf("cannot open user interface thread.\n");
	//	goto clean2;
	//} 
	ret_val = WaitForMultipleObjectsWrap(NumOfClientThreads, hThread, INFINITE, TRUE);
	if (ret_val != SUCCESS)
	{
		KillThreads(hThread, NumOfClientThreads);
		goto clean3;
	}
	ret_val = SUCCESS;



clean3:	// close handles 
	Close_Threads(NumOfClientThreads, hThread);
clean2:
	CloseSocketGracefullySender(m_socket);
clean1:
	WSACleanup();
	return ret_val;
	//return ret_val;
}
