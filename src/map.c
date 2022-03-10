#include "include/map.h"

MapObjectAction *Map_AttemptObjectAction(Map *map, MapObjectAction *action)
{
    if(action->type == MAPOBJECTACTIONTYPE_DROP)
    {
        if(action->targetItem == NULL) return action;

        int equipAt = MapObject_GetEquippedAt(action->object, action->targetItem);
        if(equipAt > -1)
        {
            action->type = MAPOBJECTACTIONTYPE_EQUIPUNEQUIP;
            action = Map_AttemptObjectAction(map, action);

            if(!action->result)
            {
                action->type = MAPOBJECTACTIONTYPE_DROP;
                return action;
            }
        }

        for(int y = -1; y < 2; y++)
        {
            for(int x = -1; x < 2; x++)
            {
                MapTile *tile = Map_GetTile(map, (Point2D){ action->object->position.x + x, action->object->position.y + y });
                if(tile == NULL) continue;
                if(!(tile->passable & MAPTILEPASSABLE_SOLID)) continue;
                if(tile->objectsCount == 10) continue;

                MapObject *mapObject = Map_CreateObject(map, action->targetItem->id);
                MapTile_AddObject(tile, mapObject);
                MapObject_RemoveItemFromItems(action->object, action->targetItem);
                MapObjectAsItem_Destroy(action->targetItem);

                action->resultMessage = "You drop it.";
                action->result = true;
                return action;
            }
        }

        action->resultMessage = "There's no place to drop it.";
        return action;
    }

    if(action->type == MAPOBJECTACTIONTYPE_EQUIPUNEQUIP)
    {
        if(action->targetItem == NULL) return action;

        if(!(action->targetItem->flags & MAPOBJECTFLAG_ISEQUIPMENT))
        {
            action->resultMessage = "That cannot be equipped.";
            return action;
        }

        if(action->object->equipment[action->targetItem->equipAt] == action->targetItem)
        {
            action->object->equipment[action->targetItem->equipAt] = NULL;
            action->resultMessage = "You put away the %s.";
            action->result = true;
            return action;
        }

        if(action->object->equipment[action->targetItem->equipAt] != NULL)
        {
            //...
        }

        action->object->equipment[action->targetItem->equipAt] = action->targetItem;
        action->resultMessage = "You ready the %s.";
        action->result = true;
        return action;
    }

    if(action->type == MAPOBJECTACTIONTYPE_MOVE)
    {
        Point2D to = (Point2D){ action->object->position.x + action->direction.x, action->object->position.y + action->direction.y };

        MapTile *tile = Map_GetTile(map, to);
        if(tile == NULL)
        {
            action->resultMessage = "Point not on map.";
            return action;
        }

        if(!(tile->passable & MAPTILEPASSABLE_SOLID))
        {
            if(tile->objectsCount > 0)
            {
                for(int i = 0; i < tile->objectsCount; i++)
                {
                    if(!(tile->objects[i]->flags & MAPOBJECTFLAG_CANOPEN)) continue;
                    
                    action->type = MAPOBJECTACTIONTYPE_OPEN;
                    action->target = tile->objects[i];
                    action->to = action->target->position;
                    return Map_AttemptObjectAction(map, action);
                }
            }

            action->resultMessage = "Tile impassable.";
            return action;
        }

        action->object->lastRoomIndex = Map_GetRoomIndexContaining(map, action->object->position);
        action->result = true;
        Map_MoveObject(map, action->object, to);

        return action;
    }

    if(action->type == MAPOBJECTACTIONTYPE_OPEN)
    {
        action = Map_ObjectAttemptActionAsTarget(map, action->target, action);
        return action;
    }

    if(action->type == MAPOBJECTACTIONTYPE_PICKUP)
    {
        MapTile *tile = Map_GetTile(map, action->object->position);
        if(tile->objectsCount == 0) return action;

        MapObject *target = MapTile_GetObjectWithFlags(tile, MAPOBJECTFLAG_ISITEM);
        if(target == NULL)
        {
            action->resultMessage = "There's nothing to pick up.";
            return action;
        }

        action->target = target;
        if(action->object->itemsCount == 10)
        {
            action->resultMessage = "Inventory is full.";
            return action;
        }

        action->result = true;

        MapObjectAsItem *item = MapObject_ToItem(action->target);
        MapTile_RemoveObject(tile, action->target);
        Map_DestroyObject(map, action->target);
        action->target = NULL;
        action->targetItem = item;

        action->object->items[action->object->itemsCount] = item;
        action->object->itemsCount++;

        return action;
    }

    if(action->type == MAPOBJECTACTIONTYPE_USESTAIRS)
    {
        MapTile *tile = Map_GetTile(map, action->object->position);
        if(tile->objectsCount == 0) return action;

        MapObject *stairs = MapTile_GetObjectWithFlags(tile, MAPOBJECTFLAG_STAIRS);
        if(stairs == NULL) return action;

        action->result = true;
        if(action->object != map->player) return action;

        map->level++;
        MapTile_RemoveObject(tile, map->player);
        Map_Clear(map);
        Map_Generate(map);

        Map_ResetObjectView(map, map->player);
        Map_PlaceObject(map, map->player);

        return action;
    }

    return action;
}

void Map_Clear(Map *map)
{
    for(int y = 0; y < map->size.height; y++)
    {
        for(int x = 0; x < map->size.width; x++)
        {
            MapTile *tile = Map_GetTile(map, (Point2D){ x, y });
            MapTile_DestroyObjects(tile, map);
        }
    }
}

void Map_ClearExcludePlayer(Map *map)
{
    for(int y = 0; y < map->size.height; y++)
    {
        for(int x = 0; x < map->size.width; x++)
        {
            MapTile *tile = Map_GetTile(map, (Point2D){ x, y });
            MapTile_DestroyObjectsExcludePlayer(tile, map);
        }
    }
}

Map *Map_Create(Size2D size, Point2D renderOffset)
{
    Map *map = malloc(sizeof(Map));

    map->level = 1;
    map->levelFloodTimer = 999;
    map->levelFloodTimerSize = 999;
    map->player = NULL;
    map->size = size;
    map->renderOffset = renderOffset;

    map->tiles = malloc(sizeof(MapTile) * (map->size.width * map->size.height));
    for(int i = 0; i < (map->size.width * map->size.height); i++)
    {
        map->tiles[i] = malloc(sizeof(MapTile));
        map->tiles[i]->objectsCount = 0;
        map->tiles[i]->type == MAPTILETYPE_EMPTY;
    }

    return map;
}

MapObject *Map_CreateObject(Map *map, uint16_t id)
{
    MapObject *mapObject = MapObject_Create("none");
    mapObject->id = id;
    mapObject->view = NULL;
    bool hasView = false;

    if(id == MAPOBJECTID_PLAYER) // Player
    {
        mapObject->attackBase = 1;
        mapObject->attackToHitBase = 1;
        mapObject->defenseBase = 3;
        mapObject->description = "Yourself.";
        mapObject->layer = 1;
        mapObject->name = "Player";
        mapObject->flags |= (MAPOBJECTFLAG_CANMOVE | MAPOBJECTFLAG_ISLIVING | MAPOBJECTFLAG_PLACEINROOM | MAPOBJECTFLAG_PLAYER);
        mapObject->hp = 10;
        mapObject->hpMax = 10;
        mapObject->o2 = 20;
        mapObject->o2Max = 20;
        mapObject->turnTicks = 10;
        mapObject->turnTicksSize = 10;
        mapObject->wchr = L'@';
        mapObject->wchrAlt = L'@';
        hasView = true;
    }

    if(id == MAPOBJECTID_DOOR) // Door
    {
        mapObject->description = "A door.";
        mapObject->layer = 2;
        mapObject->name = "door";
        mapObject->flags |= (MAPOBJECTFLAG_CANOPEN | MAPOBJECTFLAG_BLOCKSGAS | MAPOBJECTFLAG_BLOCKSLIGHT | MAPOBJECTFLAG_BLOCKSLIQUID | MAPOBJECTFLAG_BLOCKSSOLID | MAPOBJECTFLAG_PLACEINDOORWAYS);
        mapObject->wchr = L'+';
        mapObject->wchrAlt = L'`';
    }

    if(id == MAPOBJECTID_DIVEKNIFE) // Dive knife
    {
        mapObject->attack = 3;
        mapObject->attackToHit = 1;
        mapObject->description = "A dive knife.";
        mapObject->equipAt = MAPOBJECTEQUIPAT_WEAPON;
        mapObject->layer = 2;
        mapObject->name = "dive knife";
        mapObject->flags |= (MAPOBJECTFLAG_ISEQUIPMENT | MAPOBJECTFLAG_ISITEM);
        mapObject->wchr = L'/';
        mapObject->wchrAlt = L'/';
    }

    if(id == MAPOBJECTID_WATER) // Water
    {
        mapObject->description = "Water.";
        mapObject->colorPair = CONSOLECOLORPAIR_CYANBLACK;
        mapObject->layer = 3;
        mapObject->name = "water";
        mapObject->flags |= MAPOBJECTFLAG_ISLIQUID;
        mapObject->turnTicks = 2;
        mapObject->turnTicksSize = 2;
        mapObject->wchr = L'~';
        mapObject->wchrAlt = L'≈';
    }

    if(id == MAPOBJECTID_WATERSOURCE) // Water source
    {
        mapObject->description = "A hole in the ship.";
        mapObject->colorPair = CONSOLECOLORPAIR_CYANBLACK;
        mapObject->layer = 4;
        mapObject->name = "water source";
        mapObject->flags |= MAPOBJECTFLAG_ISLIQUIDSOURCE;
        mapObject->turnTicks = 5;
        mapObject->turnTicksSize = 5;
        mapObject->wchr = L'○';
        mapObject->wchrAlt = L'○';
    }

    if(id == MAPOBJECTID_STAIRS) // Stairs
    {
        mapObject->description = "Stairs.";
        mapObject->layer = 2;
        mapObject->name = "stairs";
        mapObject->flags |= (MAPOBJECTFLAG_PLACEINROOM | MAPOBJECTFLAG_STAIRS);
        mapObject->wchr = L'<';
        mapObject->wchrAlt = L'<';
    }

    if(hasView)
    {
        mapObject->view = malloc(sizeof(int) * (map->size.width * map->size.height));

        for(int i = 0; i < map->size.width * map->size.height; i++)
            mapObject->view[i] = MAPOBJECTVIEW_UNSEEN;
    }

    MapObject_UpdateAttributes(mapObject);

    return mapObject;
}

void Map_Destroy(Map *map)
{
    for(int i = 0; i < map->size.width * map->size.height; i++)
        MapTile_Destroy(map->tiles[i], map);
    free(map->tiles);
}

void Map_DestroyObject(Map* map, MapObject *mapObject)
{
    for(int i = 0; i < mapObject->itemsCount; i++)
        Map_DestroyObject(map, mapObject->items[i]);
    if(mapObject->view != NULL)
        free(mapObject->view);
    free(mapObject);
}

void Map_Generate(Map *map)
{
    map->levelFloodTimerSize = (120 + rand() % 10) - (map->level * 5);
    if(map->levelFloodTimerSize < 50) map->levelFloodTimerSize = 50;
    map->levelFloodTimer = (int)map->levelFloodTimerSize;

    while(true)
    {
        for(int y = 0; y < map->size.height; y++)
        {
            for(int x = 0; x < map->size.width; x++)
            {
                MapTile *tile = map->tiles[(y * map->size.width) + x];
                MapTile_SetType(tile, MAPTILETYPE_WALL);
                MapTile_DestroyObjects(tile, map);
            }
        }

        while(true)
        {
            for(int i = 0; i < map->roomsCount; i++)
                free(map->rooms[i]);
            map->roomsCount = 0;

            Point2D divisions = (Point2D){ 3, 2 };

            for(int y = 1; y < map->size.height - 1; y += map->size.height / divisions.y)
            {
                for(int x = 1; x < map->size.width - 1; x += map->size.width / divisions.x)
                {
                    if(rand() % 100 >= 80) continue;

                    map->rooms[map->roomsCount] = malloc(sizeof(Rect2D));
                    map->rooms[map->roomsCount]->position = (Point2D){ x, y };
                    map->rooms[map->roomsCount]->size = (Size2D){ (map->size.width / divisions.x) - 1, (map->size.height / divisions.y) - 1 };
                    map->roomsCount++;
                }
            }

            if(map->roomsCount >= 4 && map->roomsCount < 6) break;
        }

        for(int i = 0; i < map->roomsCount; i++)
        {
            Point2D offset = (Point2D){ 1 + rand() % 3, 1 + rand() % 3 };
            map->rooms[i]->position.x += offset.x;
            map->rooms[i]->position.y += offset.y;
            map->rooms[i]->size.width -= ((offset.x * 2) + rand() % 2);
            map->rooms[i]->size.height -= ((offset.y * 2) + rand() % 2);

            if(rand() % 10 >= 2)
            {
                map->rooms[i]->size.width /= 1.5;
                map->rooms[i]->size.height /= 1.3;
            }
            else
            {
                if(rand() % 10 >= 8)
                {
                    map->rooms[i]->size.width = 3 + rand() % 2;
                    map->rooms[i]->size.height = 3 + rand() % 2;
                }
            }

            while(map->rooms[i]->position.y + map->rooms[i]->size.width > map->size.width - 2)
                map->rooms[i]->size.width--;
            while(map->rooms[i]->position.y + map->rooms[i]->size.height > map->size.height - 2)
                map->rooms[i]->size.height--;
            if(map->rooms[i]->size.width < 3) map->rooms[i]->size.width = 3;
            if(map->rooms[i]->size.height < 3) map->rooms[i]->size.height = 3;
        }

        bool toContinue = false;
        for(int i = 0; i < map->roomsCount; i++)
        {
            Rect2D *room = map->rooms[i];
            if(room->position.x < 3 || room->position.y < 3 || room->position.x + room->size.width >= map->size.width - 3 || room->position.y + room->size.height >= map->size.height - 3)
            {
                toContinue = true;
                break;
            }
        }

        if(!toContinue) break;
    }

    for(int i = 0; i < map->roomsCount; i++)
    {
        for(int y = 0; y < map->rooms[i]->size.height; y++)
        {
            for(int x = 0; x < map->rooms[i]->size.width; x++)
            {
                Point2D point = (Point2D){ map->rooms[i]->position.x + x, map->rooms[i]->position.y + y };
                if(point.x < 1 || point.y < 1 || point.x >= map->size.width - 1 || point.y >= map->size.height - 1) 
                    continue;
                MapTile_SetType(Map_GetTile(map, point), MAPTILETYPE_FLOOR);
            }
        }
    }

    bool roomConnected[25];
    for(int i = 0; i < map->roomsCount; i++) roomConnected[i] = false;
    int timeout = 1000;

    while(timeout > 0)
    {
        timeout--;

        int atRoomIndex = rand() % map->roomsCount;
        while(roomConnected[atRoomIndex])
        {
            atRoomIndex++;
            if(atRoomIndex >= map->roomsCount) atRoomIndex = 0;
        }
        int toRoomIndex = rand() % map->roomsCount;
        while(atRoomIndex == toRoomIndex) toRoomIndex = rand() % map->roomsCount;
        Point2D at, to;

        roomConnected[atRoomIndex] = true;
        at = (Point2D){ map->rooms[atRoomIndex]->position.x + map->rooms[atRoomIndex]->size.width / 2, map->rooms[atRoomIndex]->position.y + map->rooms[atRoomIndex]->size.height / 2 };
        to = (Point2D){ map->rooms[toRoomIndex]->position.x + map->rooms[toRoomIndex]->size.width / 2, map->rooms[toRoomIndex]->position.y + map->rooms[toRoomIndex]->size.height / 2 };

        while(at.x != to.x || at.y != to.y)
        {
            if(Map_GetRoomIndexContaining(map, at) == toRoomIndex && map->tiles[(at.y * map->size.width) + at.x]->type == MAPTILETYPE_FLOOR)
                break;

            if(at.x > 0 && at.y > 0 && at.x < map->size.width - 1 && at.y < map->size.height - 1)
                MapTile_SetType(map->tiles[(at.y * map->size.width) + at.x], MAPTILETYPE_FLOOR);

            if(rand() % 10 >= 8)
            {
                if(rand() % 10 >= 3)
                    at.x += -1 + rand() % 2;
                else
                    at.y += -1 + rand() % 2;
            }
            else
            {
                if(to.x > at.x) at.x++;
                if(to.x < at.x) at.x--;
                if(to.y > at.y) at.y++;
                if(to.y < at.y) at.y--;
            }
        }

        bool toBreak = true;
        for(int i = 0; i < map->roomsCount; i++)
        {
            if(!roomConnected[i])
            {
                toBreak = false;
                break;
            }
        }

        if(toBreak) break;
    }

    map->roomDoorwaysCount = 0;

    for(int i = 0; i < map->roomsCount; i++)
    {
        Rect2D *room = map->rooms[i];

        for(int y = -1; y <= (int)room->size.height; y++)
        {
            for(int x = -1; x <= (int)room->size.width; x++)
            {
                Point2D point = (Point2D){ room->position.x + x, room->position.y + y };

                if(point.x < 1 || point.y < 1 || point.x >= map->size.width - 1 || point.y >= map->size.height - 1) 
                    continue;

                if(Map_GetTile(map, point)->type != MAPTILETYPE_FLOOR) 
                    continue;

                if(!((Map_GetTile(map, (Point2D){ point.x - 1, point.y })->type == MAPTILETYPE_WALL && Map_GetTile(map, (Point2D){ point.x + 1, point.y })->type == MAPTILETYPE_WALL) || (Map_GetTile(map, (Point2D){ point.x, point.y - 1 })->type == MAPTILETYPE_WALL && Map_GetTile(map, (Point2D){ point.x, point.y + 1 })->type == MAPTILETYPE_WALL)))
                    continue;

                map->roomDoorways[map->roomDoorwaysCount].x = point.x;
                map->roomDoorways[map->roomDoorwaysCount].y = point.y;
                map->roomDoorwaysCount++;
            }
        }
    }

    if(map->player == NULL)
    {
        map->player = Map_CreateObject(map, MAPOBJECTID_PLAYER);
        Map_PlaceObject(map, map->player);
    }

    for(int i = 0; i < 4 + rand() % 2; i++)
        Map_PlaceObject(map, Map_CreateObject(map, MAPOBJECTID_DOOR));

    Map_PlaceObject(map, Map_CreateObject(map, MAPOBJECTID_STAIRS));

    int itemIDs[10];
    itemIDs[0] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[1] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[2] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[3] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[4] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[5] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[6] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[7] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[8] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[9] = MAPOBJECTID_DIVEKNIFE;

    for(int i = 0; i < 3 + rand() % 2; i++)
        Map_PlaceObject(map, Map_CreateObject(map, itemIDs[rand() % 10]));
}

int Map_GetObjectView(Map *map, MapObject *mapObject, Point2D point)
{
    if(mapObject->view == NULL) return MAPOBJECTVIEW_UNSEEN;
    if(point.x < 0 || point.y < 0 || point.x >= map->size.width || point.y >= map->size.height)
        return MAPOBJECTVIEW_UNSEEN;

    return mapObject->view[(point.y * map->size.width) + point.x];
}

int Map_GetPointColorPair(Map *map, Point2D point)
{
    MapTile *tile = Map_GetTile(map, point);
    if(tile == NULL) return CONSOLECOLORPAIR_WHITEBLACK;

    if(tile->objectsCount > 0)
    {
        int layer = tile->objects[0]->layer;
        int colorPair = tile->objects[0]->colorPair;
        for(int i = 1; i < tile->objectsCount; i++)
        {
            if(tile->objects[i]->layer >= layer) continue;
            layer = tile->objects[i]->layer;
            colorPair = tile->objects[i]->colorPair;
        }

        return colorPair;
    }
    
    return CONSOLECOLORPAIR_WHITEBLACK;
}

char *Map_GetPointDescription(Map *map, Point2D point)
{
    MapTile *tile = Map_GetTile(map, point);
    if(tile == NULL) return "Nothing";

    if(tile->objectsCount > 0)
    {
        int layer = tile->objects[0]->layer;
        char *desc = tile->objects[0]->description;
        for(int i = 1; i < tile->objectsCount; i++)
        {
            if(tile->objects[i]->layer >= layer) continue;
            layer = tile->objects[i]->layer;
            desc = tile->objects[i]->description;
        }

        return desc;
    }

    if(tile->type == MAPTILETYPE_FLOOR)
        return "The floor.";
    if(tile->type == MAPTILETYPE_WALL)
        return "The wall.";

    return "Nothing";
}

wchar_t Map_GetPointWChr(Map *map, Point2D point)
{
    MapTile *tile = Map_GetTile(map, point);
    if(tile == NULL) return L' ';

    if(tile->objectsCount > 0)
    {
        int layer = tile->objects[0]->layer;
        wchar_t wchr = tile->objects[0]->wchr;
        for(int i = 1; i < tile->objectsCount; i++)
        {
            if(tile->objects[i]->layer >= layer) continue;
            layer = tile->objects[i]->layer;
            wchr = tile->objects[i]->wchr;
        }

        return wchr;
    }
    
    //░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀

    if(tile->type == MAPTILETYPE_FLOOR)
        return (Map_GetRoomIndexContaining(map, (Point2D){ point.x, point.y }) > -1) ? L'.' : L'▒';
    if(tile->type == MAPTILETYPE_WALL)
    {
        wchar_t wchr = L' ';

        if(map->tiles[((point.y + 1) * map->size.width) + (point.x + 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x + 1, point.y + 1 }) > -1)
            wchr = L'┌';
        if(map->tiles[((point.y + 1) * map->size.width) + (point.x - 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x - 1, point.y + 1 }) > -1)
            wchr = L'┐';
        if(map->tiles[((point.y - 1) * map->size.width) + (point.x + 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x + 1, point.y - 1 }) > -1)
            wchr = L'└';
        if(map->tiles[((point.y - 1) * map->size.width) + (point.x - 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x - 1, point.y - 1 }) > -1)
            wchr = L'┘';
        if((map->tiles[(point.y * map->size.width) + (point.x - 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x - 1, point.y }) > -1) || (map->tiles[(point.y * map->size.width) + (point.x + 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x + 1, point.y }) > -1))
            wchr = L'│';
        if((map->tiles[((point.y - 1) * map->size.width) + point.x]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x , point.y - 1 }) > -1) || (map->tiles[((point.y + 1) * map->size.width) + point.x]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x , point.y + 1 }) > -1))
            wchr = L'─';

        return wchr;
    }

    return L' ';
}

int Map_GetRoomIndexContaining(Map *map, Point2D point)
{
    for(int i = 0; i < map->roomsCount; i++)
    {
        if(point.x < map->rooms[i]->position.x || point.y < map->rooms[i]->position.y || point.x >= map->rooms[i]->position.x + map->rooms[i]->size.width || point.y >= map->rooms[i]->position.y + map->rooms[i]->size.height)
            continue;
        return i;
    }

    return -1;
}

MapTile *Map_GetTile(Map *map, Point2D point)
{
    if(point.x < 0 || point.y < 0 || point.x >= map->size.width || point.y >= map->size.height)
        return NULL;

    return map->tiles[(point.y * map->size.width) + point.x];
}

void Map_MoveObject(Map *map, MapObject *mapObject, Point2D to)
{
    MapTile *tileFrom = Map_GetTile(map, mapObject->position);
    MapTile *tileTo = Map_GetTile(map, to);
    if(tileFrom == NULL || tileTo == NULL) return;

    MapTile_RemoveObject(tileFrom, mapObject);
    mapObject->position = to;
    MapTile_AddObject(tileTo, mapObject);
}

MapObjectAction *Map_ObjectAttemptActionAsTarget(Map *map, MapObject *mapObject, MapObjectAction *action)
{
    if(action->target != mapObject) return action;

    if(action->type == MAPOBJECTACTIONTYPE_OPEN)
    {
        if(!(mapObject->flags & MAPOBJECTFLAG_CANOPEN))
        {
            action->resultMessage = "Cannot be opened.";
            return action;
        }

        mapObject->flags ^= MAPOBJECTFLAG_ISOPEN;
        wchar_t wchr = mapObject->wchr;
        mapObject->wchr = mapObject->wchrAlt;
        mapObject->wchrAlt = wchr;

        MapTile_UpdatePassable(Map_GetTile(map, mapObject->position));

        action->result = true;
        return action;
    }

    return action;
}

void Map_PlaceObject(Map *map, MapObject *mapObject)
{
    int timeout = 1000;

    while(timeout > 0)
    {
        timeout--;
        
        Rect2D *room = map->rooms[rand() % map->roomsCount];

        Point2D point = (Point2D){ -1, -1 };
        MapTile *tile = NULL;

        if(mapObject->flags & MAPOBJECTFLAG_PLACEINDOORWAYS)
        {
            if(map->roomDoorwaysCount == 0) return;

            Point2D roomDoorway = map->roomDoorways[rand() % map->roomDoorwaysCount];
            point = (Point2D){ roomDoorway.x, roomDoorway.y };
            tile = Map_GetTile(map, point);
            if(tile == NULL) continue;

            if(!(tile->passable & MAPTILEPASSABLE_SOLID)) continue;
        }
        else
        {
            point = (Point2D){ room->position.x + 1 + rand() % (room->size.width - 2), room->position.y + 1 + rand() % (room->size.height - 2) };
        
            if(point.x < 1 || point.y < 1 || point.x >= map->size.width - 1 || point.y >= map->size.height - 1) 
                continue;

            tile = Map_GetTile(map, point);
            
            if(!(tile->passable & MAPTILEPASSABLE_SOLID)) continue;
        }

        mapObject->position = point;
        MapTile_AddObject(tile, mapObject);
        Map_UpdateObjectView(map, mapObject);

        return;
    }
}

void Map_Render(Map *map, MapObject *viewer, Console *console)
{
    Rect2D rect;
    rect.position = (Point2D){ 0, 0 };
    rect.size = (Size2D){ map->size.width, map->size.height };

    Map_RenderRect(map, viewer, console, rect);
}

void Map_RenderForPlayer(Map *map, Console *console)
{
    if(map->player->lastRoomIndex > -1)
    {
        Rect2D *room = map->rooms[map->player->lastRoomIndex];
        Rect2D rectRoom;
        rectRoom.position = (Point2D){ room->position.x - 1, room->position.y - 1 };
        rectRoom.size = (Size2D){ room->size.width + 2, room->size.height + 2 };

        Map_RenderRect(map, map->player, console, rectRoom);
    }

    int ric = Map_GetRoomIndexContaining(map, map->player->position);
    if(ric > -1)
    {
        Rect2D *room = map->rooms[ric];
        Rect2D rectRoom;
        rectRoom.position = (Point2D){ room->position.x - 1, room->position.y - 1 };
        rectRoom.size = (Size2D){ room->size.width + 2, room->size.height + 2 };

        Map_RenderRect(map, map->player, console, rectRoom);
        return;
    }

    Rect2D rectPlayer;
    rectPlayer.position = (Point2D){ map->player->position.x - 5, map->player->position.y - 5 };
    rectPlayer.size = (Size2D){ 10, 10 };

    Map_RenderRect(map, map->player, console, rectPlayer);
}

void Map_RenderRect(Map *map, MapObject *viewer, Console *console, Rect2D rect)
{
    for(int y = 0; y < map->size.height; y++)
    {
        for(int x = 0; x < map->size.width; x++)
        {
            if(x < rect.position.x || y < rect.position.y || x >= rect.position.x + rect.size.width || y >= rect.position.y + rect.size.height)
                continue;

            Point2D mapPoint = (Point2D){ x, y };
            MapTile *tile = Map_GetTile(map, mapPoint);

            int colorPair = CONSOLECOLORPAIR_WHITEBLACK;
            wchar_t wchr = L' ';

            if(x > 0 && y > 0 && x < map->size.width - 1 && y < map->size.height - 1)
            {
                int view = viewer->view[(y * map->size.width) + x];
                
                if(view == MAPOBJECTVIEW_VISIBLE)
                {
                    colorPair = Map_GetPointColorPair(map, mapPoint);
                    wchr = Map_GetPointWChr(map, mapPoint);
                }
                else
                {
                    if(view == MAPOBJECTVIEW_SEEN)
                    {
                        if(tile->type == MAPTILETYPE_FLOOR)
                            wchr = (Map_GetRoomIndexContaining(map, mapPoint) > -1) ? L' ' : L'#';
                        if(tile->type == MAPTILETYPE_WALL)
                            wchr = Map_GetPointWChr(map, mapPoint);
                    }
                }
            }

            Point2D renderPoint = (Point2D){ map->renderOffset.x + x, map->renderOffset.y + y };
            Console_SetCharW(console, renderPoint.y, renderPoint.x, wchr, colorPair, 0);
        }
    }
}

void Map_ResetObjectView(Map* map, MapObject *mapObject)
{
    if(!(mapObject->flags & MAPOBJECTFLAG_ISLIVING)) return;

    for(int i = 0; i < map->size.width * map->size.height; i++)
        mapObject->view[i] = MAPOBJECTVIEW_UNSEEN;
}

void Map_UpdateObjectView(Map* map, MapObject *mapObject)
{
    if(!(mapObject->flags & MAPOBJECTFLAG_ISLIVING)) return;

    for(int i = 0; i < map->size.width * map->size.height; i++)
    {
        if(mapObject->view[i] != MAPOBJECTVIEW_VISIBLE)
            continue;

        mapObject->view[i] = MAPOBJECTVIEW_SEEN;
    }

    Rect2D viewRect;
    int ric = Map_GetRoomIndexContaining(map, mapObject->position);

    if(ric > -1)
    {
        viewRect.position = (Point2D){ map->rooms[ric]->position.x - 1, map->rooms[ric]->position.y - 1 };
        viewRect.size = (Size2D){ map->rooms[ric]->size.width + 2, map->rooms[ric]->size.height + 2 };
    }
    else
    {
        viewRect.position.x = mapObject->position.x - 1;
        viewRect.position.y = mapObject->position.y - 1;
        viewRect.size = (Size2D){ 3, 3 };
    }

    for(int y = 0; y < viewRect.size.height; y++)
    {
        for(int x = 0; x < viewRect.size.width; x++)
        {
            Point2D point = (Point2D){viewRect.position.x + x, viewRect.position.y + y};
            mapObject->view[(point.y * map->size.width) + point.x] = MAPOBJECTVIEW_VISIBLE;
        }
    }

    /*
    int viewLength = (Map_GetRoomIndexContaining(map, mapObject->position) > -1) ? 5 : 2;

    for(int d = 0; d < 360; d++)
    {
        for(int l = 0; l < viewLength; l++)
        {
            Point2D point = (Point2D){ round(mapObject->position.x + sin(d) * l), round(mapObject->position.y + cos(d) * l) };
            MapTile *tile = Map_GetTile(map, point);
            if(tile == NULL) break;

            mapObject->view[(point.y * map->size.width) + point.x] = MAPOBJECTVIEW_VISIBLE;

            if(!(tile->passable & MAPTILEPASSABLE_LIGHT)) break;
        }
    }
    */
}

MapObject *MapObject_Copy(MapObject *mapObject)
{
    MapObject *copy = MapObject_Create(mapObject->name);

    copy->colorPair = mapObject->colorPair;
    copy->description = mapObject->description;
    copy->flags = mapObject->flags;
    copy->height = mapObject->height;
    copy->hp = mapObject->hp;
    copy->hpMax = mapObject->hpMax;
    copy->layer = mapObject->layer;
    copy->o2 = mapObject->o2;
    copy->o2Max = mapObject->o2Max;
    copy->position = mapObject->position;
    copy->turnTicks = mapObject->turnTicks;
    copy->turnTicksSize = mapObject->turnTicksSize;
    copy->wchr = mapObject->wchr;
    copy->wchrAlt = mapObject->wchrAlt;

    return copy;
}

MapObject *MapObject_Create(const char *name)
{
    MapObject *mapObject = malloc(sizeof(MapObject));

    mapObject->attack = 0;
    mapObject->attackBase = 0;
    mapObject->attackToHit = 0;
    mapObject->attackToHitBase = 0;
    mapObject->colorPair = CONSOLECOLORPAIR_WHITEBLACK;
    mapObject->defense = 0;
    mapObject->defenseBase = 0;
    mapObject->description = "Nothing.";
    mapObject->equipAt = -1;
    mapObject->hp = 0;
    mapObject->hpMax = 0;
    mapObject->lastRoomIndex = -1;
    mapObject->name = name;
    mapObject->o2 = 0;
    mapObject->o2Max = 0;
    for(int i = 0; i < 2; i++)
        mapObject->equipment[i] = NULL;
    mapObject->flags = 0;
    mapObject->itemsCount = 0;
    mapObject->turnTicks = 0;
    mapObject->turnTicksSize = 0;
    mapObject->wchr = L' ';

    return mapObject;
}

int MapObject_GetEquippedAt(MapObject *mapObject, MapObjectAsItem *item)
{
    for(int i = 0; i < 2; i++)
    {
        if(mapObject->equipment[i] != item) continue;

        return i;
    }

    return -1;
}

void MapObject_RemoveItemFromItems(MapObject *mapObject, MapObjectAsItem *item)
{
    for(int i = 0; i < mapObject->itemsCount; i++)
    {
        if(mapObject->items[i] != item) continue;

        for(int j = i; j < mapObject->itemsCount - 1; j++)
            mapObject->items[j] = mapObject->items[j + 1];
        mapObject->itemsCount--;
        return;
    }
}

MapObjectAsItem *MapObject_ToItem(MapObject *mapObject)
{
    if(!(mapObject->flags & MAPOBJECTFLAG_ISITEM)) return NULL;

    MapObjectAsItem *item = malloc(sizeof(MapObjectAsItem));

    item->attack = mapObject->attack;
    item->attackToHit = mapObject->attackToHit;
    item->colorPair = mapObject->colorPair;
    item->defense = mapObject->defense;

    item->description = malloc(sizeof(char) * (strlen(mapObject->description) + 1));
    memset(item->description, 0, (strlen(mapObject->description) + 1));
    strcpy(item->description, mapObject->description);

    item->equipAt = mapObject->equipAt;
    item->flags = mapObject->flags;
    item->id = mapObject->id;

    item->name = malloc(sizeof(char) * (strlen(mapObject->name) + 1));
    memset(item->name, 0, (strlen(mapObject->name) + 1));
    strcpy(item->name, mapObject->name);

    item->wchr = mapObject->wchr;
    item->wchrAlt = mapObject->wchrAlt;

    return item;
}

void MapObject_UpdateAttributes(MapObject *mapObject)
{
    if(mapObject->flags & MAPOBJECTFLAG_ISITEM) return;

    mapObject->attack = mapObject->attackBase;
    mapObject->attackToHit = mapObject->attackToHitBase;
    mapObject->defense = mapObject->defenseBase;

    for(int i = 0; i < mapObject->itemsCount; i++)
    {
        int equippedAt = MapObject_GetEquippedAt(mapObject, mapObject->items[i]);
        if(equippedAt == -1) continue;

        mapObject->attack += mapObject->items[i]->attack;
        mapObject->attackToHit += mapObject->items[i]->attackToHit;
        mapObject->defense += mapObject->items[i]->defense;
    }
}

MapObjectAction *MapObjectAction_Create(int type)
{
    MapObjectAction *action = malloc(sizeof(MapObjectAction));

    action->object = NULL;
    action->result = false;
    action->resultMessage = "";
    action->target = NULL;
    action->targetItem = NULL;
    action->to = (Point2D){ -1, -1 };
    action->type = type;

    return action;
}

void MapObjectAction_Destroy(MapObjectAction *action)
{
    free(action);
}

void MapObjectAsItem_Destroy(MapObjectAsItem *item)
{
    free(item->name);
    free(item->description);
    free(item);
}

void MapTile_AddObject(MapTile *tile, MapObject *mapObject)
{
    if(tile->objectsCount == 10) return;

    for(int i = (int)tile->objectsCount; i > 0; i--)
        tile->objects[i] = tile->objects[i - 1];

    tile->objects[0] = mapObject;
    tile->objectsCount++;

    MapTile_UpdatePassable(tile);
}

void MapTile_Destroy(MapTile *tile, Map *map)
{
    MapTile_DestroyObjects(tile, map);
    free(tile);
}

void MapTile_DestroyObjects(MapTile *tile, Map *map)
{
    for(int i = 0; i < tile->objectsCount; i++)
        Map_DestroyObject(map, tile->objects[i]);
    tile->objectsCount = 0;
}

void MapTile_DestroyObjectsExcludePlayer(MapTile *tile, Map *map)
{
    for(int i = 0; i < tile->objectsCount; i++)
    {
        if(tile->objects[i] == map->player) continue;
        Map_DestroyObject(map, tile->objects[i]);
        tile->objectsCount--;
    }
}

MapObject *MapTile_GetObjectWithFlags(MapTile *tile, uint32_t flags)
{
    for(int i = 0; i < tile->objectsCount; i++)
    {
        if(!(tile->objects[i]->flags & flags)) continue;

        return tile->objects[i];
    }

    return NULL;
}

bool MapTile_HasObject(MapTile *tile, MapObject *mapObject)
{
    for(int i = 0; i < tile->objectsCount; i++)
    {
        if(tile->objects[i] == mapObject) return true;
    }

    return false;
}

void MapTile_RemoveObject(MapTile *tile, MapObject *mapObject)
{
    for(int i = 0; i <= tile->objectsCount; i++)
    {
        if(tile->objects[i] != mapObject) continue;

        for(int j = i; j < tile->objectsCount - 1; j++)
            tile->objects[j] = tile->objects[j + 1];

        tile->objectsCount--;
    }

    MapTile_UpdatePassable(tile);
}

void MapTile_SetType(MapTile *tile, int type)
{
    tile->type = type;

    MapTile_UpdatePassable(tile);
}

void MapTile_UpdatePassable(MapTile *tile)
{
    if(tile->type == MAPTILETYPE_EMPTY || tile->type == MAPTILETYPE_WALL)
        tile->passable = 0;
    if(tile->type == MAPTILETYPE_FLOOR)
        tile->passable = MAPTILEPASSABLE_SOLID | MAPTILEPASSABLE_LIQUID | MAPTILEPASSABLE_GAS | MAPTILEPASSABLE_LIGHT;

    if(tile->objectsCount == 0) return;

    MapObject *mapObject = tile->objects[0];

    if((mapObject->flags & MAPOBJECTFLAG_CANOPEN) && (mapObject->flags & MAPOBJECTFLAG_ISOPEN))
        return;

    if(mapObject->flags & MAPOBJECTFLAG_BLOCKSGAS)
        tile->passable &= ~MAPTILEPASSABLE_GAS;
    if(mapObject->flags & MAPOBJECTFLAG_BLOCKSLIGHT)
        tile->passable &= ~MAPTILEPASSABLE_LIGHT;
    if(mapObject->flags & MAPOBJECTFLAG_BLOCKSLIQUID)
        tile->passable &= ~MAPTILEPASSABLE_LIQUID;
    if(mapObject->flags & MAPOBJECTFLAG_BLOCKSSOLID)
        tile->passable &= ~MAPTILEPASSABLE_SOLID;
}