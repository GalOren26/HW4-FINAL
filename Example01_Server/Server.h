/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
 This file was written for instruction purposes for the 
 course "Introduction to Systems Programming" at Tel-Aviv
 University, School of Electrical Engineering, Winter 2011, 
 by Amnon Drory.
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef SERVER_H
#define SERVER_H
#include "../Shared/HardCodedData.h"
#include <stdio.h>
#include <string.h>
//win includs 
#include <winsock2.h>
#pragma comment(lib,"shlwapi.lib")
#include <shlwapi.h>
#include <ws2tcpip.h>
#include <windows.h>
//
#include "Socket.h"
#include "messages.h"
#include "threads.h"
#include "Functions.h"
#include <stdbool.h> 
#include "bullsAndCows.h"


#define MAX_LOOPS 3
#define FirstThread 0 
#define SecondThread 1



void MainServer(int port);
int FindFirstUnusedThreadSlot();
void CleanupWorkersThreads();
int InitSyncElements();


int PlayRound(message* lp_message, int* round, int myIndex, char* My_Secret, char* Other_Secret, SOCKET* t_socket);
int ManageMessageReceived(message* lp_message, int myIndex, SOCKET* t_socket, bool* OUT IOpenTheFile, char  My_Secret[], char Other_Secret[]);
int player1_read_write_file(char* input_line, char* header_message, char* OUT output_line);
int player2_read_write_file(char* input_line, char* OUT output_line);


#endif // SOCKET_EXAMPLE_SERVER_H