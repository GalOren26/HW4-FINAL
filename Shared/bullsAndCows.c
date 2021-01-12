#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "bullsAndCows.h"
#include "HardCodedData.h"
#include "messages.h"
#define guess_size 4
#define choise_size 1


int playerChoice() {
	char* choice = NULL, decision = NULL;
	int flag = 1;

	if (NULL == (choice = malloc(sizeof(char*) * choise_size))) {
		printf("memory allocation failed (playerChoice).\n");
		return -1;
	}
	while (flag) {
		scanf_s("%s", choice, choise_size);
		if (1 != strlen(choice)) {
			printf("Invalid choice. please try again.\n");
			continue;
		}
		decision = choice[0];
		if (decision != '1' && decision != '2') {
			printf("Invalid choice. please try again.\n");
			continue;
		}
		flag = 0;
	}
	free(choice);
	if ('1' == decision)
		return 1;
	else
		return 2;
}

//input arguments - choose : an option of one of the menus , portNumber - the port number, ip - the ip
//output arguments : none
//functionality : this function prints the right message(out of 3) by a chosen state
void showMenu(int choose, int portNumber, char* ip) {
	//int decision;
	switch (choose) {
	case MAIN:
		printf("Choose what to do next:\n");
		printf("1. Play against another client\n");
		printf("2. Quit\n");
		break;
	case FAILURE:
		printf("Failed connecting to server on %s:%d.\n", ip, portNumber);
		printf("Choose what to do next:\n");
		printf("1. Try to reconnect\n");
		printf("2. Exit\n");
		break;
	case DENIED:
		printf("Server on %s:%d denied the connection request.\n", ip, portNumber);
		printf("Choose what to do next:\n");
		printf("1. Try to reconnect\n");
		printf("2. Exit\n");
		break;
	}
}
//input arguments - number - the player's initial number or the player's guess
//output arguments : 1 for valid number an 0 for unvalid number
//functionality : this function prints the right message(out of 3) by a chosen state
int isValid(char* number) {
	if (!isRightSize(number))
		return 0;
	if (!isOnlyDigits(number))
		return 0;
	if (!containsNoDuplicates(number))
		return 0;
	return 1;
}
//input arguments - number - the player's initial number or the player's guess
//output arguments : true for valid number and false for unvalid number
//functionality : this functionchecks if the number is in the right size( 4 chars)
bool isRightSize(char* guess) {
	if (guess_size!= strlen(guess)) {
		printf("insert exactly 4 digits. please try again \n");
		return false;
	}
	return true;
}
//input arguments - number - the player's initial number or the player's guess
//output arguments : true for valid number and false for unvalid number
//functionality : this function checks if the number is just digits 
bool isOnlyDigits(char* number) {
	for (int i = 0; i < guess_size; i++) {
		if (!isdigit(number[i])) {
			printf("insert only digits. please try again \n");
			return false;
		}
	}
	return true;
}
//input arguments - number - the player's initial number or the player's guess
//output arguments : true for valid number and false for unvalid number
//functionality : this function checks if the number contain the same number more than once
bool containsNoDuplicates(char* guess)
{
	int len = strlen(guess);
	for (int i = 0; i < len - 1; i++) {
		for (int j = i + 1; j < len; j++) {
			if (guess[i] == guess[j]) {
				printf("the number should'nt contain duplicates , please try again \n");
				return false;
			}
		}
	}
	return true;
}
//input arguments - number - the player's given number, guess- the player's guess, bulls - will be the number of same number in the same place at the "guess" and "number, coes- the number of the same number not in the 
//right place in the "number" and "guess", return How many hits- true- to return number of bulls and coes and false for not to return
//output arguments : true-if temp_bulls == guess_size(the player wins) or false for the inequality(the player didn't win)
//functionality : this function operats one round for a player
bool PlayRoundPlayer(char* number, char* guess, int* OUT bulls, int* OUT  cows,bool ReturnHowManyHits)
{
	int i;
	int temp_bulls = 0, temp_cows = 0;
	for (i = 0; i < guess_size; i++) {
		if (number[i] == guess[i]) {
			(temp_bulls)++;
		}
		else if (strchr(number, guess[i]) != NULL) {
			(temp_cows)++;
		}
	}
	if (ReturnHowManyHits)
	{
		*cows = temp_cows;
		*bulls = temp_bulls;
	}
	if (temp_bulls == guess_size)
		return true; 
	return false; 
}
