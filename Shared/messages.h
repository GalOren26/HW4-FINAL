//Name:messages.h
//Description: headers for the message module
#ifndef _MESSAGES_H
#define _MESSAGES_H
#include "HardCodedData.h"
#include <string.h>
#include <stdio.h>
#include "Socket.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#define SERVER_MESSAGES_LEN 11
#define CLIENT_MESSAGES_LEN 5


typedef struct message {

	const char* message_type;
	char* message_arguments;

}message;

extern const char* server_messages[SERVER_MESSAGES_LEN];
extern const char* client_messges[CLIENT_MESSAGES_LEN];
/*	Declarations	*/
void delete_message(message* lp_msg);

message* process_Message(const char* message_text, int is_server);

int get_message_code(const char* message_type, int is_server);

int send_message_server(message* lp_message, SOCKET s_target);



#endif