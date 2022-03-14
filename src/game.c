#include "include/game.h"

Game *Game_Create(Console *console)
{
    Game *game = malloc(sizeof(Game));
    game->active = true;
    game->commandActive = -1;
    game->console = console;
    game->deathMessageLogged = false;
    game->map = Map_Create((Size2D){ 80, 20 }, (Point2D){ 0, 0 });
    game->refreshMap = false;
    game->screen = SCREEN_TITLE;
    game->turn = 1;
    game->uiInventoryOpen = false;

    Console_SetCursor(game->console, 1);
    Console_Clear(game->console);

    Console_Write(game->console, 0, 0, ".-,--'                .  .         .-,--.          .  .       ", CONSOLECOLORPAIR_CYANBLACK, 0);
    Console_Write(game->console, 1, 0, " \\|__ ,-. ,-. ,-,-.   |- |-. ,-.   ' |   \\ ,-. ,-. |- |-. ,-. ", CONSOLECOLORPAIR_CYANBLACK, 0);
    Console_Write(game->console, 2, 0, "  |   |   | | | | |   |  | | |-'   , |   / |-' | | |  | | `-. ", CONSOLECOLORPAIR_CYANBLACK, 0);
    Console_Write(game->console, 3, 0, " `'   '   `-' ' ' '   `' ' ' `-'   `-^--'  `-' |-' `' ' ' `-' ", CONSOLECOLORPAIR_CYANBLACK, 0);
    Console_Write(game->console, 4, 0, "                                               |              ", CONSOLECOLORPAIR_CYANBLACK, 0);
    Console_Write(game->console, 5, 0, "                                               '", CONSOLECOLORPAIR_CYANBLACK, 0);

    Console_Write(game->console, 6, 0, "This game is a traditional roguelike created by Begebies for the ", 0, 0);
    Console_Write(game->console, 7, 0, "  2022 Seven Day Roguelike. You're stuck on a sinking ship, and ", 0, 0);
    Console_Write(game->console, 8, 0, "  must quickly ascend to the next floor before water floods the ", 0, 0);
    Console_Write(game->console, 9, 0, "  area, while making sure to grab items to help you survive.", 0, 0);

    Console_Write(game->console, 11, 0, "                            Controls: ", 0, 0);
    Console_Write(game->console, 12, 0, " Movement:     vi keys / numpad keys / arrow keys", 0, 0);
    Console_Write(game->console, 13, 0, " Commands: ", 0, 0);
    Console_Write(game->console, 14, 0, " - Drop:                               d", 0, 0);
    Console_Write(game->console, 15, 0, " - Fire:                               f", 0, 0);
    Console_Write(game->console, 16, 0, " - Look:                               x", 0, 0);
    Console_Write(game->console, 17, 0, " - Open / close:                       o", 0, 0);
    Console_Write(game->console, 18, 0, " - Pick up:                            g / ,", 0, 0);
    Console_Write(game->console, 19, 0, " - View help:                          ?", 0, 0);
    Console_Write(game->console, 20, 0, " - View inventory:                     i", 0, 0);
    Console_Write(game->console, 21, 0, " - Wait:                               . / Numpad 5", 0, 0);
    Console_Write(game->console, 22, 0, " - Wield / wear / remove / put away:   w", 0, 0);

    Console_Write(game->console, 24, 0, "                    PRESS ANY KEY TO START!", CONSOLECOLORPAIR_YELLOWBLACK, 0);

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
        Game_Log(game, "The ship begins to slowly sink...", CONSOLECOLORPAIR_WHITEBLACK, 0);

        Console_Clear(game->console);
        Map_Render(game->map, game->map->player, game->console);
        
        Game_RenderUI(game);
        
        Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
        Console_Refresh(game->console);

        game->screen = SCREEN_MAIN;
        return;
    }

    if(game->screen == SCREEN_HELP)
    {
        // Esc.
        if(game->key == 27)
        {
            game->screen = SCREEN_MAIN;

            Console_Clear(game->console);
            Map_Render(game->map, game->map->player, game->console);
            Game_RenderUI(game);

            Console_SetCursor(game->console, 1);
            Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
            Console_Refresh(game->console);
            return;
        }

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
            if(game->key == 27 || ((game->commandActive == COMMAND_DROP || game->commandActive == COMMAND_WEARWIELD ) && game->key == 105)) // Esc.
            {
                game->commandActive = -1;

                Game_Log(game, "Nevermind.", CONSOLECOLORPAIR_WHITEBLACK, 0);

                if(game->uiInventoryOpen)
                {
                    game->uiInventoryOpen = false;
                    Console_Clear(game->console);
                    Map_Render(game->map, game->map->player, game->console);
                }

                Game_RenderUI(game);
                Console_SetCursor(game->console, 1);
                Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
                Console_Refresh(game->console);
                return;
            }

            // Command: Drop

            if(game->commandActive == COMMAND_DROP)
            {
                if(game->key < 'a' || game->key > 'j') return;

                int index = game->key - 'a';
                if(index >= game->map->player->itemsCount) return;

                MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_DROP);
                action->object = game->map->player;
                action->targetItem = game->map->player->items[index];
            
                action = Map_AttemptObjectAction(game->map, action);

                if(!action->result)
                {
                    Game_Log(game, action->resultMessage, CONSOLECOLORPAIR_WHITEBLACK, 0);
                    Game_RenderUI(game);
                    Console_Refresh(game->console);
                    MapObjectAction_Destroy(action);
                    return;
                }

                Game_Log(game, action->resultMessage, CONSOLECOLORPAIR_WHITEBLACK, 0);

                MapObjectAction_Destroy(action);
                game->refreshMap = true;
                game->uiInventoryOpen = false;
                game->commandActive = -1;
                Console_Clear(game->console);
                Console_SetCursor(game->console, 1);
                Game_MapNextTurn(game, game->map);
            }

            // Command: Fire

            if(game->commandActive == COMMAND_FIRE)
            {
                if(direction.x == 0 && direction.y == 0)
                {
                    // Enter
                    if(game->key == 10)
                    {
                        if(game->commandPoint.x == game->map->player->position.x && game->commandPoint.y == game->map->player->position.y)
                        {
                            Game_Log(game, "You can't shoot yourself!", CONSOLECOLORPAIR_WHITEBLACK, 0);
                            Game_RenderUI(game);
                            Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->commandPoint.x, game->map->renderOffset.y + game->commandPoint.y });
                            Console_Refresh(game->console);
                            return;
                        }

                        MapTile *tile = Map_GetTile(game->map, game->commandPoint);
                        MapObject *target = MapTile_GetObjectWithFlags(tile, MAPOBJECTFLAG_ISHOSTILE);
                        
                        if(target == NULL)
                        {
                            Game_Log(game, "There's nothing to fire at there.", CONSOLECOLORPAIR_WHITEBLACK, 0);
                            Game_RenderUI(game);
                            Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->commandPoint.x, game->map->renderOffset.y + game->commandPoint.y });
                            Console_Refresh(game->console);
                            return;
                        }

                        MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_ATTACK);
                        action->object = game->map->player;
                        action->objectItem = game->map->player->equipment[MAPOBJECTEQUIPAT_WEAPON];
                        action->target = target;

                        action = Map_AttemptObjectAction(game->map, action);

                        if(action->result)
                        {
                            if(action->target == NULL)
                                Game_Log(game, "You kill it!", CONSOLECOLORPAIR_WHITEBLACK, 0);
                            else
                                Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "You shoot the %s!", action->target->name);
                        }
                        else
                            Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "You try to shoot the %s, but miss!", action->target->name);
                        
                        game->commandActive = -1;
                        Game_MapNextTurn(game, game->map);
                    }
                    else
                        return;
                }
                else
                {
                    Point2D to = (Point2D){ game->commandPoint.x + direction.x, game->commandPoint.y + direction.y };

                    int view = Map_GetObjectView(game->map, game->map->player, to);
                    if(view != MAPOBJECTVIEW_VISIBLE) return;

                    game->commandPoint = to;
                    Game_RenderUI(game);
                    Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->commandPoint.x, game->map->renderOffset.y + game->commandPoint.y });
                    Console_Refresh(game->console);
                    return;
                }
            }

            // Command: Look

            if(game->commandActive == COMMAND_LOOK)
            {
                if(direction.x == 0 && direction.y == 0) return;
                
                Point2D to = (Point2D){ game->commandPoint.x + direction.x, game->commandPoint.y + direction.y };

                int view = Map_GetObjectView(game->map, game->map->player, to);
                if(view != MAPOBJECTVIEW_VISIBLE) return;

                game->commandPoint = to;
                char *desc = Map_GetPointDescription(game->map, to);
                Game_LogChangeF(game, CONSOLECOLORPAIR_YELLOWBLACK, 0, "Look at what? (direction / esc.) - %s", desc);

                Game_RenderUI(game);
                Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->commandPoint.x, game->map->renderOffset.y + game->commandPoint.y });
                Console_Refresh(game->console);
                return;
            }

            // Command: Open / Close

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
                    Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
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
                Game_MapNextTurn(game, game->map);
            }

            // Command: Wear / Wield

            if(game->commandActive == COMMAND_WEARWIELD)
            {
                if(game->key < 'a' || game->key > 'j') return;

                int index = game->key - 'a';
                if(index >= game->map->player->itemsCount) return;

                MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_EQUIPUNEQUIP);
                action->object = game->map->player;
                action->targetItem = game->map->player->items[index];
            
                action = Map_AttemptObjectAction(game->map, action);

                if(!action->result)
                {
                    Game_Log(game, action->resultMessage, CONSOLECOLORPAIR_WHITEBLACK, 0);
                    Game_RenderUI(game);
                    Console_Refresh(game->console);
                    MapObjectAction_Destroy(action);
                    return;
                }

                Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, action->resultMessage, action->targetItem->name);

                MapObjectAction_Destroy(action);
                game->refreshMap = true;
                game->uiInventoryOpen = false;
                game->commandActive = -1;
                Console_Clear(game->console);
                Console_SetCursor(game->console, 1);
                Game_MapNextTurn(game, game->map);
            }
        }
        else
        {
            // 'i' == Inventory

            if(game->key == 105 || (game->key == 27 && game->uiInventoryOpen))
            {
                game->uiInventoryOpen = !game->uiInventoryOpen;

                Console_Clear(game->console);

                if(!game->uiInventoryOpen)
                    Map_Render(game->map, game->map->player, game->console);

                Game_RenderUI(game);

                if(game->uiInventoryOpen)
                    Console_SetCursor(game->console, 0);
                else
                    Console_SetCursor(game->console, 1);

                Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
                Console_Refresh(game->console);
                return;
            }

            if(game->uiInventoryOpen) return;

            // 'd' == Drop

            if(game->key == 100)
            {
                game->commandActive = COMMAND_DROP;
                game->uiInventoryOpen = true;

                Game_Log(game, "Drop which item? (a - j)", CONSOLECOLORPAIR_YELLOWBLACK, 0);

                Console_Clear(game->console);
                Game_RenderUI(game);
                Console_SetCursor(game->console, 0);
                Console_Refresh(game->console);
                return;
            }

            // 'f' == Fire

            if(game->key == 102)
            {
                MapObjectAsItem *weapon = game->map->player->equipment[MAPOBJECTEQUIPAT_WEAPON];
                
                if(weapon == NULL)
                {
                    Game_Log(game, "You need to wield a ranged weapon.", CONSOLECOLORPAIR_WHITEBLACK, 0);
                    Game_RenderUI(game);
                    Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
                    Console_Refresh(game->console);
                    return;
                }

                if(!(weapon->flags & MAPOBJECTFLAG_ITEMISRANGED))
                {
                    Game_Log(game, "You need to wield a ranged weapon.", CONSOLECOLORPAIR_WHITEBLACK, 0);
                    Game_RenderUI(game);
                    Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
                    Console_Refresh(game->console);
                    return;
                }

                if(weapon->consumesItemWhenUsedID != -1)
                {
                    MapObjectAsItem *ammo = MapObject_GetItemByID(game->map->player, weapon->consumesItemWhenUsedID);
                    if(ammo == NULL)
                    {
                        Game_Log(game, "Your weapon needs ammo to fire.", CONSOLECOLORPAIR_WHITEBLACK, 0);
                        Game_RenderUI(game);
                        Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
                        Console_Refresh(game->console);
                        return;
                    }
                }

                game->commandActive = COMMAND_FIRE;
                game->commandPoint = (Point2D){ game->map->player->position.x, game->map->player->position.y };

                MapObject *closest = Map_GetClosestObjectWithFlags(game->map, game->map->player->position, MAPOBJECTFLAG_ISHOSTILE);
                if(closest != NULL)
                {
                    if(Map_GetObjectView(game->map, game->map->player, closest->position) == MAPOBJECTVIEW_VISIBLE)
                        game->commandPoint = (Point2D){ closest->position.x, closest->position.y };
                }

                Game_Log(game, "Fire at what? (direction, enter, esc.)", CONSOLECOLORPAIR_YELLOWBLACK, 0);
                Game_RenderUI(game);
                Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->commandPoint.x, game->map->renderOffset.y + game->commandPoint.y });
                Console_Refresh(game->console);
                return;
            }

            // 'g' / ',' == Get

            if(game->key == 44 || game->key == 103)
            {
                MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_PICKUP);
                action->object = game->map->player;
                action->to = game->map->player->position;
                action = Map_AttemptObjectAction(game->map, action);

                if(!action->result)
                {
                    Game_Log(game, action->resultMessage, CONSOLECOLORPAIR_WHITEBLACK, 0);
                    MapObjectAction_Destroy(action);

                    Game_RenderUI(game);
                    Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
                    Console_Refresh(game->console);
                    return;
                }

                Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "You pick up the %s.", action->targetItem->name);
                MapObjectAction_Destroy(action);
                Game_MapNextTurn(game, game->map);
            }

            // 'o' == Open / Close

            if(game->key == 111)
            {
                game->commandActive = COMMAND_OPENCLOSE;
                Game_Log(game, "Open / close in which direction?", CONSOLECOLORPAIR_YELLOWBLACK, 0);

                Game_RenderUI(game);
                Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->map->player->position.x, game->map->renderOffset.y + game->map->player->position.y });
                Console_Refresh(game->console);
                return;
            }

            // 'w' == Wear / wield

            if(game->key == 119)
            {
                game->commandActive = COMMAND_WEARWIELD;
                game->uiInventoryOpen = true;

                Game_Log(game, "Wear / wield / remove / put away which item? (a - j)", CONSOLECOLORPAIR_YELLOWBLACK, 0);

                Console_Clear(game->console);
                Game_RenderUI(game);
                Console_SetCursor(game->console, 0);
                Console_Refresh(game->console);
                return;
            }
        
            // 'x' == Look

            if(game->key == 120)
            {
                game->commandActive = COMMAND_LOOK;
                Game_Log(game, "Look at what? (direction, esc.) - Yourself.", CONSOLECOLORPAIR_YELLOWBLACK, 0);
                game->commandPoint = (Point2D){ game->map->player->position.x, game->map->player->position.y };

                Game_RenderUI(game);
                Console_SetCursor(game->console, 2);
                Console_MoveCursor(game->console, (Point2D){ game->map->renderOffset.x + game->commandPoint.x, game->map->renderOffset.y + game->commandPoint.y });
                Console_Refresh(game->console);
                return;
            }
        
            // '<' == Go up stairs

            if(game->key == 60)
            {
                MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_USESTAIRS);
                action->object = game->map->player;
                action = Map_AttemptObjectAction(game->map, action);

                if(!action->result)
                {
                    MapObjectAction_Destroy(action);
                    return;
                }

                MapObjectAction_Destroy(action);
                Game_Log(game, "You ascend the stairs.", CONSOLECOLORPAIR_WHITEBLACK, 0);
                Console_Clear(game->console);
            }

            // '?' == View help

            if(game->key == '?')
            {
                game->screen = SCREEN_HELP;

                Console_Clear(game->console);

                Console_Write(game->console, 0, 0, " - Try to wait to recover hp before moving on.", 0, 0);
                Console_Write(game->console, 1, 0, " - Levels flood quicker as you progress upwards.", 0, 0);
                Console_Write(game->console, 2, 0, " - You only loose oxygen in deep water.", 0, 0);
                Console_Write(game->console, 3, 0, " - Use scuba gear to give you extra oxygen.", 0, 0);
                Console_Write(game->console, 4, 0, " - Be wary of aquatic enemies once the level begins to flood.", 0, 0);

                Console_Write(game->console, 6, 0, "                   Controls: ", 0, 0);
                Console_Write(game->console, 7, 0, " - Movement:                   vi keys / numpad keys / arrow keys", 0, 0);
                Console_Write(game->console, 8, 0, " - Drop:                       d", 0, 0);
                Console_Write(game->console, 9, 0, " - Fire:                       f", 0, 0);
                Console_Write(game->console, 10, 0, " - Look:                       x", 0, 0);
                Console_Write(game->console, 11, 0, " - Open / close:               o", 0, 0);
                Console_Write(game->console, 12, 0, " - Pick up:                    g / ,", 0, 0);
                Console_Write(game->console, 13, 0, " - View help:                  ?", 0, 0);
                Console_Write(game->console, 14, 0, " - View inventory:             i", 0, 0);
                Console_Write(game->console, 15, 0, " - Wait:                       . / Numpad 5", 0, 0);
                Console_Write(game->console, 16, 0, " - Wield / wear / put away:    w", 0, 0);

                Console_Write(game->console, 19, 0, " PRESS ESC. TO CLOSE THIS SCREEN", 0, 0);
                return;
            }

            #if RELEASE == 0

                // '*' == DEBUG: GO TO NEXT FLOOR

                if(game->key == '*')
                {
                    game->map->level++;
                    MapTile_RemoveObject(Map_GetTile(game->map, game->map->player->position), game->map->player);
                    Map_Generate(game->map);

                    Map_ResetObjectView(game->map, game->map->player);
                    Map_PlaceObject(game->map, game->map->player);
                    Console_Clear(game->console);
                }

            #endif

            // Movement

            if(direction.x != 0 || direction.y != 0)
            {
                MapObject_UpdateAttributesExcludeItemsWithFlags(game->map->player, MAPOBJECTFLAG_ITEMISRANGED);

                MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_MOVE);
                action->direction = direction;
                action->object = game->map->player;

                action = Map_AttemptObjectAction(game->map, action);

                if(action->type == MAPOBJECTACTIONTYPE_ATTACK)
                {
                    if(action->result)
                    {
                        if(action->target == NULL)
                            Game_Log(game, "You kill it!", CONSOLECOLORPAIR_WHITEBLACK, 0);
                        else
                            Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "You hit the %s!", action->target->name);
                    }
                    else
                        Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "You try to hit the %s, but miss!", action->target->name);

                    Game_MapNextTurn(game, game->map);
                }
                else
                {
                    if(action->result)
                    {
                        Game_MapNextTurn(game, game->map);

                        if(action->type == MAPOBJECTACTIONTYPE_OPEN)
                            Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "You open the %s.", action->target->name);
                    }
                }

                MapObjectAction_Destroy(action);
            }
            
            // '.' / Numpad 5 == Wait
            if(game->key == 46 || game->key == 53)
                Game_MapNextTurn(game, game->map);
        }

        Map_UpdateObjectView(game->map, game->map->player);

        Map_Render(game->map, game->map->player, game->console);
        game->refreshMap = false;

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

    if(Map_LevelFloodTimerTick(game->map))
        Game_Log(game, "The ship groans and water rushes upwards!", CONSOLECOLORPAIR_CYANBLACK, 0);

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
    if(mapObject == map->player && mapObject->hp <= 0 && !game->deathMessageLogged)
    {
        game->deathMessageLogged = true;
        Game_Log(game, "YOU DIE! GAME OVER!", CONSOLECOLORPAIR_REDBLACK, A_BOLD);
        return;
    }

    mapObject->turnTicks = mapObject->turnTicksSize;

    if(mapObject->flags & MAPOBJECTFLAG_ISLIVING)
    {
        if(mapObject->hp < mapObject->hpMax && mapObject->hpRecoverTimerLength > 0)
        {
            mapObject->hpRecoverTimer--;
            if(mapObject->hpRecoverTimer <= 0)
            {
                mapObject->hpRecoverTimer += mapObject->hpRecoverTimerLength;
                mapObject->hp++;
            }
        }

        if(!(mapObject->flags & MAPOBJECTFLAG_ISAQUATIC) && mapObject->o2 == 0)
            mapObject->hp--;
        
        if(mapObject->hp <= 0)
        {
            mapObject->hp = 0;
            mapObject->flags &= ~MAPOBJECTFLAG_ISLIVING;
            if(mapObject == map->player)
            {
                if(!game->deathMessageLogged)
                {
                    game->deathMessageLogged = true;
                    if(mapObject->o2 == 0)
                        Game_Log(game, "YOU DROWN! GAME OVER!", CONSOLECOLORPAIR_REDBLACK, A_BOLD);
                }
            }
            else
            {
                MapTile_RemoveObject(Map_GetTile(map, mapObject->position), mapObject);
                MapObject_Destroy(mapObject);
            }
        }

        MapObject_UpdateAttributes(mapObject);
        MapObject_UpdateItems(mapObject);

        if(!(mapObject->flags & MAPOBJECTFLAG_ISAQUATIC))
        {
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
        }

        if(mapObject->o2 < mapObject->o2Max) mapObject->o2++;

        if(mapObject == map->player) return;

        if(mapObject->flags & MAPOBJECTFLAG_ISHOSTILE)
        {
            for(int y = 0; y < map->size.height - 1; y++)
            {
                for(int x = 0; x < map->size.width - 1; x++)
                {
                    int view = Map_GetObjectView(map, mapObject, (Point2D){ x, y });
                    if(view != MAPOBJECTVIEW_VISIBLE) continue;

                    if(map->player->position.x != x || map->player->position.y != y)
                        continue;

                    int distance = Map_GetSimpleDistance(map, mapObject->position, map->player->position);
                    if(distance <= mapObject->attackDistance)
                    {
                        MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_ATTACK);
                        action->object = mapObject;
                        action->target = map->player;
                        Map_AttemptObjectAction(map, action);

                        if(!action->result)
                        {
                            Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "The %s tries to %s you, but misses!", mapObject->name, mapObject->attackVerbs[0]);
                            return;
                        }

                        Game_LogF(game, CONSOLECOLORPAIR_WHITEBLACK, 0, "The %s %s you! (-%d)", mapObject->name, mapObject->attackVerbs[1], action->resultValueInt);
                        
                        MapObjectAction_Destroy(action);
                        return;
                    }

                    Direction2D direction = (Direction2D){ 0, 0 };
                    if(map->player->position.x > mapObject->position.x) direction.x = 1;
                    if(map->player->position.x < mapObject->position.x) direction.x = -1;
                    if(map->player->position.y > mapObject->position.y) direction.y = 1;
                    if(map->player->position.y < mapObject->position.y) direction.y = -1;
                    MapObjectAction *action = MapObjectAction_Create(MAPOBJECTACTIONTYPE_MOVE);
                    action->direction = direction;
                    action->object = mapObject;
                    Map_AttemptObjectAction(map, action);
                    MapObjectAction_Destroy(action);
                }
            }
        }

        return;
    }

    if(mapObject->flags & MAPOBJECTFLAG_ISLIQUID)
    {
        mapObject->description = (mapObject->height < 5) ? "Shallow water." : "Deep water.";
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
    #if RELEASE == 0
        Console_WriteF(game->console, 0, 0, CONSOLECOLORPAIR_BLACKWHITE, 0, "KEY = %d   ", game->key);
    #endif
    
    int max = 5;
    if(game->logSize < 5) max = game->logSize;

    for(int i = 0; i < max; i++)
    {
        LogMessage *lm = game->log[game->logSize - (i + 1)];
        Console_ClearRow(game->console, 20 + i);
        Console_Write(game->console, 20 + i, 23, lm->str, lm->colorPair, lm->attributes);
    }

    Console_Write(game->console, 20, 0, game->map->player->name, CONSOLECOLORPAIR_WHITEBLACK, A_BOLD);
    
    Console_Write(game->console, 21, 0, "HP: [               ]", CONSOLECOLORPAIR_WHITEBLACK, 0);
    Console_Write(game->console, 22, 0, "O2: [               ]", CONSOLECOLORPAIR_WHITEBLACK, 0);
    Console_DrawBarW(game->console, 21, 5, 15, game->map->player->hp, game->map->player->hpMax, CONSOLECOLORPAIR_REDBLACK, 0);
    Console_DrawBarW(game->console, 22, 5, 15, game->map->player->o2, game->map->player->o2Max, CONSOLECOLORPAIR_CYANBLACK, 0);
    Console_WriteF(game->console, 23, 0, CONSOLECOLORPAIR_WHITEBLACK, 0, "ATT: %d (+%d)  DEF: %d   ", game->map->player->attack, game->map->player->attackToHit, game->map->player->defense);
    Console_WriteF(game->console, 24, 0, CONSOLECOLORPAIR_WHITEBLACK, 0, "FLOOR: %d  T: %d   ", game->map->level, game->turn);

    if(game->uiInventoryOpen)
    {
        Rect2D rect;
        rect.position = (Point2D){ 0, 0 };
        rect.size = (Size2D){ 80, 15 };
        wchar_t wchrs[6] = { L'═', L'║', L'╔', L'╗', L'╝', L'╚' };
        Console_DrawRect(game->console, rect, wchrs, CONSOLECOLORPAIR_WHITEBLACK, 0);

        Console_Write(game->console, 1, 2, "Inventory: ", CONSOLECOLORPAIR_BLACKWHITE, 0);

        for(int i = 0; i < 10; i++)
        {
            char *str = "...";
            char *strDetails = " ";
            char *strExtra = " ";
            int colorPair = CONSOLECOLORPAIR_WHITEBLACK;
            if(i < game->map->player->itemsCount)
            {
                MapObjectAsItem *item = game->map->player->items[i];
                str = item->description;
                strDetails = item->details;
                
                int equippedAt = MapObject_GetEquippedAt(game->map->player, item);
                if(equippedAt > -1)
                {
                    strExtra = (equippedAt == MAPOBJECTEQUIPAT_WEAPON) ? "(ready)" : "(wearing)";
                    colorPair = CONSOLECOLORPAIR_YELLOWBLACK;
                }

                Console_WriteF(game->console, 3 + i, 3, colorPair, 0, "%c. %s", 'a' + i, str);
                int x = 28;
                if(item->flags & MAPOBJECTFLAG_ITEMSUPPLY02)
                    Console_WriteF(game->console, 3 + i, 3 + x, colorPair, 0, strDetails, item->o2);
                else
                    Console_Write(game->console, 3 + i, 3 + x, strDetails, colorPair, 0);
                x += 30;
                Console_Write(game->console, 3 + i, 3 + x, strExtra, colorPair, 0);
            }
            else
                Console_WriteF(game->console, 3 + i, 3, colorPair, 0, "%c. ...", 'a' + i);
        }
    }
}