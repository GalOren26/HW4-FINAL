#include "Client.h"
#include "bullsAndCows.h"
//#include <winsock.h>
//---resources---



SOCKET m_socket = 0;
SOCKADDR_IN clientService;
//HANDLE Send_event;
HANDLE hThread[NumOfClientThreads] = { NULL,NULL };
state my_state = GetName;
bool server_disconecnt = false;
HANDLE client_decide_what_to_do;
bool client_decice_exit = false;
bool server_denied = false;
int my_port;
char my_ip[SIZE_OF_IP] = { 0 };
char my_user_name[MAX_LEN_NAME_PLAYER] = { 0 };
//---synch elements---
struct timeval waitTime = { TIME_OUT_THREADS,0 };

static DWORD RecvDataThread(void);
static DWORD SendDataThread(void);
struct timeval waitTime = { 15000,0 };
//---threads declarations--- 


int MainClient(int port, char* ip, char* username)
{
	//int port = atoi(argv[1]);
	//int ret_val = create_event_simple(&Send_event);
	/*if (ret_val != SUCCESS)
		return ret_val;*/
	strcpy(my_user_name, username);
	strcpy(my_ip, ip);
	my_port = port;

	int ret_val = 0;
	WSADATA wsaData; //Create a WSADATA object called wsaData.
	ret_val = create_event_simple(&client_decide_what_to_do);
	if (ret_val != SUCCESS)
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
	clientService.sin_addr.s_addr = inet_addr(ip); //Setting the IP address to connect to
	clientService.sin_port = htons(port); //Setting the port to connect to.

	ret_val = ConnectToServerWithUI(&m_socket, (SOCKADDR*)(&clientService), sizeof(clientService), port, ip);
	if (ret_val != SUCCESS)
		goto clean2;



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


//input Aarguments: NONE
//output agruments : DWORD that indicates if the function "recieve buffer" succeeded or not
//funcionality: this function is operated by ta thread that recivec and send data to the server 
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
			if (client_decice_exit)
			{
				goto clean1;
			}
			else {
				server_disconecnt = true;
				if (server_denied)
					showMenu(DENIED, my_port, my_ip);
				else
					showMenu(FAILURE, my_port, my_ip);
				shutdown(m_socket, SD_SEND);
				closesocket(m_socket);
				m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
	if (server_disconecnt)
		shutdown(m_socket, SD_SEND);
	closesocket(m_socket);
	return RecvRes;
}


//input Aarguments: NONE
//output agruments : DWORD that indicates if the function succeeded or not
//funcionality: this function is operated by ta thread that recivec and send data from the user 
static DWORD SendDataThread(void)
{
	//to -do free message
	int ret_val1;
	char SendStr[MAX_LEN_MESSAGE] = { 0 };
	char inputstr[MAX_LEN_MESSAGE] = { 0 };
	int valid;
	bool first_iteartion = true;
	TransferResult_t send_result;
<<<<<<< HEAD

	/*SendString("CLIENT_REQUEST:gal", m_socket);*/

	//SendString("CLIENT_VERSUS", m_socket);  
	while (1)
	{
		//if (STRINGS_ARE_EQUAL(SendStr, "2"));
		//my_state = WANTTODISCONNECT;
		if (!first_iteartion)
			gets_s(inputstr, sizeof(inputstr)); //Reading a string from the keyboard
		else
			first_iteartion = false;
	
=======
<<<<<<< HEAD
	char* user_name = NULL;
	 
=======
	char user_name[MAX_LEN_MESSAGE] = { 0 };
	/*SendString("CLIENT_REQUEST:gal", m_socket);*/

	//SendString("CLIENT_VERSUS", m_socket);  
>>>>>>> ea19d67508b5dd8b1ed85947fe2699a91e825bae
	printf("Insert username:\n");

	while (1)
	{
		gets_s(inputstr, sizeof(inputstr)); //Reading a string from the keyboard
>>>>>>> 7cbcf022a4a06fb408a9a2162929a1bb6b9cbdc3
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
					if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR)
					{
						showMenu(FAILURE, my_port, my_ip);
						continue;
					}
					server_disconecnt = false;
					client_decice_exit = false;
					SetEvent(client_decide_what_to_do);
					my_state = GetName;
				}
<<<<<<< HEAD
				my_state = WANTTOPLAY;
=======
				else
				{
					my_state = WANTTOPLAY;
				}
			}
			else {
				printf(" invalid input please type 1 or 2 ! ;)\n");
>>>>>>> ea19d67508b5dd8b1ed85947fe2699a91e825bae
			}
		}
		if (my_state == GetName)
		{
<<<<<<< HEAD
			sprintf(SendStr, "%s:%s", "CLIENT_REQUEST", my_user_name);
			send_result = SendString(SendStr, m_socket);//to do-cheack return value
			waitTime.tv_sec =  TIME_OUT_THREADS;
			setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval));
=======
<<<<<<< HEAD
			sprintf(SendStr, "%s:%s", "CLIENT_REQUEST", inputstr);
			send_result = SendString(SendStr, m_socket);//to do-cheack return value	
			if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval)) < 0) {
				showMenu(MAIN, SERVER_PORT, SERVER_ADDRESS_STR);
			}
=======
			//	user_name = getUserName();
			if (user_name[0] == 0)
				strcpy(user_name, inputstr);
			sprintf(SendStr, "%s:%s", "CLIENT_REQUEST", user_name);
			send_result = SendString(SendStr, m_socket);//to do-cheack return value
>>>>>>> ea19d67508b5dd8b1ed85947fe2699a91e825bae
>>>>>>> 7cbcf022a4a06fb408a9a2162929a1bb6b9cbdc3
		}
		else if (my_state == WANTTOPLAY) {
			sprintf(SendStr, "%s", "CLIENT_VERSUS");
			send_result = SendString(SendStr, m_socket);//to do-cheack return value
<<<<<<< HEAD
			waitTime.tv_sec = 2*TIME_OUT_THREADS;
			setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval));
=======
			waitTime.tv_sec = 30000;
			if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval)) < 0) {
				showMenu(MAIN, SERVER_PORT, SERVER_ADDRESS_STR);
			}
>>>>>>> 7cbcf022a4a06fb408a9a2162929a1bb6b9cbdc3
		}
		else if (my_state == GetSecret)
		{
			valid = isValid(&inputstr[0]);
			if (valid) {
				sprintf(SendStr, "%s:%s", "CLIENT_SETUP", inputstr);
				send_result = SendString(SendStr, m_socket);//to do-cheack return value
<<<<<<< HEAD
				waitTime.tv_sec = 0;//for blocking wait for user
				setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval));
=======
				waitTime.tv_sec = 0;
				if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval)) < 0) {
					showMenu(MAIN, SERVER_PORT, SERVER_ADDRESS_STR);
				}
>>>>>>> 7cbcf022a4a06fb408a9a2162929a1bb6b9cbdc3
			}
			continue;
		}
		else if (my_state == GetGuess)
		{
			valid = isValid(&inputstr[0]);
			if (valid) {
				sprintf(SendStr, "%s:%s", "CLIENT_PLAYER_MOVE", inputstr);
				send_result = SendString(SendStr, m_socket);//to do-cheack return value
<<<<<<< HEAD
				waitTime.tv_sec = 0;//for blocking wait for user
				setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval));
=======
				waitTime.tv_sec = 0;
				if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval)) < 0) {
					showMenu(MAIN, SERVER_PORT, SERVER_ADDRESS_STR);
				}
>>>>>>> 7cbcf022a4a06fb408a9a2162929a1bb6b9cbdc3
			}
			continue;
		}
		else if (my_state == WANTTODISCONNECT) {
			sprintf(SendStr, "%s", "CLIENT_DISCONNECTED");
			send_result = SendString(SendStr, m_socket);//to do-cheack return value
			waitTime.tv_sec = 15000;
			if (setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*) & waitTime, sizeof(struct timeval)) < 0) {
				showMenu(MAIN, SERVER_PORT, SERVER_ADDRESS_STR);
			}
			shutdown(m_socket, SD_SEND);
			waitTime.tv_sec = TIME_OUT_THREADS;
			setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval));
		}
	

	}
	return 1;
}
//input Aarguments: msg- struct of message , sender - a socket
//output agruments : None
//funcionality: this function operates by the message type that was sent by the server according to the instructions
void exec_protocol(message* msg, SOCKET sender) {
	int exit = 0;
	int i = 0;
<<<<<<< HEAD
	//char type = (char)(msg->ServerType);
	switch (msg->ServerType) {
	case SERVER_MAIN_MENU:
		showMenu(MAIN, my_port, my_ip);
		waitTime.tv_sec = 0;
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval));
=======
	int portNumber = SERVER_PORT;
	char ip = SERVER_ADDRESS_STR;
	switch (msg->ServerType) {
	case SERVER_MAIN_MENU:
		showMenu(MAIN, SERVER_PORT, SERVER_ADDRESS_STR);
		waitTime.tv_sec = 0;
>>>>>>> 7cbcf022a4a06fb408a9a2162929a1bb6b9cbdc3
		my_state = MENU;
		break;
	case SERVER_APPROVED:
<<<<<<< HEAD
		printf("Connected to server on %s : %d\nWelcome to server :)\n\n", my_ip, my_port);
=======
<<<<<<< HEAD
		printf("welcome to the server\n\n");
		break;
	case SERVER_DENIED:
		//disconnect
		server_disconecnt = true;
		waitTime.tv_sec = 0;
		showMenu(FAILURE, SERVER_PORT, SERVER_ADDRESS_STR);
		my_state = MENU;
=======
		printf("Connected to server on %s : %d\nWelcome to server :)\n\n", SERVER_ADDRESS_STR, SERVER_PORT);
>>>>>>> 7cbcf022a4a06fb408a9a2162929a1bb6b9cbdc3
		//my_state = WANTTOPLAY;
		break;
	case SERVER_DENIED:
		//disconnect
		printf("%s\n", msg->message_arguments[0]);
		server_denied = true;
<<<<<<< HEAD
		waitTime.tv_sec = 0;
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval));
=======
>>>>>>> ea19d67508b5dd8b1ed85947fe2699a91e825bae
>>>>>>> 7cbcf022a4a06fb408a9a2162929a1bb6b9cbdc3
		break;
	case SERVER_INVITE:
		printf("Game is on!\n\n");
		break;
	case SERVER_SETUP_REQUSET:
		printf("Please choose a 4-digits number, with no duplicates!\n\n");
		my_state = GetSecret;
		waitTime.tv_sec = 0;//for blocking wait for user
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval));
		break;
	case SERVER_PLAYER_MOVE_REQUEST:
		printf("Please choose your guess!\n");
		my_state = GetGuess;
<<<<<<< HEAD
		waitTime.tv_sec = 0;//for blocking wait for user
		setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&waitTime, sizeof(struct timeval));
		//number = chooseNumber();
		//SendString(number, sender);
=======
>>>>>>> 7cbcf022a4a06fb408a9a2162929a1bb6b9cbdc3
		break;
	case SERVER_GAME_RESULTS:
		printf("you get:\nbulls:%s\ncows:%s\n\n%s played %s.\n\n", msg->message_arguments[0], msg->message_arguments[1], msg->message_arguments[2], msg->message_arguments[3]);
		break;
	case SERVER_WIN:
		printf("%s won!\nopponents number was %s\n\n", msg->message_arguments[0], msg->message_arguments[1]);
		break;
	case SERVER_DRAW:
		printf("it's a tie:/ \n");
		break;
	case SERVER_NO_OPPONENTS:
		printf("you got no other players to play with ,please try again later\n\n");
		break;
	case SERVER_OPPONENT_QUIT:
		printf("the other player disconnected.\n\n");
		break;
	default:
		printf("Undifiend message type recieved\n");
		break;
	}
}

//input Aarguments: my_socket-a socket, my_clientservice is a sockaddr struct, sizeof(clientService), portNumber and ip 
//output agruments : int indicates if the function succeeded or not
//funcionality: this function creates the connection to the server and validates the input argument(1 or 2) from the client
int ConnectToServerWithUI(SOCKET* my_socket, SOCKADDR* my_clientService, int SizeOfclientService, int portNumber, char* ip)
{
	char SendStr[MAX_LEN_MESSAGE];//TO DO CHANGE TO CONSR IN MESSAGE 
	while (1)
	{
		if (connect(*my_socket, my_clientService, SizeOfclientService) == SOCKET_ERROR) {
			showMenu(FAILURE, portNumber, ip);
			gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard
			if (STRINGS_ARE_EQUAL(SendStr, "2"))
				return exitUser;
			else if (STRINGS_ARE_EQUAL(SendStr, "1"))
				continue;
			else 
				printf(" invalid input please type 1 or 2 ! ;)\n");
		}
		else
		{
			break;
		}
	}
	return SUCCESS;
}

//TO-DO send create mutex cheach if server disconnect.
//TO-DO wait for 15 sec ! 
//to do add event client aprooved