#include "include/game.h"

int main(int argc, char **argv)
{
    Console *console = Console_Create();

    Game *game = Game_Create(console);

    while(game->active)
    {
        Game_HandleInput(game);
    }

    Game_Destroy(game);
    Console_Destroy(console);

    exit(1);

    return 1;
}