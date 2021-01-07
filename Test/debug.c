//
//#include "stdio.h"
//#include "stdlib.h"
//#include "string.h"
//
//#define _CRT_SECURE_NO_WARNINGS 1 
//#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 
//#define SERVER_MESSAGES_LEN 11
//#define CLIENT_MESSAGES_LEN 5
//#define MAX_NUM_OF_PARAMS 7
//#define MAX_LEN_OF_PARAM 50
//#define MAX_LEM_MESSAGE MAX_LEN_OF_PARAM*(MAX_NUM_OF_PARAMS+1)
//#define IsServer 1 
//#define IsClient 0
//int get_message_code(tmp_message, is_server);
//typedef  enum {
//	SERVER_MAIN_MENU,
//	SERVER_APPROVED,
//	SERVER_DENIED,
//	SERVER_INVITE,
//	SERVER_SETUP_REQUSET,
//	SERVER_PLAYER_MOVE_REQUEST,
//	SERVER_GAME_RESULTS,
//	SERVER_WIN,
//	SERVER_DRAW,
//	SERVER_NO_OPPONENTS,
//	SERVER_OPPONENT_QUIT,
//	end
//}ServerMesType;
//typedef  enum {
//	CLIENT_REQUEST,
//	CLIENT_VERSUS,
//	CLIENT_SETUP,
//	CLIENT_PLAYER_MOVE,
//	CLIENT_DISCONNECT,
//	end1
//}ClientMesType;
//
//typedef struct {
//
//	ClientMesType ClientType;
//	ServerMesType ServerType;
//	char message_arguments[MAX_NUM_OF_PARAMS][MAX_LEN_OF_PARAM];
//	int num_of_arguments;
//
//}message;
//
//
//
//
//
//
//const char* server_messages[] = { "SERVER_MAIN_MENU",
//							  "SERVER_APPROVED",
//							  "SERVER_DENIED",
//							  "SERVER_INVITE",
//							  "SERVER_SETUP_REQUSET",
//							  "SERVER_PLAYER_MOVE_REQUEST",
//							  "SERVER_GAME_RESULTS",
//							  "SERVER_WIN",
//							  "SERVER_DRAW",
//							  "SERVER_NO_OPPONENTS",
//							  "SERVER_OPPONENT_QUIT" }; //extern because client.c uses this too
//
//
//const char* client_messges[] = { "CLIENT_REQUEST",
//							   "CLIENT_VERSUS",
//							   "CLIENT_SETUP",
//							   "CLIENT_PLAYER_MOVE",
//							   "CLIENT_DISCONNECT" };
//message* process_Message(const char* message_text, int is_server);
//int main()
//{
//	message* x = process_Message("CLIENT_VERSUS:gal;ori;shimon", 1);
//	printf("im here ");
//	//char* x ;
//	//x = test();
//	//if (x == -1)
//	//{
//	//	printf("im here ");
//	//}
//}
//
//
//message* process_Message(const char* message_text, int is_server) {
//	char tmp_message[MAX_LEM_MESSAGE];
//	strcpy(tmp_message, message_text);
//	/*int ret_val = SUCCESS;*/
//	is_server = !is_server;
//	const char** MessageArray;
//	if (is_server)
//	{
//		MessageArray = server_messages;
//	}
//	else
//	{
//		MessageArray = client_messges;
//	}
//	int message_type_code;
//	char* lp_params = NULL;
//	message* proccessed_message;
//	proccessed_message = (message*)malloc(sizeof(message));
//	memset(proccessed_message->message_arguments, 0, MAX_NUM_OF_PARAMS * MAX_LEN_OF_PARAM);
//	//ret_val = CheckAlocation(proccessed_message);
//	/*if (ret_val != SUCCESS)
//	{
//		return ret_val;
//	}*/
//	/* get the message type */
//	lp_params = strchr(tmp_message, ':');
//	lp_params++;
//	//if (lp_params != NULL)//parameterless message type
//	message_type_code = get_message_code(tmp_message, is_server);
//	// one of the messages and has params; 
//	if (message_type_code == -1)
//	{
//		printf("invalid message");
//		return -1;
//	}
//	if (is_server)
//	{
//		proccessed_message->ServerType = (ServerMesType)message_type_code;
//		proccessed_message->ClientType = -1;
//	}
//	else
//	{
//		proccessed_message->ClientType = (ClientMesType)message_type_code;;
//		proccessed_message->ServerType = -1;
//	}
//	proccessed_message->num_of_arguments = 0;
//	if (lp_params)
//	{
//		int i= 0;
//		int j = 0;
//		int num_of_params = 0;
//		while (lp_params[i] != '\0')
//		{
//			if (lp_params[i] == ';')
//			{
//				lp_params[i] = '\0';
//				strcpy(proccessed_message->message_arguments[num_of_params++], &lp_params[j]);
//				j = i + 1;
//			}
//			i++;
//		}
//		strcpy(proccessed_message->message_arguments[num_of_params++], &lp_params[j]);
//		proccessed_message->num_of_arguments = num_of_params;
//	}
//	return proccessed_message;
//}
//
//int get_message_code(const char* message_type, int is_server) {
//	int i, len;
//	const char** lut;
//	if (is_server)
//	{
//		lut = server_messages;
//		len = SERVER_MESSAGES_LEN;
//	}
//	else
//	{
//		lut = client_messges;
//		len = CLIENT_MESSAGES_LEN;
//	}
//
//	for (i = 0; i < len; i++) {
//		if (strstr(message_type, lut[i]) == message_type)
//			return i;
//	}
//	return -1;
//}