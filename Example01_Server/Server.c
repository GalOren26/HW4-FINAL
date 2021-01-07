

#include "Server.h"


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
//---resources---
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS] = { 0 };
HANDLE ExitThreadHandle;
HANDLE FailureThreadHandle;

SOCKET Sockets[NUM_OF_WORKER_THREADS] = { 0 };

char usernames[NUM_OF_WORKER_THREADS][MAX_LEN_NAME_PLAYER];

//---synch elements---
HANDLE file_binary_semphore;
HANDLE new_players_mutex;
HANDLE fileHandle = NULL;
HANDLE player1Event;
HANDLE player2Event;
HANDLE FailureEvent[NUM_OF_WORKER_THREADS];

//---threads declarations--- 

static DWORD ServiceThread(int me);
static DWORD InputExitThread(SOCKET* t_socket);
static DWORD FailureThread();
// threads function 
void CleanupWorkersThreadsSockets();
int FindFirstUnusedThreadSlot();
int InitSyncElements();
int ManageMessageReceived(message* lp_message, int me, SOCKET* t_socket, bool* OUT IOpenTheFile);
void MainServer(int port)
{
	//int res = OpenFileWrap(FILEPATH, OPEN_EXISTING, &fileHandle);	//TODO -- is file
	//if (res == SUCCESS) {
	//	CloseHandleWrap(fileHandle);
	//	DeleteFileA(FILEPATH);
	//}

	int Ind;
	SOCKET MainSocket = INVALID_SOCKET;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;
	int ret_val = 0;
	unsigned long Address;
	/// ------- create sync elements ------
	ret_val = InitSyncElements();
	if (ret_val != SUCCESS)
	{
		goto server_cleanup_0;
	}
	// create socket
	MainSocket = createSocket();
	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		goto server_cleanup_1;
	}
	//bind to socket 
	bindRes = bindWrap(&MainSocket, &service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
		goto server_cleanup_2;

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}
	//---------create connection end ---

	/*	----create Exit and failure  Threads----*/
	ExitThreadHandle = CreateThreadSimple((LPTHREAD_START_ROUTINE)InputExitThread, NULL, &MainSocket);
	if (ExitThreadHandle == NULL)
	{
		printf("failed create thread %d", GetLastError());
		goto server_cleanup_2;
	}
	FailureThreadHandle = CreateThreadSimple((LPTHREAD_START_ROUTINE)FailureThread, NULL,0 );
	if (FailureThreadHandle == NULL)
	{
		CloseHandle(FailureThreadHandle);
		printf("failed create thread %d", GetLastError());
		goto server_cleanup_2;
	}

	memset(usernames[0],0, MAX_LEN_NAME_PLAYER* NUM_OF_WORKER_THREADS);
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;
	// Initialize all thread handles to NULL, to mark that they have not been initialized

	while (true)
	{
		
		printf("Waiting for a client to connect...\n");

		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);//TO-DO dISCCONENT this socket in exit.
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed,server is closed .. , error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}
		printf("Client Connected.\n");

		Ind = FindFirstUnusedThreadSlot();

		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			printf("No slots available for client, dropping the connection.\n");
			SendString("SERVER_DENIED:Server already has two players,try later:)", MainSocket);
		}
		else
		{
			Sockets[Ind] = AcceptSocket; // shallow copy: don't close   AcceptSocket, instead close    ThreadInputs[Ind] when the   time comes.
			ThreadHandles[Ind] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServiceThread, Ind, 0, NULL);
			if (ThreadHandles[Ind] == NULL)
			{
				printf("failed create thread %d", GetLastError());
				goto server_cleanup_3;
			}


		}
	}

server_cleanup_3:
	//clean up all threads and sockets.
	CleanupWorkersThreadsSockets();
server_cleanup_2:
	if (MainSocket != NULL)
	{
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
	}
server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
server_cleanup_0:

	CloseHandle(new_players_mutex);
	CloseHandle(file_binary_semphore);
	CloseHandle(FailureEvent[1]);
	CloseHandle(FailureEvent[0]);
	CloseHandle(player1Event);
	CloseHandle(player2Event);
}

//Service thread is the thread that opens for each successful client connection and "talks" to the client.

static DWORD ServiceThread(int me)
{

	SOCKET* t_socket = &(Sockets[me]);
	char SendStr[MAX_LEN_MESSAGE];
	int other = 0;
	if (other == me)
		other = 1;

	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	bool IOpenTheFile = false;
	strcpy(SendStr, "Welcome to this server!");
	SendRes = SendString("Welcome to this server!", *t_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing to client.\n");
		SetEvent(FailureEvent[me]);
		return SendRes;
	}
	char* user_name = NULL;
	//game state machine
	while (!Done)
	{
		char* AcceptedStr = NULL;
		message* lp_message = NULL;
		RecvRes = ReceiveString(&AcceptedStr, *t_socket);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			SetEvent(FailureEvent[me]);
			return RecvRes;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			SetEvent(FailureEvent[me]);
			return RecvRes;
		}
		else
		{
			printf("Got string : %s\n", AcceptedStr);
			lp_message = NULL;
			lp_message = process_Message(AcceptedStr, IsServer);
			ManageMessageReceived(lp_message, me, t_socket, &IOpenTheFile);//to do cheak return value and manage it 
			//cheak-return value
		}
	}
	printf("Conversation ended.\n");
	/*closesocket(*t_socket);*/
	return 0;
}

static DWORD InputExitThread(SOCKET* t_socket)
{
	char input_str[MAX_LEN_MESSAGE];
	while (true)
	{
		gets_s(input_str, sizeof(input_str)); //Reading a string from the keyboard
		if (STRINGS_ARE_EQUAL(input_str, "exit"))
		{
			printf("quit is received by the server admin, server quit!\n");
			closesocket(*t_socket);
			break;
		}
	}
}

static DWORD FailureThread()
{
	DWORD ret_val1 = 0;
	while (1)
	{
		for (int i = 0; i < NUM_OF_WORKER_THREADS;i++)
		{
			ret_val1 = WaitForSingleObject(FailureEvent[i], 0);
			if (ret_val1 == WAIT_OBJECT_0)
			{
				int other = (i + 1) % 2;
				SendString("SERVER_QUIT_OPPONENT", Sockets[other]);
				SendString("SERVER_MAIN_MENU", Sockets[other]);
				//ResetEvent(FailureEvent[i]);
				CloseSocketGracefullySender(Sockets[i]);
			}
		}
	}

}


/*oOoOo------FunctionsServer------oOoOoOoOoOoOoOoOoO*/

int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}



void CleanupWorkersThreadsSockets()
{
	int Ind = 0;
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (Sockets[Ind] != 0)
		{
			CloseSocketGracefullySender(Sockets[Ind]);
			Sockets[Ind] = 0;
		}
	}

	//int ret_val= WaitForMultipleObjectsWrap(2, ThreadHandles, TIME_OUT_THREADS, TRUE);//to do -- inside the threads cheak for failure of socket
	//if (ret_val != SUCCESS)
	//{
	//	KillThreads(ThreadHandles, NUM_OF_WORKER_THREADS);
	//}
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], TIME_OUT_THREADS);

			if (Res != WAIT_OBJECT_0)
			{
				TerminateThread(ThreadHandles[Ind], -1);
			}
			CloseHandle(ThreadHandles[Ind]);
			ThreadHandles[Ind] = NULL;
		}
	}
	CloseHandle(FailureThreadHandle);
	CloseHandle(ExitThreadHandle);
}


int InitSyncElements()
{
	int ret_val = 0;
	/// ------- create sync elements------
	ret_val = create_event_simple(&player1Event);// TO-DO CHEACK ALLOC
	if (ret_val != SUCCESS)
		goto clean0;
	ret_val = create_event_simple(&player2Event);// TO-DO CHEACK ALLOC
	if (ret_val != SUCCESS)
	{
		goto clean1;
	}
	ret_val = CreateSemphoreWrap(1, &file_binary_semphore, 1);//todo-cheak for failure 		
	if (ret_val != SUCCESS)
	{
		goto clean2;

	}
	ret_val = create_event_simple(&FailureEvent[0]);//todo-cheak for failure 
	if (ret_val != SUCCESS)
	{
		goto clean3;
	}
	ret_val = create_event_simple(&FailureEvent[1]);
	if (ret_val != SUCCESS)
	{
		goto clean4;
	}
	return SUCCESS;


clean4:
	CloseHandle(FailureEvent[0]);
clean3:
	CloseHandle(file_binary_semphore);
clean2:
	CloseHandle(player2Event);
clean1:
	CloseHandle(player1Event);
clean0:
	return ret_val;
}

int ManageMessageReceived(message* lp_message, int me, SOCKET* t_socket, bool* OUT IOpenTheFile)
{
	char SendStr[MAX_LEN_MESSAGE];
	int ret_val1 = 0;
	int ret_val2 = 0;
	int other = (me + 1) % 2;

	if (lp_message->ServerType == CLIENT_REQUEST)
	{

		strcpy(usernames[me],(const char*) lp_message->message_arguments[0]);
		ret_val1 = SendString("SERVER_APPROVED", *t_socket);
		ret_val2 = SendString("SERVER_MAIN_MENU", *t_socket);
		if (ret_val1 == TRNS_FAILED || ret_val2 == TRNS_FAILED)
		{
			ret_val1 = TRNS_FAILED;
			printf("Service socket error while reading, closing thread.\n");
			goto clean1_mes;
		}
		free(lp_message);
		return SUCCESS;
	}


	else if (lp_message->ServerType == CLIENT_VERSUS)
	{
		//first player create the file 
		ret_val1 = WaitForSingleObjectWrap(file_binary_semphore, TIME_OUT_THREADS);
		if (ret_val1 != SUCCESS)
			goto clean1_mes;
		//open file
		if (!PathFileExistsA(FILEPATH))
		{
			ret_val1 = OpenFileWrap(FILEPATH, CREATE_ALWAYS, &fileHandle);
			if (ret_val1 != SUCCESS)
				goto clean1_mes;
			*IOpenTheFile = true;
		}
		ReleaseSemphoreWrap(file_binary_semphore, 1);

		//first user 
		int res;
		if (*IOpenTheFile == true) {
			SetEvent(player1Event);
			res = WaitForSingleObjectWrap(player2Event, TIME_OUT_THREADS);
			if (res != SUCCESS)
				ResetEvent(player1Event);
		}
		//second user 
		else {	// corner  case: first player schduale here  between else and set_event so second thread not set it event and first is time out .

			SetEvent(player2Event);
			res = WaitForSingleObjectWrap(player1Event, TIME_OUT_THREADS);
			if (res != SUCCESS)
			{
				ResetEvent(player2Event);
			}
		}
		//case of time out of one of the users 
		if (res != SUCCESS)
		{
			CloseHandleWrap(fileHandle);
			fileHandle = NULL;
			DeleteFileA(FILEPATH);
			ret_val1 = SendString("SERVER_NO_OPPONENTS", *t_socket);
			ret_val2 = SendString("SERVER_MAIN_MENU", *t_socket);
			if (ret_val1 == TRNS_FAILED || ret_val2 == TRNS_FAILED)
			{
				ret_val1 = TRNS_FAILED;
				printf("Service socket error while reading, closing thread.\n");
				goto clean1_mes;
			}
			free(lp_message);
			return SUCCESS;
		}

		//2 players in the game
		message invite;
		invite.ServerType = SERVER_INVITE;
		invite.ClientType = -1;
		strcpy(invite.message_arguments[0], (const char *)usernames[me]);
		MessageToString(&invite, IsServer, SendStr);
		//strcat(invite, "SERVER_INVITE:");
		//strcat(invite + strlen("SERVER_INVITE:"), usernames[other]);
		ret_val1 = SendString(SendStr, *t_socket);
		if (ret_val1 == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			goto clean1_mes;
		}
		return SUCCESS;

	}
	//else if (STRINGS_ARE_EQUAL(AcceptedStr, "bye"))
	//{
	//strcpy(SendStr, "see ya!");
	//Done = TRUE;
	//}
	//else
	//{
	//strcpy(SendStr, "I don't understand");
	//}
	////free(AcceptedStr); //will delete the lp_message data also.

clean1_mes:
	SetEvent(FailureEvent[me]);
	free(lp_message);
	return ret_val1;
}




//ClientMesType FindMeesageType(message* ClientMes, int is_server)
//{
//
//	for (ClientMesType i = 0;i < end; i++)
//	{
//		if (STRINGS_ARE_EQUAL(ClientMes->message_type, server_messages[i]))
//		{
//			return i; 
//		}
//	}
//	return -1; 
//}



