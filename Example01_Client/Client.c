/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
 This file was written for instruction purposes for the
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering.
Last updated by Amnon Drory, Winter 2011.
 */
 /*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/


#include "Client.h"
#include "game.h"

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
//HANDLE Send_event;
SOCKET m_socket;
SOCKADDR_IN clientService;

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Reading data coming from the server
int ConnectToServerWithUI(SOCKET* my_socket, SOCKADDR* my_clientService, int SizeOfclientService)
{
	char SendStr[256];
	while (1)
	{
		if (connect(*my_socket,my_clientService, SizeOfclientService) == SOCKET_ERROR) {
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


//This thread is waiting on recieve
// I can assume that I get correct data

typedef enum { menu, data }sel;
sel state;
static DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;
	char* AcceptedStr;
	/// time is the wait time for the server. 30000(miliseconds) is arbitrary
	int time = 30000;
	message* new_message = NULL;
	while (1)
	{
		AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, m_socket,time);
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
			new_message = process_Message(AcceptedStr, 0);
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
///דואג לאינפוט מהמשתמש
//-בהתאם לאינפוט הוא עושה מה שצריך לבצע
// "input thread"
//Sending data to the server
static DWORD SendDataThread(void)
{
	char SendStr[256];
	TransferResult_t SendRes;
	int decision, chose;
	int portNumber = SERVER_PORT;
	char ip = SERVER_ADDRESS_STR;
	SendString("CLIENT_REQUEST:gal", m_socket);
	//SendString("CLIENT_VERSUS", m_socket);  
	//printf("Insert username:");
	while (1)
	{
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard
		//message* user_mes = process_Message(SendStr, 0); //TO-DO DEFINE IS SERVER; 
		if (state == menu) {
			chose = playerchoice();
			decision = showMenu(chose, portNumber, ip);
			if (decision == 2)
				SendString("CLIENT_VERSUS", m_socket);

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


int MainClient(int argc ,char *argv [] )
{
	//int port = atoi(argv[1]);

	//int ret_val = create_event_simple(&Send_event);
	/*if (ret_val != SUCCESS)
		return ret_val;*/
	char* client_Messages[] = { "CLIENT_REQUEST","CLIENT_VERSUS","CLIENT_SETUP","CLIENT_PLAYER_MOVE","CLIENT_DISCONNECT" };
	char* server_Messages[] = { "SERVER_MAIN_MENU","SERVER_APPROVED","SERVER_DENIED","SERVER_INVITE","SERVER_SETUP_REQUEST","SERVER_PLAYER_MOVE_REQUEST",
	"SERVER_GAME_RESULTS","SERVER_WIN","SERVER_DRAW","SERVER_NO_OPPONENTS","SERVER_OPPONENT_QUIT" };
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

	ret_val=ConnectToServerWithUI(&m_socket, (SOCKADDR * )(&clientService), sizeof(clientService));
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
clean1:
	WSACleanup();
	return ret_val;
	//return ret_val;
}

char* client_Messages[] = { "CLIENT_REQUEST","CLIENT_VERSUS","CLIENT_SETUP","CLIENT_PLAYER_MOVE","CLIENT_DISCONNECT" };

/*this function executes the command that the server sent*/
int exec_protocol(message* msg, game_results* results, SOCKET sender,int game_status, int game_status) {
	int exit = 0;
	int portNumber = SERVER_PORT;
	char ip = SERVER_ADDRESS_STR;
	char* temp = NULL;
	char type =(char) msg->message_type;
	char* server_Messages[] = { "SERVER_MAIN_MENU","SERVER_APPROVED","SERVER_DENIED","SERVER_INVITE","SERVER_SETUP_REQUEST","SERVER_PLAYER_MOVE_REQUEST",
   "SERVER_GAME_RESULTS","SERVER_WIN","SERVER_DRAW","SERVER_NO_OPPONENTS","SERVER_OPPONENT_QUIT" };

	switch (type) {
		case SERVER_MAIN_MENU:
			showMenu(MAIN, portNumber, ip);
			break;
		case SERVER_APPROVED:
			printf("%s", "SERVER_APPROVED");
			break;
		case SERVER_DENIED:

		case SERVER_INVITE:
			printf("Game is on!\n");
			break;
		case SERVER_SETUP_REQUEST:
		case SERVER_PLAYER_MOVE_REQUEST:
		case SERVER_GAME_RESULTS:

		case SERVER_WIN:
		case SERVER_DRAW:
		case SERVER_NO_OPPONENTS:
		case SERVER_OPPONENT_QUIT:


	}
	switch (msg->message_type) {
	case NEW_USER_ACCEPTED:
		printf("Request to join accepted. number of players connected: %d.\n", atoi(msg->message_arguments));

		break;
	case NEW_USER_DECLINED:
		printf("Request to join was refused\n");
		// close program now
		exit = 1;
		break;
	case GAME_STARTED:
		printf("Game is on!\n");

		break;
	case BOARD_VIEW:// 42 arguments
		update_board(msg->message_arguments);
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		PrintBoard(board, hConsole);
		break;
	case TURN_SWITCH:
		printf("%s's turn\n", msg->message_arguments);
		if (STRINGS_ARE_EQUAL(username, msg->message_arguments))
			SetEvent(turn_event);// my turn
		break;
	case PLAY_ACCEPTED:
		printf("Well played\n");

		break;
	case PLAY_DECLINED:
		printf("Error:%s\n", msg->message_arguments);


		break;
	case GAME_ENDED:
		if (STRINGS_ARE_EQUAL(msg->message_arguments, "Tie"))
			printf("Game ended. Everybody wins!\n");
		else
			printf("Game ended. The winner is %s!\n", msg->message_arguments);
		exit = 1;
		break;
	case RECEIVE_MESSAGE:

		temp = (char*)malloc(sizeof * (msg->message_arguments));
		strcpy(temp, (msg->message_arguments));
		strchr(temp, ';')[0] = ':';
		remove_all_chars(temp, ';');
		printf("%s\n", temp);
		break;


	default:
		printf("Undifiend message type recieved\n");
		break;
	}
	return exit;
}




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