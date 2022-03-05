#include "include/game.h"

Game *Game_Create(Console *console)
{
    Game *game = malloc(sizeof(Game));
    game->active = true;
    game->console = console;
    game->map = Map_Create((Size2D){ console->size.width, console->size.height - 5 }, (Point2D){ 0, 0 });

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

void Game_HandleInput(Game *game)
{
    game->key = Console_Getch(game->console);

    Map_Generate(game->map);

    Console_Clear(game->console);
    Map_Render(game->map, game->console);
    Console_Refresh(game->console);
}