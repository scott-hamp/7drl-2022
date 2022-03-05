#ifndef GAME_H_eX8JrWUXqh42cKgV
#define GAME_H_eX8JrWUXqh42cKgV

#include "map.h"

typedef struct Game
{
    bool active;
    Console *console;
    char key;
    Map *map;
} Game;

Game *Game_Create(Console *console);
void Game_Destroy(Game *game);
void Game_HandleInput(Game *game);

#endif