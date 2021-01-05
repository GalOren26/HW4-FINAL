#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct game_results {
	char* player;
	char* oponent_name;
	char* player_move;
	char* opponent_move;
	int bulls;
	int cows;
}game_results;

bool containsNoDuplicates(char* guess);
void countBullsAndCows(char* number, char* guess, int* bulls, int* cows);
char* chooseNumber();
void gameResults(game_results* results, int game_status);
int playerChoice();
int otherPlayerQuit(char* message);
int showMenu(int menu, int portNumber, char ip);
char* chooseNumber();
void countBullsAndCows(char* number, char* guess, int* bulls, int* cows);