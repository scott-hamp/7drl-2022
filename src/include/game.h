#ifndef GAME_H_eX8JrWUXqh42cKgV
#define GAME_H_eX8JrWUXqh42cKgV

#include "map.h"

#define COMMAND_DROP          0
#define COMMAND_FIRE          1
#define COMMAND_LOOK          2
#define COMMAND_OPENCLOSE     3
#define COMMAND_WEARWIELD     4

#define SCREEN_HELP     0
#define SCREEN_MAIN     1
#define SCREEN_TITLE    2

typedef struct LogMessage
{
    int attributes;
    int colorPair;
    char *str;
} LogMessage;

typedef struct Game
{
    bool active;
    int commandActive;
    Point2D commandPoint;
    Console *console;
    bool deathMessageLogged;
    char key;
    LogMessage *log[UINT16_MAX];
    size_t logSize;
    Map *map;
    bool refreshMap;
    int screen;
    uint64_t turn;
    bool uiInventoryOpen;
} Game;

Game *Game_Create(Console *console);
void Game_Destroy(Game *game);
Direction2D Game_GetInputDirection(Game *game, char key);
void Game_HandleInput(Game *game);
void Game_Log(Game *game, char *str, int colorPair, int attributes);
void Game_LogChange(Game *game, char *str, int colorPair, int attributes);
void Game_LogChangeF(Game *game, int colorPair, int attributes, const char *fmt, ...);
void Game_LogF(Game *game, int colorPair, int attributes, const char *fmt, ...);
void Game_MapNextTurn(Game *game, Map *map);
void Game_MapObjectTakesTurn(Game *game, Map *map, MapObject *mapObject);
void Game_RenderUI(Game *game);

#endif