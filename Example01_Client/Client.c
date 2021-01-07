

#include "Client.h"

//---resources---

SOCKET m_socket=0;
SOCKADDR_IN clientService;
//HANDLE Send_event;
HANDLE hThread[NumOfClientThreads] = { NULL,NULL };

//---synch elements---


//---threads declarations--- 
typedef enum { menu, data }sel;
sel state;







int MainClient(int argc ,char *argv [] )
{
	//int port = atoi(argv[1]);

	//int ret_val = create_event_simple(&Send_event);
	/*if (ret_val != SUCCESS)
		return ret_val;*/
	int ret_val = 0;

	// TO-DO WARP WITH CREATE-SOCKET 
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	ret_val = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (ret_val != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		goto clean0;
	}

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		ret_val = m_socket;
		printf("Error at socket(): %ld\n", WSAGetLastError());
		goto clean1;
	}

	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(SERVER_ADDRESS_STR); //Setting the IP address to connect to
	clientService.sin_port = htons(SERVER_PORT); //Setting the port to connect to.

	ret_val=ConnectToServerWithUI(&m_socket, (SOCKADDR * )(&clientService), sizeof(clientService));
	if (ret_val != SUCCESS)
		goto clean11;
	printf("Connected to server on %s : %d\n", SERVER_ADDRESS_STR, SERVER_PORT);


	hThread[0] = CreateThreadSimple((LPTHREAD_START_ROUTINE)SendDataThread, NULL, NULL);
	if (hThread[0] == NULL)
	{
		printf("cannot open send Thread\n");
		goto clean2;
	}
	hThread[1] = CreateThreadSimple((LPTHREAD_START_ROUTINE)RecvDataThread,NULL,NULL);
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
	ret_val=WaitForMultipleObjectsWrap(NumOfClientThreads, hThread, INFINITE, TRUE);
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
	m_socket = NULL;
clean11:
	if (m_socket != NULL)
		closesocket(m_socket);
clean1:
	WSACleanup();
clean0 : 
	return ret_val;
	//return ret_val;
}

void CleanupWorkersThreadsSocketsClient()
{
	int Ind = 0;
	if (m_socket != 0)
	{
	CloseSocketGracefullyReciver(m_socket);
	m_socket = 0;
	}
	for (Ind = 0; Ind < NumOfClientThreads; Ind++)
	{
		if (hThread[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(hThread[Ind], TIME_OUT_THREADS);

			if (Res != WAIT_OBJECT_0)
			{
				TerminateThread(ThreadHandles[Ind], -1);
			}
			CloseHandle(ThreadHandles[Ind]);
			ThreadHandles[Ind] = NULL;
		}
	}
}


//Reading data coming from the server
int ConnectToServerWithUI(SOCKET* my_socket, SOCKADDR* my_clientService, int SizeOfclientService)
{
	char SendStr[256];//TO DO CHANGE TO CONSR IN MESSAGE 
	while (1)
	{
		if (connect(*my_socket, my_clientService, SizeOfclientService) == SOCKET_ERROR) {
			printf("Failed to Connect to server on %s : %d\n", SERVER_ADDRESS_STR, SERVER_PORT);
			printf("Choose what to do next:\n");
			printf("1. to Try to reconnect\n");
			printf("2. to Exit\n");
			gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard
			if (STRINGS_ARE_EQUAL(SendStr, "2"));
			return exitUser;
		}
		else
		{
			break;
		}
	}
	return SUCCESS;
}



//to-do change name 

static DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;
	char* AcceptedStr;
	while (1)
	{
		AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, m_socket);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			goto clean1;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Server disconected \n", SERVER_ADDRESS_STR, SERVER_PORT);
			RecvRes = ConnectToServerWithUI(&m_socket, &clientService, sizeof(clientService));
			if (RecvRes != SUCCESS)
				goto clean1;
			printf("Connected to server on %s : %d\n", SERVER_ADDRESS_STR, SERVER_PORT);
		}
		else
		{
			printf("%s\n", AcceptedStr);
			//TODO - parse the string to message, check type
			//if message type is menu
			//	state = menu
			//if type is no oppenent or something 
			// state = data or whatever
		}

		free(AcceptedStr);

	}
clean1:
	free(AcceptedStr);
	return RecvRes;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//to-do change name 

//Sending data to the server
static DWORD SendDataThread(void)
{
	char SendStr[256];
	TransferResult_t SendRes;
	SendString("CLIENT_REQUEST:gal", m_socket);
	//SendString("CLIENT_VERSUS", m_socket);  
	//printf("Insert username:");
	while (1)
	{
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard
		//message* user_mes = process_Message(SendStr, 0); //TO-DO DEFINE IS SERVER; 
		if (state == menu) {
			//print the menu
			//get the user input 
			//get_s() or new state for inputing 
			//if input is 2
			//SendString("CLIENT_VERSUS", m_socket);
		}
		//else if (state == )
		else if (state == data) {

		}
		//else...

		if (STRINGS_ARE_EQUAL(SendStr, "quit"))
			return exit; //"quit" signals an exit from the client side

		SendRes = SendString(SendStr, m_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return SendRes;
		}
	}
	return 1;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/







//
//
////change the recieved message to the accepted format.*
//char* ParseInputMessage(char* user_input, int mode)
//{
//
//	char* temp = NULL;
//	char* temp2;
//	switch (mode) {
//	case(NEW_USER_REQUEST):
//		temp = (char*)malloc(sizeof(user_input) + sizeof(server_Messages[mode]) + 10 * sizeof(char));
//		strcpy(temp, server_Messages[mode]);
//		strcat(temp, ":");
//		strcat(temp, user_input);
//		break;
//	case(PLAY_REQUEST):
//		temp = (char*)malloc(sizeof(user_input) + sizeof(server_Messages[mode]) + 10 * sizeof(char));
//		strcpy(temp, server_Messages[mode]);
//		temp2 = strchr(user_input, ' ');
//		temp2[0] = ':';
//		strcat(temp, temp2);
//		break;
//	case(SEND_MESSAGE):
//		temp = (char*)malloc(sizeof(user_input) + sizeof(server_Messages[mode]) + 100 * sizeof(char));
//		strcpy(temp, server_Messages[mode]);
//		temp2 = strchr(user_input, ' ');
//		temp2[0] = ':';
//		int counter = 0;
//		while (temp2[counter] != '\0')// put ";_;" insted of '_'
//		{
//			if (temp2[counter] == ' ') {
//				insert_substring(temp2, "; ;", counter + 1);
//				counter += 2;
//			}
//			else
//				counter++;
//		}
//		strcat(temp, temp2);
//		break;
//
//	default:
//		printf("Error, undefiened mode (cannot create message).\n");
//		break;
//	}
//
//	return temp;
//}

