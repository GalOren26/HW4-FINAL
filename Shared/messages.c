

/*
*	Name:messages.c
*	messages module implementation
*/


#include "messages.h"

const char* server_messages[] = { "SERVER_MAIN_MENU",
							   "SERVER_APPROVED",
							   "SERVER_DENIED",
							   "SERVER_INVITE",
							   "SERVER_SETUP_REQUSET",
							   "SERVER_PLAYER_MOVE_REQUEST",
							   "SERVER_GAME_RESULTS",
							   "SERVER_WIN",
							   "SERVER_DRAW",
							   "SERVER_NO_OPPONENTS",
							   "SERVER_OPPONENT_QUIT" }; //extern because client.c uses this too


const char* client_messges[] = { "CLIENT_REQUEST",
							   "CLIENT_VERSUS",
							   "CLIENT_SETUP",
							   "CLIENT_PLAYER_MOVE",
							   "CLIENT_DISCONNECT" };

#define MAX_MESSAGE_TYPE_SIZE 19 //according to to what we are given, that is 18 characters + null terminator


/* Free memory	*/
void delete_message(message* lp_msg) {

	free(lp_msg);
}


///*	Functions that handle particular incoming messages	*/
//int send_message_server(message* lp_message, SOCKET s_target) {
//
//	TransferResult_t SendRes;
//	char* SendStr = NULL;
//
//	SendStr = (char*)malloc(1 + strlen(lp_message->message_arguments));
//	if (SendStr == NULL) {
//
//		printf("Allocation error\n");
//		return 0;
//	}
//	strcat(SendStr, lp_message->message_arguments);
//	SendRes = SendString(SendStr, s_target);
//	free(SendStr);
//	if (SendRes == TRNS_FAILED)
//	{
//		printf("Service socket error\n");
//		closesocket(s_target);
//		return 0;
//	}
//	return 1;
//}
message* parametersToString() {

}
void MessageToString(message* lp_message, int is_server, char*  string)
{
	if (is_server)
	{
		strcpy(string,server_messages[lp_message->ServerType]);
	}
	else
	{
		strcpy(string, server_messages[lp_message->ClientType]);
	}
	strcat(string, ":");
	for (int i = 0; i < lp_message->num_of_arguments;i++)
	{
		strcat(string, lp_message->message_arguments[i]);
		strcat(string, ";");
	}
	string[strlen(string) - 1] = '\0';//delete the last ; 
}


/*	Utility functions that support other function for general things	*/
int get_message_code(const char* message_type, int is_server) {
	int i, len;
	const char** lut;
	if (is_server)
	{
		lut = server_messages;
		len = SERVER_MESSAGES_LEN;
	}
	else
	{
		lut = client_messges;
		len = CLIENT_MESSAGES_LEN;
	}

	for (i = 0; i < len; i++) {
		if (strstr(message_type, lut[i]) == message_type)
			return i;
	}
	return -1;
}



message* process_Message(const char* message_text, int is_server) {
	char tmp_message[MAX_LEN_MESSAGE];
	strcpy(tmp_message, message_text);
	int ret_val = SUCCESS;
	is_server = !is_server;
	const char** MessageArray;
	if (is_server)
	{
		MessageArray = server_messages;
	}
	else
	{
		MessageArray = client_messges;
	}
	int message_type_code;
	char* lp_params = NULL;
	message* proccessed_message;
	proccessed_message = (message*)malloc(sizeof(message));
	ret_val = CheckAlocation(proccessed_message);
	if (ret_val != SUCCESS)
	{
		return ret_val;
	}
	memset(proccessed_message->message_arguments, 0, MAX_NUM_OF_PARAMS * MAX_LEN_OF_PARAM);
	/* get the message type */
	lp_params = strchr(tmp_message, ':');
	if (lp_params != 0)
	{
		lp_params++;
	}
	//if (lp_params != NULL)//parameterless message type
	message_type_code = get_message_code(tmp_message, is_server);
	// one of the messages and has params; 
	if (message_type_code == -1)
	{
		printf("invalid message");
		return -1;
	}
	if (is_server)
	{
		proccessed_message->ServerType = (ServerMesType)message_type_code;
		proccessed_message->ClientType = -1;
	}
	else
	{
		proccessed_message->ClientType = (ClientMesType)message_type_code;
		proccessed_message->ServerType = -1;
	}
	proccessed_message->num_of_arguments = 0;
	if (lp_params)
	{
		int i = 0;
		int j = 0;
		int num_of_params = 0;
		while (lp_params[i] != '\0')
		{
			if (lp_params[i] == ';')
			{
				lp_params[i] = '\0';
				strcpy(proccessed_message->message_arguments[num_of_params++], &lp_params[j]);
				j = i + 1;
			}
			i++;
		}
		strcpy(proccessed_message->message_arguments[num_of_params++], &lp_params[j]);
		proccessed_message->num_of_arguments = num_of_params;
	}
	return proccessed_message;
}

