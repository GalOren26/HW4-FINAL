/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
 This file was written for instruction purposes for the
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
 /*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#include "Server.h"


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define NUM_OF_WORKER_THREADS 2

#define MAX_LOOPS 3

#define SEND_STR_SIZE 35

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET Sockets[NUM_OF_WORKER_THREADS];

HANDLE file_mutex;
HANDLE new_players_mutex;

HANDLE fileHandle;
//int OpenFileRes;

HANDLE writterEvent;
HANDLE readerEvent;

char* usernames[NUM_OF_WORKER_THREADS];

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread(SOCKET* t_socket);

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/


void MainServer(int port )
{
	int res = OpenFileWrap(FILEPATH, OPEN_EXISTING, &fileHandle);	//TODO -- is file
	if (res == SUCCESS) {
		CloseHandleWrap(fileHandle);
		DeleteFileA(FILEPATH);
	}
	create_event_simple(&writterEvent);
	create_event_simple(&readerEvent);
	int Ind;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	for (int i = 0; i < NUM_OF_WORKER_THREADS; ++i)
		usernames[i] = NULL;

	CreateSemphoreWrap(1, &file_mutex, 1);//todo-cheak for failure 
	CreateSemphoreWrap(1, &new_players_mutex, 1);//todo-cheak for failure 
	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}

	/* The WinSock DLL is acceptable. Proceed. */

	// Create a socket.    
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		goto server_cleanup_1;
	}

	// Bind the socket.
	/*
		For a server to accept client connections, it must be bound to a network address within the system.
		The following code demonstrates how to bind a socket that has already been created to an IP address
		and port.
		Client applications use the IP address and port to connect to the host network.
		The sockaddr structure holds information regarding the address family, IP address, and port number.
		sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
   */
   // Create a sockaddr_in object and set its values.
   // Declare variables

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup_2;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(SERVER_PORT); //The htons function converts a u_short from host to TCP/IP network byte order 
									   //( which is big-endian ).
	/*
		The three lines following the declaration of sockaddr_in service are used to set up
		the sockaddr structure:
		AF_INET is the Internet address family.
		"127.0.0.1" is the local IP address to which the socket will be bound.
		2345 is the port number to which the socket will be bound.
	*/

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;

	printf("Waiting for a client to connect...\n");

	for (Loop = 0; Loop < MAX_LOOPS; Loop++)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}

		printf("Client Connected.\n");

		Ind = FindFirstUnusedThreadSlot();

		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			printf("No slots available for client, dropping the connection.\n");
			//TODO - send ERROR to client because ther is no place
			
		}
		else
		{
			Sockets[Ind] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // ThreadInputs[Ind] when the
											  // time comes.
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				Ind,
				0,
				NULL
			);

		}
	} // for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )

server_cleanup_3:
	//clean up all threads and sockets.
	CleanupWorkerThreads();


server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot()
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

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static void CleanupWorkerThreads()
{

	WaitForMultipleObjectsWrap(2, ThreadHandles, INFINITE, FALSE);
	//
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], INFINITE);

			if (Res == WAIT_OBJECT_0)
			{
				closesocket(Sockets[Ind]);//to do delte this therad handle it 
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
				printf("Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
static DWORD ServiceThread(int me)
{
	SOCKET* t_socket = &(Sockets[me]);
	char SendStr[SEND_STR_SIZE];
	int other = 0;
	if (other == me)
		other = 1;

	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	strcpy(SendStr, "Welcome to this server!");

	SendRes = SendString("Welcome to this server!", *t_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return 1;
	}

	
	char* user_name = NULL;
	int OpenFileRes;
	//game state machine
	while (!Done)
	{
		char* AcceptedStr = NULL;
		message* lp_message = NULL;
		RecvRes = ReceiveString(&AcceptedStr, *t_socket);

		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			//TODO - close socket -> shutdown
			closesocket(*t_socket);
			return 1;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			//TODO - close socket -> shutdown
			closesocket(*t_socket);
			return 1;
		}
		else
		{
			printf("Got string : %s\n", AcceptedStr);
			lp_message = NULL;
			lp_message = process_Message(AcceptedStr, 1);
			//TransferResult_t SendRes;
			//SendRes = SendString(AcceptedStr, *t_socket);
			//TODO - check sending result	
		}

		//TODO - exit

		if (STRINGS_ARE_EQUAL(lp_message->message_type, "CLIENT_REQUEST"))
		{
			if (usernames[me] == NULL)
			{
				//TODO - maybe do a function for parsing arguments
				//and check sending result
				usernames[me] = lp_message->message_arguments;
				SendString("SERVER_APPROVED", *t_socket);
				SendString("SERVER_MAIN_MENU", *t_socket);
			}
		}
		else if (STRINGS_ARE_EQUAL(AcceptedStr, "CLIENT_VERSUS"))
		{
			//check if new opponent is connected.
			WaitForSingleObjectWrap(file_mutex, INFINITE);//change to time out 
			//open file
			OpenFileRes = OpenFileWrap(FILEPATH, OPEN_EXISTING, &fileHandle );//TODO hardcoded
			if (OpenFileRes != SUCCESS) {
				OpenFileWrap(FILEPATH, CREATE_ALWAYS, &fileHandle);//TODO hardcoded
			}
			ReleaseSemphoreWrap(file_mutex, 1);

			if (OpenFileRes == SUCCESS) {
				SetEvent(writterEvent);
				int res = WaitForSingleObjectWrap(readerEvent, 15 * 1000);
				if (res != SUCCESS) {
					ResetEvent(writterEvent);	
					.	`-+

					SendString("SERVER_NO_OPPONENTS", *t_socket);
					SendString("SERVER_MAIN_MENU", *t_socket);
					continue;
				}
			}
			else {
				SetEvent(readerEvent);
				int res = WaitForSingleObjectWrap(writterEvent, 15 * 1000);
				if (res != SUCCESS) {
					ResetEvent(readerEvent);
					CloseHandleWrap(fileHandle);
					DeleteFileA(FILEPATH);
					SendString("SERVER_NO_OPPONENTS", *t_socket);
					SendString("SERVER_MAIN_MENU", *t_socket);
					continue;
					//TODO - check returned value
				}
			}
			//2 players in the game
			char* invite = (char*)calloc(strlen("SERVER_INVITE:") + 1 + strlen(usernames[other]), sizeof(char));
			//TODO - check allocation
			strcat(invite, "SERVER_INVITE:");
			strcat(invite + strlen("SERVER_INVITE:"), usernames[other]);
			SendString(invite, *t_socket);

		}
		else if (STRINGS_ARE_EQUAL(AcceptedStr, "bye"))
		{
			strcpy(SendStr, "see ya!");
			Done = TRUE;
		}
		else
		{
			strcpy(SendStr, "I don't understand");
		}
		//free(AcceptedStr); //will delete the lp_message data also.

	}

	printf("Conversation ended.\n");
	closesocket(*t_socket);
	return 0;
}
