//Name:messages.h
//Description: headers for the message module
#ifndef _MESSAGES_H
#define _MESSAGES_H
#include "HardCodedData.h"
#include <string.h>
#include <stdio.h>
#include "Socket.h"
#include "Functions.h"

#define SERVER_MESSAGES_LEN 11
#define CLIENT_MESSAGES_LEN 5
#define MAX_NUM_OF_PARAMS 7
#define MAX_LEN_OF_PARAM 50
#define MAX_LEN_NAME_PLAYER 20
#define MAX_LEN_MESSAGE MAX_LEN_OF_PARAM*(MAX_NUM_OF_PARAMS+1)
#define IsServer 1 
#define IsClient 0
typedef  enum {
	SERVER_MAIN_MENU,
	SERVER_APPROVED,
	SERVER_DENIED,
	SERVER_INVITE,
	SERVER_SETUP_REQUSET,
	SERVER_PLAYER_MOVE_REQUEST,
	SERVER_GAME_RESULTS,
	SERVER_WIN,
	SERVER_DRAW,
	SERVER_NO_OPPONENTS,
	SERVER_OPPONENT_QUIT,
	MAIN,
	FAILURE,
	DENIED,
	end
}ServerMesType;
typedef  enum {
	CLIENT_REQUEST,
	CLIENT_VERSUS,
	CLIENT_SETUP,
	CLIENT_PLAYER_MOVE,
	CLIENT_DISCONNECT,
	end1
}ClientMesType;

typedef struct  {

	ClientMesType ClientType;
	ServerMesType ServerType;
	char message_arguments[MAX_NUM_OF_PARAMS][MAX_LEN_OF_PARAM];
	int num_of_arguments;

}message;






extern const char* server_messages[SERVER_MESSAGES_LEN];
extern const char* client_messges[CLIENT_MESSAGES_LEN];
/*	Declarations	*/
void delete_message(message* lp_msg);

message* process_Message(const char* message_text, int is_server);

int get_message_code(const char* message_type, int is_server);
//
//int send_message_server(message* lp_message, SOCKET s_target);


void MessageToString(message* lp_message, int is_server, char*  string);


#endif