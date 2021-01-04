

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

/*	Functions that handle particular incoming messages	*/
int send_message_server(message* lp_message, SOCKET s_target) {

	TransferResult_t SendRes;
	char* SendStr = NULL;

	SendStr = (char*)malloc(1 + strlen(lp_message->message_arguments));
	if (SendStr == NULL) {
		
		printf("Allocation error\n");
		return 0;
	}
	strcat(SendStr, lp_message->message_arguments);
	SendRes = SendString(SendStr, s_target);
	free(SendStr);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error\n");
		closesocket(s_target);
		return 0;
	}


	return 1;
}

/*	Utility functions that support other function for general things	*/
int get_message_code(const char* message_type, int is_server) {
	int i, len;
	char** lut;
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
	is_server = !is_server;
	char** lut;
	if (is_server)
	{
		lut = server_messages;
	}
	else
	{
		lut = client_messges;
	}
	int message_type_code;
	char* lp_params = NULL;
	message* proccessed_message;
	proccessed_message = (message*)malloc(sizeof(message));
	/* get the message type */
	lp_params = strchr(message_text, ':');
	//if (lp_params != NULL)//parameterless message type
	message_type_code = get_message_code(message_text, is_server);
	/*else {
		message_type_size = (int)(lp_params - message_text);//lp_params-message=length of the first part of the message, e.g.:SEND_MESSAGE ==> 11 chars
		memcpy(message_type_string, message_text, message_type_size);//extract the first time of the message
		message_type_string[message_type_size] = '\0'; //add null terminator
		message_type_code = get_message_code_server(message_type_string);
	}*/

	if (message_type_code >= 0 || lp_params) {
		proccessed_message->message_type = lut[message_type_code];
		proccessed_message->message_arguments = (lp_params)+1;
	}
	else {
		proccessed_message->message_type = NULL;
		proccessed_message->message_arguments = NULL;
	} 
	return proccessed_message;
}