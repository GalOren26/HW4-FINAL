#ifndef _bulls_and_cows
#define _bulls_and_cows
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define OUT
#define _CRT_SECURE_NO_WARNINGS

#define _WINSOCKAPI_ 

bool containsNoDuplicates(char* guess);

//return num of bulls and cows in this match and if the player guess all correct (meaning draw or win ) 
bool PlayRoundPlayer(char* number, char* guess, int* OUT bulls, int* OUT  cows, bool ReturnHowManyHits);
//void gameResults(game_results* results, int game_status);
int playerChoice();
//int otherPlayerQuit(char* message);
void showMenu(int menu, int portNumber, char* ip);
//char* chooseNumber();
bool isRightSize(char* guess);
bool isOnlyDigits(char* number);
int isValid(char* number);
#endif _bulls_and_cows