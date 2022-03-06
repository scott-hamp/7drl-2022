#include "include/game.h"

Game *Game_Create(Console *console)
{
    Game *game = malloc(sizeof(Game));
    game->active = true;
    game->console = console;
    game->map = Map_Create((Size2D){ console->size.width, console->size.height - 5 }, (Point2D){ 0, 0 });
    game->screen = 0;

    Console_SetCursor(game->console, 1);
    Console_Clear(game->console);
    Console_Write(game->console, 0, 0, "~ 7DRL 2022 ~", 0, 0);
    Console_Write(game->console, 1, 0, "Press any key to start...", 0, 0);
    Console_Refresh(game->console);

    return game;
}

void Game_Destroy(Game *game)
{
    Map_Destroy(game->map);
    free(game);
}

Direction2D Game_GetInputDirection(Game *game, char key)
{
    /*
    left=104,4,52
    right=108,5,54
    up=107,3,56
    down=106,2,50
    nw=121,55
    ne=117,56
    sw=98,49
    se=110,51
    */

   Direction2D direction = (Direction2D){ 0, 0 };

    if(key == 104 || key == 4 || key == 52)
        direction = (Direction2D){ -1, 0 };
    if(key == 108 || key == 5 || key == 54)
        direction = (Direction2D){ 1, 0 };
    if(key == 107 || key == 3 || key == 56)
        direction = (Direction2D){ 0, -1 };
    if(key == 106 || key == 2 || key == 50)
        direction = (Direction2D){ 0, 1 };
    if(key == 121 || key == 55)
        direction = (Direction2D){ -1, -1 };
    if(key == 117 || key == 57)
        direction = (Direction2D){ 1, -1 };
    if(key == 98 || key == 49)
        direction = (Direction2D){ -1, 1 };
    if(key == 110 || key == 51)
        direction = (Direction2D){ 1, 1 };

    return direction;
}

void Game_HandleInput(Game *game)
{
    game->key = Console_Getch(game->console);

    if(game->screen == 0)
    {
        Map_Generate(game->map);
        Game_Log(game, "The ship teeters on the brink...", CONSOLECOLORPAIR_WHITEBLACK, 0);

        Console_Clear(game->console);
        Map_Render(game->map, game->map->player, game->console);
        
        Game_RenderUI(game);
        
        Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
        Console_Refresh(game->console);

        game->screen = 1;
        return;
    }

    if(game->screen == 1)
    {
        Direction2D direction = Game_GetInputDirection(game, game->key);
        if(direction.x != 0 || direction.y != 0)
        {
            MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_MOVE);
            action->direction = direction;
            action->object = game->map->player;

            action = Map_AttemptObjectAction(game->map, action);

            if(action->result)
            {
                Map_UpdateObjectView(game->map, game->map->player);
                Map_Render(game->map, game->map->player, game->console);

                if(action->type == MAPOBJECTACTIONTYPE_OPEN)
                    Game_Log(game, "You open the door.", CONSOLECOLORPAIR_WHITEBLACK, 0);
            }

            MapObjectAction_Destroy(action);
        }

        Game_RenderUI(game);

        Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
        Console_Refresh(game->console);

        return;
    }
}

void Game_Log(Game *game, char *str, int colorPair, int attributes)
{
    LogMessage *logMessage = malloc(sizeof(LogMessage));
    logMessage->attributes = attributes;
    logMessage->colorPair = colorPair;
    logMessage->str = str;

    game->log[game->logSize] = logMessage;
    game->logSize++;
}

void Game_LogF(Game *game, int colorPair, int attributes, const char *fmt, ...)
{
    va_list args;        
    va_start(args, fmt);

    char str[256];
    memset(str, 0, 256);
    vsnprintf(str, 256, fmt, args);

    Game_Log(game, str, colorPair, attributes);

    va_end(args);
}

void Game_RenderUI(Game *game)
{
    Console_WriteF(game->console, 0, 0, CONSOLECOLORPAIR_BLACKWHITE, 0, "KEY = %d   ", game->key);
    
    int max = 5;
    if(game->logSize < 5) max = game->logSize;

    for(int i = 0; i < max; i++)
    {
        LogMessage *lm = game->log[game->logSize - (i + 1)];
        Console_ClearRow(game->console, game->map->size.height + i);
        Console_Write(game->console, game->map->size.height + i, 0, lm->str, lm->colorPair, lm->attributes);
    }
}