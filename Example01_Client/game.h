#pragma once
#include <stdio.h>
#include <stdlib.h>

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