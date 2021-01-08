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
		return NULL;
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
//////

//void gameResults(game_results* results, int game_status) {
//	if (game_status == GAME) {
//		printf("Bulls: %c\n", results->bulls);
//		printf("Cows: %c\n", results->cows);
//		printf("%s played: %s\n", results->oponent_name, results->opponent_move);
//
//	}
//	if (game_status == TIE) {
//		printf("It's a tie");
//		return;
//	}
//	if (game_status == WIN) {
//		printf("%s won!\n", results->player);
//		printf("Opponent number was %s\n", results->player_move);
//	}
//}
///////
//
//int otherPlayerQuit(char* message) {
//	if (!strcmp(message, "SERVER_OPPONENT_QUIT")) {
//		printf("Opponent quit.\n");
//		return START_AGAIN;
//	}
//	else
//		return CONTINUE;
//}
void showMenu(int choose, int portNumber, char ip) {
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

	//decision = playerChoice();
	//return decision;
}
int isValid(char* number) {
	if (!isRightSize(number))
		return 0;
	if (!isOnlyDigits(number))
		return 0;
	if (!containsNoDuplicates(number))
		return 0;
	return 1;
}

//this function gets the number from the user
//char* chooseNumber() {
//	char* guess = NULL;
//	int i = 0, flag = 1;

//	if (NULL == (guess = malloc((guess_size + 1) * sizeof(char)))) {
//		printf("Fatal error: memory allocation failed (chooseNumber).\n");
//		return NULL;
//	}

//	while (flag) {
//		scanf_s("%s", guess, guess_size);
		///validations:
//		if (!isRightSize(guess))
//			continue;
//		if (!isOnlyDigits(guess))
//			continue;
//		if (!containsNoDuplicates(guess))
//			continue;
//		flag = 0;
//	}
//	return guess;
//}
bool isRightSize(char* guess) {
	if (guess_size != strlen(guess)) {
		printf("please insert exactly 4 digits.\n");
		return false;
	}
	return true;
}
bool isOnlyDigits(char* number) {
	for (int i = 0; i < guess_size; i++) {
		if (!isdigit(number[i])) {
			printf("Please insert only digits.\n");
			return false;
		}
	}
	return true;
}
bool containsNoDuplicates(char* guess)
{
	// If at any iteration we encounter 2
	// same characters, return false
	for (int i = 0; i < strlen(guess) - 1; i++) {
		for (int j = i + 1; j < strlen(guess); j++) {
			if (guess[i] == guess[j]) {
				return false;
			}
		}
	}
	return true;
}
///we will call countBullsAndCows(number,guess, &bulls, &cows)
void countBullsAndCows(char* number, char* guess, int* bulls, int* cows)
{
	int i;
	// int cows = 0, bulls = 0;

	for (i = 0; i < guess_size; i++) {
		if (number[i] == guess[i]) {
			(*bulls)++;
		}
		else if (strchr(number, guess[i]) != NULL) {
			(*cows)++;
		}
	}
}

