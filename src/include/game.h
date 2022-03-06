#ifndef GAME_H_eX8JrWUXqh42cKgV
#define GAME_H_eX8JrWUXqh42cKgV

#include "map.h"

typedef struct LogMessage
{
    int attributes;
    int colorPair;
    char *str;
} LogMessage;

typedef struct Game
{
    bool active;
    Console *console;
    char key;
    LogMessage *log[UINT16_MAX];
    size_t logSize;
    Map *map;
    int screen;
} Game;

Game *Game_Create(Console *console);
void Game_Destroy(Game *game);
Direction2D Game_GetInputDirection(Game *game, char key);
void Game_HandleInput(Game *game);
void Game_Log(Game *game, char *str, int colorPair, int attributes);
void Game_LogF(Game *game, int colorPair, int attributes, const char *fmt, ...);
void Game_RenderUI(Game *game);

#endif