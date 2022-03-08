#include "include/game.h"

Game *Game_Create(Console *console)
{
    Game *game = malloc(sizeof(Game));
    game->active = true;
    game->commandActive = -1;
    game->console = console;
    game->map = Map_Create((Size2D){ 80, 20 }, (Point2D){ 0, 0 });
    game->screen = SCREEN_TITLE;
    game->turn = 1;

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

    if(game->screen == SCREEN_TITLE)
    {
        Map_Generate(game->map);
        Game_Log(game, "The ship teeters on the brink...", CONSOLECOLORPAIR_WHITEBLACK, 0);

        Console_Clear(game->console);
        Map_Render(game->map, game->map->player, game->console);
        
        Game_RenderUI(game);
        
        Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
        Console_Refresh(game->console);

        game->screen = SCREEN_MAIN;
        return;
    }

    if(game->screen == SCREEN_MAIN)
    {
        if(!(game->map->player->flags & MAPOBJECTFLAG_ISLIVING))
        {
            //...
            return;
        }

        Direction2D direction = Game_GetInputDirection(game, game->key);

        if(game->commandActive > -1)
        {
            if(game->key == 27) // Esc.
            {
                game->commandActive = -1;
                Game_Log(game, "Nevermind.", CONSOLECOLORPAIR_WHITEBLACK, 0);

                Game_RenderUI(game);
                Console_SetCursor(game->console, 1);
                Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
                Console_Refresh(game->console);
                return;
            }

            if(game->commandActive == COMMAND_LOOK)
            {
                if(direction.x == 0 && direction.y == 0) return;
                
                Point2D to = (Point2D){ game->commandPoint.x + direction.x, game->commandPoint.y + direction.y };

                int view = Map_GetObjectView(game->map, game->map->player, to);
                if(view != MAPOBJECTVIEW_VISIBLE) return;

                game->commandPoint = to;
                char *desc = Map_GetPointDescription(game->map, to);
                Game_LogChangeF(game, CONSOLECOLORPAIR_YELLOWBLACK, 0, "Look at what? (Direction, Esc.) -> %s", desc);

                Game_RenderUI(game);
                Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->commandPoint.x, game->map->renderOffset.y + game->commandPoint.y });
                Console_Refresh(game->console);
                return;
            }

            if(game->commandActive == COMMAND_OPENCLOSE)
            {
                if(direction.x == 0 && direction.y == 0) return;
                
                Point2D to = (Point2D){ game->map->player->position.x + direction.x, game->map->player->position.y + direction.y };

                int view = Map_GetObjectView(game->map, game->map->player, to);
                if(view != MAPOBJECTVIEW_VISIBLE) return;
                MapTile *tile = Map_GetTile(game->map, to);
                if(tile == NULL) return;

                if(tile->objectsCount == 0)
                {
                    Game_Log(game, "There's nothing to open or close there.", CONSOLECOLORPAIR_WHITEBLACK, 0);
                    Game_RenderUI(game);
                    Console_Refresh(game->console);
                    return;
                }

                MapObject *toOpenClose = NULL;
                for(int i = 0; i < tile->objectsCount; i++)
                {
                    if(!(tile->objects[i]->flags & MAPOBJECTFLAG_CANOPEN)) continue;

                    toOpenClose = tile->objects[i];
                    break;
                }

                if(toOpenClose == NULL)
                {
                    Game_Log(game, "That can't be opened or closed.", CONSOLECOLORPAIR_WHITEBLACK, 0);
                    Game_RenderUI(game);
                    Console_Refresh(game->console);
                    return;
                }

                MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_OPEN);
                action->target = toOpenClose;
                action->to = to;

                action = Map_ObjectAttemptActionAsTarget(game->map, toOpenClose, action);
                if(!action->result)
                {
                    Game_Log(game, "That can't be opened or closed.", CONSOLECOLORPAIR_WHITEBLACK, 0);
                    Game_RenderUI(game);
                    Console_Refresh(game->console);
                    return;
                }

                if(toOpenClose->flags & MAPOBJECTFLAG_ISOPEN)
                    Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "You open the %s.", toOpenClose->name);
                else
                    Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "You close the %s.", toOpenClose->name);
            
                game->commandActive = -1;
            }
        }
        else
        {

            if(game->key == 111) // 'O' == Open / Close
            {
                game->commandActive = COMMAND_OPENCLOSE;
                Game_Log(game, "Open / close in which direction?", CONSOLECOLORPAIR_YELLOWBLACK, 0);

                Game_RenderUI(game);
                Console_Refresh(game->console);
                return;
            }

            if(game->key == 120) // 'X' == Look
            {
                game->commandActive = COMMAND_LOOK;
                Game_Log(game, "Look at what? (Direction, Esc.) -> Yourself.", CONSOLECOLORPAIR_YELLOWBLACK, 0);
                game->commandPoint = (Point2D){ game->map->player->position.x, game->map->player->position.y };

                Game_RenderUI(game);
                Console_SetCursor(game->console, 2);
                Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->commandPoint.x, game->map->renderOffset.y + game->commandPoint.y });
                Console_Refresh(game->console);
                return;
            }
        
            if(direction.x != 0 || direction.y != 0)
            {
                MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_MOVE);
                action->direction = direction;
                action->object = game->map->player;

                action = Map_AttemptObjectAction(game->map, action);

                if(action->result)
                {
                    Game_MapNextTurn(game, game->map);

                    if(action->type == MAPOBJECTACTIONTYPE_OPEN)
                        Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "You open the %s.", action->target->name);
                }

                MapObjectAction_Destroy(action);
            }
            else
            {
                // Wait
                if(game->key == 46 || game->key == 53)
                    Game_MapNextTurn(game, game->map);
            }
        }

        Map_UpdateObjectView(game->map, game->map->player);

        Map_RenderForPlayer(game->map, game->console);

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
    logMessage->str = malloc(sizeof(char) * (strlen(str) + 1));
    memset(logMessage->str, 0, strlen(str) + 1);
    strncpy(logMessage->str, str, strlen(str));

    game->log[game->logSize] = logMessage;
    game->logSize++;
}

void Game_LogChange(Game *game, char *str, int colorPair, int attributes)
{
    if(game->logSize == 0) return;

    LogMessage *logMessage = game->log[game->logSize - 1];
    logMessage->attributes = attributes;
    logMessage->colorPair = colorPair;
    free(logMessage->str);
    logMessage->str = malloc(sizeof(char) * (strlen(str) + 1));
    memset(logMessage->str, 0, strlen(str) + 1);
    strncpy(logMessage->str, str, strlen(str));
}

void Game_LogChangeF(Game *game, int colorPair, int attributes, const char *fmt, ...)
{
    va_list args;        
    va_start(args, fmt);

    char str[256];
    memset(str, 0, 256);
    vsnprintf(str, 256, fmt, args);

    Game_LogChange(game, str, colorPair, attributes);

    va_end(args);
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

void Game_MapNextTurn(Game *game, Map *map)
{
    game->turn++;

    map->levelFloodTimer--;
    if(map->levelFloodTimer == 0)
    {
        map->levelFloodTimer = (int)map->levelFloodTimerSize;
        Map_PlaceObject(map, Map_CreateObject(map, MAPOBJECTID_WATERSOURCE));
        
        if(map->levelFloodTimerSize > 10) map->levelFloodTimerSize -= 7 + rand() % 3;
        map->levelFloodTimer = map->levelFloodTimerSize;

        Game_Log(game, "The ship groans and water rushes upwards!", CONSOLECOLORPAIR_CYANBLACK, 0);
    }

    while(true)
    {
        for(int y = 0; y < map->size.height; y++)
        {
            for(int x = 0; x < map->size.width; x++)
            {
                MapTile *tile = Map_GetTile(map, (Point2D){ x, y });
                if((int)tile->objectsCount == 0) continue;

                for(int i = 0; i < tile->objectsCount; i++)
                {
                    MapObject *object = tile->objects[i];
                    if(object->turnTicksSize == 0)
                        continue;
                    object->turnTicks--;
                    if(object->turnTicks > 0) continue;

                    Game_MapObjectTakesTurn(game, map, object);
                    if(object == map->player) return;
                }
            }
        }
    }
}

void Game_MapObjectTakesTurn(Game *game, Map *map, MapObject *mapObject)
{
    mapObject->turnTicks = mapObject->turnTicksSize;

    if(mapObject->flags & MAPOBJECTFLAG_ISLIVING)
    {
        if(mapObject->o2 == 0)
        {
            mapObject->hp--;
            if(mapObject->hp == 0)
            {
                mapObject->flags ^= MAPOBJECTFLAG_ISLIVING;
                if(mapObject == map->player)
                    Game_Log(game, "YOU DROWN! GAME OVER!", CONSOLECOLORPAIR_REDBLACK, A_BOLD);
            }
        }

        MapTile *tile = Map_GetTile(map, mapObject->position);

        if(tile->objectsCount > 0)
        {
            for(int i = 0; i < tile->objectsCount; i++)
            {
                if(!(tile->objects[i]->flags & MAPOBJECTFLAG_ISLIQUID))
                    continue;
                if(tile->objects[i]->height < 5) break;

                if(mapObject->o2 > 0) mapObject->o2--;
                return;
            }
        }

        if(mapObject->o2 < mapObject->o2Max) mapObject->o2++;
    }

    if(mapObject == map->player) return;

    if(mapObject->flags & MAPOBJECTFLAG_ISLIQUID)
    {
        if(mapObject->height == 1) return;

        Point2D offset = (Point2D){ -1 + rand() % 3, -1 + rand() % 3 };
        if(offset.x == 0 && offset.y == 0) return;
        Point2D point = (Point2D){ mapObject->position.x + offset.x, mapObject->position.y + offset.y };

        MapTile *tileTo = Map_GetTile(map, point);
        if(!(tileTo->passable & MAPTILEPASSABLE_LIQUID)) return;

        if(tileTo->objectsCount > 0)
        {
            for(int i = 0; i < tileTo->objectsCount; i++)
            {
                if(!(tileTo->objects[i]->flags & MAPOBJECTFLAG_ISLIQUID))
                    continue;
                if(tileTo->objects[i]->height >= mapObject->height || tileTo->objects[i]->height >= 7)
                    return;

                mapObject->height--;
                tileTo->objects[i]->height++;
                mapObject->wchr = (mapObject->height >= 5) ? L'≈' : L'~';
                tileTo->objects[i]->wchr = (tileTo->objects[i]->height >= 5) ? L'≈' : L'~';
                return;
            }
        }
        
        mapObject->height--;
        mapObject->wchr = (mapObject->height >= 5) ? L'≈' : L'~';
        MapObject *copy = MapObject_Copy(mapObject);
        copy->height = 1;
        copy->position = point;
        copy->wchr = L'~';
        MapTile_AddObject(tileTo, copy);
        return;
    }

    if(mapObject->flags & MAPOBJECTFLAG_ISLIQUIDSOURCE)
    {
        MapTile *tile = Map_GetTile(map, mapObject->position);

        if(tile->objectsCount > 0)
        {
            for(int i = 0; i < tile->objectsCount; i++)
            {
                if(!(tile->objects[i]->flags & MAPOBJECTFLAG_ISLIQUID))
                    continue;
                if(tile->objects[i]->height >= 7) return;
                
                tile->objects[i]->height += 3 + rand() % 3;
                return;
            }
        }

        MapObject *water = Map_CreateObject(map, MAPOBJECTID_WATER);
        water->height = 3 + rand() % 3;
        water->position = mapObject->position;
        water->wchr = L'≈';
        MapTile_AddObject(tile, water);

        return;
    }
}

void Game_RenderUI(Game *game)
{
    Console_WriteF(game->console, 0, 0, CONSOLECOLORPAIR_BLACKWHITE, 0, "KEY = %d   ", game->key);

    int max = 5;
    if(game->logSize < 5) max = game->logSize;

    for(int i = 0; i < max; i++)
    {
        LogMessage *lm = game->log[game->logSize - (i + 1)];
        Console_ClearRow(game->console, 20 + i);
        Console_Write(game->console, 20 + i, 20, lm->str, lm->colorPair, lm->attributes);
    }

    Console_Write(game->console, 20, 0, game->map->player->name, CONSOLECOLORPAIR_WHITEBLACK, A_BOLD);
    Console_WriteF(game->console, 21, 0, CONSOLECOLORPAIR_WHITEBLACK, 0, "T: %d   ", game->turn);
    Console_Write(game->console, 22, 0, "HP: [          ]", CONSOLECOLORPAIR_WHITEBLACK, 0);
    Console_Write(game->console, 23, 0, "O2: [          ]", CONSOLECOLORPAIR_WHITEBLACK, 0);
    Console_DrawBarW(game->console, 22, 5, 10, game->map->player->hp, game->map->player->hpMax, CONSOLECOLORPAIR_REDBLACK, 0);
    Console_DrawBarW(game->console, 23, 5, 10, game->map->player->o2, game->map->player->o2Max, CONSOLECOLORPAIR_CYANBLACK, 0);
}