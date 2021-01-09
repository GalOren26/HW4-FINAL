

#include "Server.h"


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
//---resources---
HANDLE ThreadHandles[NUM_OF_WORKER_THREADS] = { 0 };
HANDLE ExitThreadHandle;
SOCKET Sockets[NUM_OF_WORKER_THREADS] = { 0 };
bool ServerInitateShutDown = false;
char usernames[NUM_OF_WORKER_THREADS][MAX_LEN_NAME_PLAYER];

//---synch elements---
HANDLE file_mutex;
HANDLE fileHandle = NULL;
HANDLE player1Event;
HANDLE player2Event;

//---threads declarations--- 

static DWORD ServiceThread(int*me);
static DWORD InputExitThread(SOCKET* t_socket);
// threads function 
int WaitForUser(HANDLE player_event);
void CleanupWorkersThreads();
int FindFirstUnusedThreadSlot();
int InitSyncElements();
int PlayRound(message* lp_message, int* round, int myIndex, char* My_Secret, char* Other_Secret, SOCKET* t_socket);
int ManageMessageReceived(message* lp_message, int myIndex, SOCKET* t_socket, bool* OUT IOpenTheFile);
int player1_read_write_file(char* input_line, char* header_message, char* OUT output_line);
int player2_read_write_file(char* input_line, char* OUT output_line);

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
	/// ------- create synch elements ------
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

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		ThreadHandles[Ind] = NULL;
		memset(usernames[Ind], 0, MAX_LEN_NAME_PLAYER);
	}

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
			ThreadHandles[Ind] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ServiceThread, (LPWORD*)Ind, 0, NULL);
			if (ThreadHandles[Ind] == NULL)
			{
				printf("failed create thread %d", GetLastError());
				goto server_cleanup_3;
			}
		}
	}
server_cleanup_3:
	//clean up all threads
	CleanupWorkersThreads();
server_cleanup_2:
	if (MainSocket != 0)
	{
		if (closesocket(MainSocket) == SOCKET_ERROR)
			printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());
	}
server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
server_cleanup_0:
	CloseHandle(file_mutex);
	CloseHandle(player1Event);
	CloseHandle(player2Event);
}

//Service thread is the thread that opens for each successful client connection and "talks" to the client.

static DWORD ServiceThread(int* me)
{
	TransferResult_t RecvRes;
	int MyIndex = *me;
	SOCKET* t_socket = &(Sockets[MyIndex]);
	int otherIndex = 0;
	if (otherIndex == MyIndex)
		otherIndex = 1;

	BOOL Done = FALSE;
	bool IOpenTheFile = false;
	//for messages

	int ret_val = 0;
	char* AcceptedStr = NULL;
	message* lp_message = NULL;
	//game state machine

	while (!Done)
	{
		RecvRes = ReceiveString(&AcceptedStr, *t_socket);
		if (RecvRes == TRNS_FAILED)
		{
			ret_val = RecvRes;
			printf("error while reading, closing this thread .\n");
			Done = true;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			ret_val = RecvRes;
			printf("Connection closed while reading, closing thread.\n");
			Done = true;
		}
		else
		{
			printf("Got string:%s\n", AcceptedStr);
			lp_message = process_Message(AcceptedStr, IsServer);
			ret_val = ManageMessageReceived(lp_message, MyIndex, t_socket, &IOpenTheFile);//to do cheak return value and manage it 
			if (ret_val != SUCCESS)
				Done = true;
		}

		if (AcceptedStr != NULL)
		{
			free(AcceptedStr);
			AcceptedStr = NULL;
		}
		if (lp_message != NULL)
		{
			free(lp_message);
			lp_message = NULL;
		}

	}

	printf("Conversation ended.\n");
	printf("Waiting for a client to connect...\n");
	if (!ServerInitateShutDown)
	{
		int other = (MyIndex + 1) % 2;
		SendString("SERVER_QUIT_OPPONENT", Sockets[other]);
		SendString("SERVER_MAIN_MENU", Sockets[other]);
		shutdown(Sockets[MyIndex], SD_SEND);
	}
	closesocket(Sockets[MyIndex]);
	return ret_val;
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
			ServerInitateShutDown = true;
			closesocket(*t_socket);
			*t_socket = 0;
			break;
		}
	}
	return SUCCESS;
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

void CleanupWorkersThreads()
{
	int Ind = 0;
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			shutdown(Sockets[Ind], SD_SEND);
			TerminateThreadGracefully(&ThreadHandles[Ind]);
		}
	}
	TerminateThreadGracefully(&ExitThreadHandle);
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
	ret_val = CreateMutexWrap(FALSE, &file_mutex);//todo-cheak for failure 		
	if (ret_val != SUCCESS)
	{
		goto clean2;
	}
	return SUCCESS;
clean2:
	CloseHandle(player2Event);
clean1:
	CloseHandle(player1Event);
clean0:
	return ret_val;
}



int ManageMessageReceived(message* lp_message, int myIndex, SOCKET* t_socket, bool* OUT IOpenTheFile)
{
	char SendStr[MAX_LEN_MESSAGE];
	int ret_val1 = 0;
	int ret_val2 = 0;
	int other = (myIndex + 1) % 2;
	static char My_Secret[MAX_LEN_MESSAGE] = { 0 };
	static char Other_Secret[MAX_LEN_MESSAGE] = { 0 };
	static int round = 1;
	if (lp_message->ClientType == CLIENT_REQUEST)
	{

		strcpy(usernames[myIndex], (const char*)lp_message->message_arguments[0]);
		ret_val1 = SendString("SERVER_APPROVED", *t_socket);
		ret_val2 = SendString("SERVER_MAIN_MENU", *t_socket);
		if (ret_val1 == TRNS_FAILED || ret_val2 == TRNS_FAILED)
		{
			ret_val1 = TRNS_FAILED;
			printf("error while sending, closing thread.\n");
			return ret_val1;
		}
		return SUCCESS;
	}
	else if (lp_message->ClientType == CLIENT_VERSUS)
	{
		//first player create the file 
		ret_val1 = WaitForSingleObjectWrap(file_mutex, TIME_OUT_THREADS);
		if (ret_val1 != SUCCESS)
			return ret_val1;
		//open file
		if (!PathFileExistsA(FILEPATH))
		{
			ret_val1 = OpenFileWrap(FILEPATH, CREATE_ALWAYS, &fileHandle);
			if (ret_val1 != SUCCESS)
				return ret_val1;
			*IOpenTheFile = true;
		}
		ret_val1 = ReleaseMutexeWrap(file_mutex);
		if (ret_val1 != SUCCESS)
			return ret_val1;

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
				printf("error while sending, closing thread.\n");
				return  ret_val1;
			}
			return SUCCESS;
		}
		//2 players in the game
		sprintf(SendStr, "SERVER_INVITE:%s", usernames[myIndex]);
		ret_val1 = SendString(SendStr, *t_socket);
		if (ret_val1 == TRNS_FAILED)
		{
			printf("error while sending, closing thread.\n");
			return  ret_val1;
		}
		sprintf(SendStr, "SERVER_SETUP_REQUSET");
		ret_val1 = SendString(SendStr, *t_socket);
		if (ret_val1 == TRNS_FAILED)
		{
			printf(" error while sending, closing thread.\n");
			return  ret_val1;
		}
		return SUCCESS;
	}
	else if (lp_message->ClientType == CLIENT_DISCONNECT)
	{
		return CLIENT_DISCONNECT;
	}
	else if (lp_message->ClientType == CLIENT_SETUP)
	{
		sprintf(My_Secret, "%s", lp_message->message_arguments[0]);
		if (myIndex == SecondThread)
		{
			ret_val1 = player2_read_write_file(My_Secret, Other_Secret);
			if (ret_val1 != SUCCESS)
				return ret_val1;
		}
		if (myIndex == FirstThread)
		{
			ret_val1 = player1_read_write_file(My_Secret, Other_Secret, "----Secrets----");
			if (ret_val1 != SUCCESS)
				return ret_val1;
		}

		sprintf(SendStr, "SERVER_SETUP_REQUSET");
		ret_val1 = SendString(SendStr, *t_socket);
		if (ret_val1 == TRNS_FAILED)
		{
			printf(" error while sending, closing thread.\n");
			return  ret_val1;
		}
	}
	else if (lp_message->ClientType == CLIENT_PLAYER_MOVE)
	{
		ret_val1=PlayRound(lp_message, &round, myIndex, My_Secret, Other_Secret, t_socket);
		if (ret_val1 != SUCCESS)
			return ret_val1;
	}
	return SUCCESS;
}

	int WaitForUser(HANDLE player_event) {
		int res = 0;
		while (true)
		{
			res = WaitForSingleObject(player_event, TIME_OUT_THREAD / 2);
			if (res == WAIT_OBJECT_0)
			{
				return SUCCESS;
			}
			else if (ServerInitateShutDown == true)
			{
				return exitUser;
			}
		}
	}
	int player1_read_write_file(char* input_line, char* header_message, char* OUT output_line)
	{
		int ret_val1;

		//lock mutex
		ret_val1 = WaitForSingleObjectWrap(file_mutex, TIME_OUT_THREAD);
		if (ret_val1 != SUCCESS)
			return ret_val1;

		//write line from input array to file  
		ret_val1 = WriteLineString(header_message, input_line);
		if (ret_val1 != SUCCESS)
			return ret_val1;
		ret_val1 = WriteLineString(fileHandle, input_line);
		if (ret_val1 != SUCCESS)
			return ret_val1;
		//realse mutex 
		ret_val1 = ReleaseMutexeWrap(file_mutex);
		if (ret_val1 != SUCCESS)
			return ret_val1;

		//set even player 1- let player 2 get out of blocking 
		SetEvent(player1Event);

		//wait for player 2 
		ret_val1 = WaitForSingleObjectWrap(player2Event, TIME_OUT_THREAD);
		if (ret_val1 != SUCCESS)
			return ret_val1;

		//lock file mutex
		ret_val1 = WaitForSingleObjectWrap(file_mutex, TIME_OUT_THREAD);
		if (ret_val1 != SUCCESS)
			return ret_val1;

		//read line from file to output array 
		ret_val1 = ReadLine(fileHandle, output_line);
		if (ret_val1 != SUCCESS)
			return ret_val1;

		//realse mutex 
		ret_val1 = ReleaseMutexeWrap(file_mutex);
		if (ret_val1 != SUCCESS)
			return ret_val1;
		return SUCCESS;

	}
	int player2_read_write_file(char* input_line, char* OUT output_line)
	{
		int ret_val1;


		//wait for player 1
		ret_val1 = WaitForSingleObjectWrap(player1Event, TIME_OUT_THREAD);
		if (ret_val1 != SUCCESS)
			return ret_val1;
		//lock mutex
		ret_val1 = WaitForSingleObjectWrap(file_mutex, TIME_OUT_THREAD);
		if (ret_val1 != SUCCESS)
			return ret_val1;

		//read line from filr to output array 
		ret_val1 = ReadLine(fileHandle, output_line);
		if (ret_val1 != SUCCESS)
			return ret_val1;

		//write line from input array to file  
		ret_val1 = WriteLineString(fileHandle, input_line);
		if (ret_val1 != SUCCESS)
			return ret_val1;

		//realse mutex 
		ret_val1 = ReleaseMutexeWrap(file_mutex);
		if (ret_val1 != SUCCESS)
			return ret_val1;

		//set even player 1- let player 2 get out of blocking 
		SetEvent(player2Event);
		return SUCCESS;
	}
	int PlayRound(message * lp_message, int* round, int myIndex, char* My_Secret, char* Other_Secret, SOCKET * t_socket)
	{

		char Guess[MAX_LEN_MESSAGE] = { 0 };
		char OtherGuess[MAX_LEN_MESSAGE] = { 0 };
		char header[MAX_LEN_OF_PARAM] = { 0 };
		char SendStr[MAX_LEN_MESSAGE] = { 0 };
		int bulls = 0;
		int cows = 0;
		int statusPlayer1 = 0;
		int statusPlayer2 = 0;
		int other = (myIndex + 1) % 2;
		int ret_val1 = 0;
		sprintf(Guess, "%s", lp_message->message_arguments[0]);
		sprintf(header, "----Round%d----", *round++);
		if (myIndex == SecondThread)
		{
			ret_val1 = player2_read_write_file(Guess, OtherGuess);
			if (ret_val1 != SUCCESS)
				return ret_val1;
		}
		if (myIndex == FirstThread)
		{
			ret_val1 = player1_read_write_file(Guess, OtherGuess, header);
			if (ret_val1 != SUCCESS)
				return ret_val1;
		}
		//status of first player 
		statusPlayer1 = PlayRoundPlayer(My_Secret, Guess, &bulls, &cows, true);
		//status of second  player 
		statusPlayer2 = PlayRoundPlayer(Other_Secret, Other_Secret, NULL, NULL, false);
		//send game results 
		sprintf(SendStr, "SERVER_GAME_RESULTS:%d;%d;%s;%s", bulls, cows, usernames[other], OtherGuess);
		ret_val1 = SendString(SendStr, *t_socket);
		if (ret_val1 == TRNS_FAILED)
			goto clean_game0;
		//player1 win 
		if (statusPlayer1 == true && statusPlayer2 == false)
		{
			sprintf(SendStr, "SERVER_WIN:%s;%s", usernames[myIndex], OtherGuess);
			ret_val1 = SendString(SendStr, *t_socket);
			if (ret_val1 == TRNS_FAILED)
				goto clean_game0;
		}
		//player2 win 
		else if (statusPlayer1 == false && statusPlayer2 == true)
		{
			sprintf(SendStr, "SERVER_WIN:%s;%s", usernames[other], OtherGuess);
			ret_val1 = SendString(SendStr, *t_socket);
			if (ret_val1 == TRNS_FAILED)
				goto clean_game0;
		}
		//draw 
		else if (statusPlayer1 == true && statusPlayer2 == true) {
			sprintf(SendStr, "SERVER_DRAW");
			SendString(SendStr, *t_socket);
			if (ret_val1 == TRNS_FAILED)
				goto clean_game0;
		}

		return SUCCESS;

	clean_game0:
		printf(" error while sending, closing thread.\n");
		return  ret_val1;
	}




	/*HANDLE new_players_mutex;
	HANDLE fileHandle = NULL;
	HANDLE player1Event;
	HANDLE player2Event;

	*/