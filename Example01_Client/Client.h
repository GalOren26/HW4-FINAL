/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering, Winter 2011, 
 by Amnon Drory.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef CLIENT_H
#define CLIENT_H
#include "../Shared/HardCodedData.h"
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include "messages.h"
#include "Socket.h"
#include "threads.h"

#define NumOfClientThreads 2
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

int MainClient(int argc, char* argv[] );

int ConnectToServerWithUI(SOCKET *my_socket, SOCKADDR* my_clientService, int SizeOfclientService);
static DWORD SendDataThread(void);
static DWORD RecvDataThread(void);

#endif // SOCKET_EXA MPLE_CLIENT_H