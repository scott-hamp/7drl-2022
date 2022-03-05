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
        Console_Clear(game->console);
        Map_Render(game->map, game->map->player, game->console);
        Console_WriteF(game->console, game->map->size.height + 1, 0, CONSOLECOLORPAIR_BLACKWHITE, 0, "KEY = %d   ", game->key);
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
                MapObject_UpdateView(game->map->player, game->map);
                Map_Render(game->map, game->map->player, game->console);
            }

            MapObjectAction_Destroy(action);
        }

        Console_WriteF(game->console, game->map->size.height + 1, 0, CONSOLECOLORPAIR_BLACKWHITE, 0, "KEY = %d   ", game->key);
        Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
        Console_Refresh(game->console);

        return;
    }
}