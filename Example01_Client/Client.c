#include "Client.h"
#include "bullsAndCows.h"
//---resources---
	//I need to use that :
////
	//shutdown(Sockets[MyIndex], SD_SEND);
//}
//closesocket(Sockets[MyIndex]);
////

SOCKET m_socket = 0;
SOCKADDR_IN clientService;
//HANDLE Send_event;
HANDLE hThread[NumOfClientThreads] = { NULL,NULL };
state my_state = GetName;
bool server_disconecnt = false;
HANDLE client_decide_what_to_do;
bool client_decice_exit = false;
//---synch elements---
static DWORD RecvDataThread(void);
static DWORD SendDataThread(void);
//---threads declarations--- 


int MainClient(int argc, char* argv[])
{
	//int port = atoi(argv[1]);
	//int ret_val = create_event_simple(&Send_event);
	/*if (ret_val != SUCCESS)
		return ret_val;*/
	int ret_val = 0;
	// TO-DO WARP WITH CREATE-SOCKET 
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	ret_val=create_event_simple(&client_decide_what_to_do);
		if(ret_val!=SUCCESS)
			goto clean00;
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

	ret_val = ConnectToServerWithUI(&m_socket, (SOCKADDR*)(&clientService), sizeof(clientService), SERVER_PORT, SERVER_ADDRESS_STR);
	if (ret_val != SUCCESS)
		goto clean2;
	printf("Connected to server on %s : %d\n", SERVER_ADDRESS_STR, SERVER_PORT);


	hThread[0] = CreateThreadSimple((LPTHREAD_START_ROUTINE)SendDataThread, NULL, NULL);
	if (hThread[0] == NULL)
	{
		printf("cannot open send Thread\n");
		goto clean3;
	}
	hThread[1] = CreateThreadSimple((LPTHREAD_START_ROUTINE)RecvDataThread, NULL, NULL);
	if (hThread[1] == NULL)
	{
		TerminateThreadGracefully(hThread[0]);
		printf("cannot open recv Thread");
		goto clean3;
	}
	ret_val = WaitForMultipleObjectsWrap(NumOfClientThreads, hThread, INFINITE, FALSE);
	if (ret_val != SUCCESS)
	{
		KillThreads(hThread, NumOfClientThreads);
		goto clean4;
	}
	ret_val = SUCCESS;

	////must remember to free user_name

clean4:	// close handles 
	Close_Threads(NumOfClientThreads, hThread);
clean3://in this case the thared will be close this socket 
	m_socket = 0;
	//Clean 2 means that the initial connection failed, so only the socket needes to be closed
clean2:
	if (m_socket != NULL)
	{
		shutdown(m_socket, SD_BOTH);
		closesocket(m_socket);
	}
clean1:
	WSACleanup();
clean0:
	CloseHandle(client_decide_what_to_do);
clean00:
	return ret_val;

}



//recive data from the server 
static DWORD RecvDataThread(void)
{
	//to -do free message
	TransferResult_t RecvRes;
	message* new_message;
	char* AcceptedStr;
	while (1)
	{
		AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, m_socket);
		if (RecvRes == TRNS_FAILED || RecvRes == TRNS_DISCONNECTED)
		{
			my_state = MENU;
			if(client_decice_exit)
			{
				goto clean1;
			}
			else {
				server_disconecnt = true;
				showMenu(FAILURE, SERVER_PORT, SERVER_ADDRESS_STR);
				WaitForSingleObject(client_decide_what_to_do, INFINITE);
			}
		}
		else
		{
			printf("%s\n", AcceptedStr);
			new_message = process_Message(AcceptedStr, 0);
			if ((int)new_message == -1)
			{
				goto clean1;

			}
			exec_protocol(new_message, m_socket);
			free(AcceptedStr);
			AcceptedStr = NULL;

		}
	}
	//clean 1 free the product of process_message
clean1:
	if (AcceptedStr != NULL)
		free(AcceptedStr);
	// Ifr the client Initate close -then first it do shutdown then wait on recive to get shut down back
	//and after his it 
	//close the socket otherwise- the server inatiate the shutdown so the client reply with shutdown back and
	//then close the socket. 
	if(server_disconecnt)
		shutdown(m_socket, SD_SEND);
	closesocket(m_socket);
	return RecvRes;
}


//Sending data to the server
static DWORD SendDataThread(void)
{
	//to -do free message
	int ret_val1;
	char SendStr[MAX_LEN_MESSAGE] = { 0 };
	char inputstr[MAX_LEN_MESSAGE] = { 0 };
	int valid;
	TransferResult_t send_result;
	char* user_name = NULL;
	/*SendString("CLIENT_REQUEST:gal", m_socket);*/

	//SendString("CLIENT_VERSUS", m_socket);  
	printf("Insert username:\n");

	while (1)
	{
		gets_s(inputstr, sizeof(inputstr)); //Reading a string from the keyboard

		//if (STRINGS_ARE_EQUAL(SendStr, "2"));
		//my_state = WANTTODISCONNECT;
		if (my_state == MENU) {
			if (STRINGS_ARE_EQUAL(inputstr, "2")) {
				my_state = WANTTODISCONNECT;
				if (server_disconecnt)
				{
					SetEvent(client_decide_what_to_do);
					client_decice_exit = true;
					break;
				}
				else
				{
					server_disconecnt = false;
					client_decice_exit = true;
					SetEvent(client_decide_what_to_do);
					shutdown(m_socket, SD_SEND);
					break;
				}
			}
			else if (STRINGS_ARE_EQUAL(inputstr, "1")) {
				if (server_disconecnt)
				{
					if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR);
					{
						showMenu(FAILURE, SERVER_PORT, SERVER_ADDRESS_STR);
						continue;
					}
					server_disconecnt = false;
					client_decice_exit = false;
					SetEvent(client_decide_what_to_do);
					showMenu(MAIN, SERVER_PORT, SERVER_ADDRESS_STR);
					continue;
				}

				my_state = WANTTOPLAY;
			}
		}

		//mutex
		//
		//change state 
		if (my_state == GetName)
		{
			//	user_name = getUserName();
			//I need to remember to free user_name
			sprintf(SendStr, "%s:%s", "CLIENT_REQUEST", inputstr);
			send_result = SendString(SendStr, m_socket);//to do-cheack return value
		}
		else if (my_state == WANTTOPLAY) {
			sprintf(SendStr, "%s", "CLIENT_VERSUS");
			send_result = SendString(SendStr, m_socket);//to do-cheack return value
		}
		else if (my_state == GetSecret)
		{
			valid = isValid(&inputstr[0]);
			if (valid) {
				sprintf(SendStr, "%s:%s", "CLIENT_SETUP", inputstr);
				send_result = SendString(SendStr, m_socket);//to do-cheack return value
			}
			continue;
		}
		else if (my_state == GetGuess)
		{
			valid = isValid(&inputstr[0]);
			if (valid) {
				sprintf(SendStr, "%s:%s", "CLIENT_PLAYER_MOVE", inputstr);
				send_result = SendString(SendStr, m_socket);//to do-cheack return value
			}
			continue;
		}
		else if (my_state == WANTTODISCONNECT) {
			sprintf(SendStr, "%s", "CLIENT_DISCONNECTED");
			send_result = SendString(SendStr, m_socket);//to do-cheack return value
			shutdown(m_socket, SD_SEND);
		}

	}
	return 1;
}


void exec_protocol(message* msg, SOCKET sender) {
	int exit = 0;
	int i = 0;
	int portNumber = SERVER_PORT;
	char ip = SERVER_ADDRESS_STR;
	//char type = (char)(msg->ServerType);
	switch (msg->ServerType) {
	case SERVER_MAIN_MENU:
		showMenu(MAIN, SERVER_PORT, SERVER_ADDRESS_STR);
		my_state = MENU;
		//my_state = WANTTOPLAY;
		break;
	case SERVER_APPROVED:
		printf("welcome to the server\n\n");
		//my_state = WANTTOPLAY;
		break;
	case SERVER_DENIED:
		//disconnect

		server_disconecnt = true;
		showMenu(FAILURE, SERVER_PORT, SERVER_ADDRESS_STR);
		my_state = MENU;
		break;
	case SERVER_INVITE:
		printf("Game is on!\n\n");
		break;
	case SERVER_SETUP_REQUSET:
		printf("Please choose a 4-digits number, with no duplicates!\n\n");
		my_state = GetSecret;
		break;
	case SERVER_PLAYER_MOVE_REQUEST:
		printf("Please choose your guess!\n");
		my_state = GetGuess;
		//number = chooseNumber();
		//SendString(number, sender);
		break;
	case SERVER_GAME_RESULTS:
		printf("you had \n bulls: %s \n cows: %s \n %s played %s\n\n", msg->message_arguments[0], msg->message_arguments[1], msg->message_arguments[2], msg->message_arguments[3]);
		break;
	case SERVER_WIN:
		printf("%s won!\n His sequence was %s\n\n", msg->message_arguments[0], msg->message_arguments[1]);
		my_state = MENU;
		break;
	case SERVER_DRAW:
		printf("it's a tie:/ \n");
		break;
	case SERVER_NO_OPPONENTS:
		printf("you got no other players to play with ,please try again later\n\n");
		my_state = MENU;
		break;
	case SERVER_OPPONENT_QUIT:
		printf("the other player disconnected.\n\n");
		my_state = MENU;
		break;
	default:
		printf("Undifiend message type recieved\n");
		break;
	}
}


//connect in loop to server 
int ConnectToServerWithUI(SOCKET* my_socket, SOCKADDR* my_clientService, int SizeOfclientService, int portNumber, char* ip)
{
	char SendStr[MAX_LEN_MESSAGE];//TO DO CHANGE TO CONSR IN MESSAGE 
	while (1)
	{
		if (connect(*my_socket, my_clientService, SizeOfclientService) == SOCKET_ERROR) {
			showMenu(FAILURE, portNumber, ip);
			gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard
			if (STRINGS_ARE_EQUAL(SendStr, "2"))
			{
				return exitUser;
			}
			else if (STRINGS_ARE_EQUAL(SendStr, "1"))
			{
				continue;
			}
			//CloseSocketGracefullySender();
			else {
				printf(" invalid input please type 1 or 2 ! ;)\n");
			}
		}
		else
		{
			break;
		}
	}
	return SUCCESS;
}


//void CleanupWorkersThreadsSocketsClient()
//{
//	int Ind = 0;
//	if (m_socket != 0)
//	{
//		CloseSocketGracefullyReciver(m_socket);
//		m_socket = 0;
//	}
//	for (Ind = 0; Ind < NumOfClientThreads; Ind++)
//	{
//		if (hThread[Ind] != NULL)
//		{
//			// poll to check if thread finished running:
//			DWORD Res = WaitForSingleObject(hThread[Ind], TIME_OUT_THREADS);
//
//			if (Res != WAIT_OBJECT_0)
//			{
//				TerminateThread(ThreadHandles[Ind], -1);
//			}
//			CloseHandle(ThreadHandles[Ind]);
//			ThreadHandles[Ind] = NULL;
//		}
//	}
//}


//TO-DO send create mutex cheach if server disconnect.
//TO-DO wait for 15 sec ! 
//to do add event client aprooved