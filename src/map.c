#include "include/map.h"

MapObjectAction *Map_AttemptObjectAction(Map *map, MapObjectAction *action)
{
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
                if(tile->objects[0]->flags & MAPOBJECTFLAG_CANOPEN)
                {
                    action->type = MAPOBJECTACTIONTYPE_OPEN;
                    action->target = tile->objects[0];
                    action->to = action->target->position;
                    return Map_AttemptObjectAction(map, action);
                }
            }

            action->resultMessage = "Tile impassable.";
            return action;
        }

        action->result = true;
        Map_MoveObject(map, action->object, to);

        return action;
    }

    if(action->type == MAPOBJECTACTIONTYPE_OPEN)
    {
        action = Map_MapObjectAttemptActionAsTarget(map, action->target, action);
        return action;
    }

    return action;
}

Map *Map_Create(Size2D size, Point2D renderOffset)
{
    Map *map = malloc(sizeof(Map));

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
    mapObject->view = NULL;
    bool hasView = false;

    if(id == MAPOBJECTID_PLAYER) // Player
    {
        mapObject->name = "Player";
        mapObject->flags |= (MAPOBJECTFLAG_CANMOVE | MAPOBJECTFLAG_PLAYER);
        mapObject->wchr = L'@';
        hasView = true;
    }

    if(id == MAPOBJECTID_DOOR) // Door
    {
        mapObject->name = "Door";
        mapObject->flags |= (MAPOBJECTFLAG_CANOPEN | MAPOBJECTFLAG_BLOCKSGAS | MAPOBJECTFLAG_BLOCKSLIGHT | MAPOBJECTFLAG_BLOCKSLIQUID | MAPOBJECTFLAG_BLOCKSSOLID | MAPOBJECTFLAG_PLACEINDOORWAYS);
        mapObject->wchr = L'+';
        mapObject->wchrAlt = L'`';
    }

    if(hasView)
    {
        mapObject->view = malloc(sizeof(MapObjectView) * (map->size.width * map->size.height));
        for(int i = 0; i < map->size.width * map->size.height; i++)
        {
            mapObject->view[i] = malloc(sizeof(MapObjectView));
            mapObject->view[i]->state = MAPOBJECTVIEWSTATE_UNSEEN;
            mapObject->view[i]->wchr = L' ';
        }
    }

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
    for(int i = 0; i < mapObject->objectsCount; i++)
        Map_DestroyObject(map, mapObject->objects[i]);

    if(mapObject->view != NULL)
    {
        for(int i = 0; i < map->size.width * map->size.height; i++)
            free(mapObject->view[i]);
        free(mapObject->view);
    }

    free(mapObject);
}

void Map_Generate(Map *map)
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

        if(map->roomsCount >= 4) break;
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

        for(int y = 0; y < map->rooms[i]->size.height; y++)
        {
            for(int x = 0; x < map->rooms[i]->size.width; x++)
            {
                Point2D point = (Point2D){ map->rooms[i]->position.x + x, map->rooms[i]->position.y + y };
                if(point.x < 1 || point.y < 1 || point.x >= map->size.width - 1 || point.y >= map->size.height - 1) 
                    continue;
                MapTile_SetType(map->tiles[(point.y * map->size.width) + point.x], MAPTILETYPE_FLOOR);
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

    for(int i = 0; i < map->roomDoorwaysCount; i++)
        free(map->roomDoorways[i]);
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

                map->roomDoorways[map->roomDoorwaysCount] = malloc(sizeof(Point2D));
                map->roomDoorways[map->roomDoorwaysCount]->x = point.x;
                map->roomDoorways[map->roomDoorwaysCount]->y = point.y;
                map->roomDoorwaysCount++;
            }
        }
    }

    map->player = Map_CreateObject(map, MAPOBJECTID_PLAYER);
    Map_PlaceObject(map, map->player);

    for(int i = 0; i < 4 + rand() % 2; i++)
        Map_PlaceObject(map, Map_CreateObject(map, MAPOBJECTID_DOOR));
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

MapTileVisual Map_GetTileVisual(Map *map, Point2D point)
{
    int colorPair = CONSOLECOLORPAIR_WHITEBLACK;
    wchar_t wchr = L' ';

    MapTile *tile = Map_GetTile(map, point);
    if(tile == NULL)
        return (MapTileVisual){ colorPair, wchr };

    if(tile->objectsCount > 0)
    {
        colorPair = tile->objects[0]->colorPair;
        wchr = tile->objects[0]->wchr;
    }
    else
    {
        //░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀

        if(tile->type == MAPTILETYPE_FLOOR) 
            wchr = (Map_GetRoomIndexContaining(map, (Point2D){ point.x, point.y }) > -1) ? L'.' : L'▒';
        if(tile->type == MAPTILETYPE_WALL)
        {
            //wchr = L'▓';
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
        }
    }

    return (MapTileVisual){ colorPair, wchr };
}

MapObjectAction *Map_MapObjectAttemptActionAsTarget(Map *map, MapObject *mapObject, MapObjectAction *action)
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

void Map_MoveObject(Map *map, MapObject *mapObject, Point2D to)
{
    MapTile *tileFrom = Map_GetTile(map, mapObject->position);
    MapTile *tileTo = Map_GetTile(map, to);
    if(tileFrom == NULL || tileTo == NULL) return;

    MapTile_RemoveObject(tileFrom, mapObject);
    mapObject->position = to;
    MapTile_AddObject(tileTo, mapObject);
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

            Point2D *roomDoorway = map->roomDoorways[rand() % map->roomDoorwaysCount];
            point = (Point2D){ roomDoorway->x, roomDoorway->y };
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
            
            if(tile->type != MAPTILETYPE_FLOOR) continue;
            if(tile->objectsCount > 0) continue;
        }

        mapObject->position = point;
        MapTile_AddObject(tile, mapObject);
        Map_UpdateObjectView(map, mapObject);

        return;
    }
}

void Map_Render(Map *map, MapObject *viewer, Console *console)
{
    for(int y = 0; y < map->size.height; y++)
    {
        for(int x = 0; x < map->size.width; x++)
        {
            MapTile *tile = map->tiles[(y * map->size.width) + x];

            int colorPair = CONSOLECOLORPAIR_WHITEBLACK;
            wchar_t wchr = L' ';

            if(x > 0 && y > 0 && x < map->size.width - 1 && y < map->size.height - 1)
            {
                MapObjectView *view = viewer->view[(y * map->size.width) + x];
                if(view->state == MAPOBJECTVIEWSTATE_VISIBLE)
                {
                    MapTileVisual visual = Map_GetTileVisual(map, (Point2D){ x, y });
                    colorPair = visual.colorPair;
                    wchr = visual.wchr;
                }
                else
                {
                    if(view->state == MAPOBJECTVIEWSTATE_SEEN)
                    {
                        wchr = view->wchr;
                        if(wchr == L'.') wchr = L' ';
                        if(wchr == L'▒') wchr = L'░';
                    }
                }
            }

            Point2D point = (Point2D){ map->renderOffset.x + x, map->renderOffset.y + y };
            Console_SetCharW(console, point.y, point.x, wchr, colorPair, 0);
        }
    }
}

void Map_ResetObjectView(Map* map, MapObject *mapObject)
{
    if(mapObject->view == NULL) return;

    for(int i = 0; i < map->size.width * map->size.height; i++)
    {
        mapObject->view[i]->state = MAPOBJECTVIEWSTATE_UNSEEN;
        mapObject->view[i]->wchr = L' ';
    }
}

void Map_UpdateObjectView(Map* map, MapObject *mapObject)
{
    if(mapObject->view == NULL) return;

    for(int i = 0; i < map->size.width * map->size.height; i++)
    {
        if(mapObject->view[i]->state != MAPOBJECTVIEWSTATE_VISIBLE)
            continue;
        mapObject->view[i]->state = MAPOBJECTVIEWSTATE_SEEN;
    }

    int viewLength = (Map_GetRoomIndexContaining(map, mapObject->position) > -1) ? 10 : 2;

    for(int d = 0; d < 360; d++)
    {
        for(int l = 0; l < viewLength; l++)
        {
            Point2D point = (Point2D){ round(mapObject->position.x + sin(d) * l), round(mapObject->position.y + cos(d) * l) };
            MapTile *tile = Map_GetTile(map, point);
            if(tile == NULL) break;

            MapObjectView *view = mapObject->view[(point.y * map->size.width) + point.x];
            view->state = MAPOBJECTVIEWSTATE_VISIBLE;
            MapTileVisual visual = Map_GetTileVisual(map, point);
            view->wchr = visual.wchr;

            if(!(tile->passable & MAPTILEPASSABLE_LIGHT)) break;
        }
    }
}

MapObject *MapObject_Create(const char *name)
{
    MapObject *mapObject = malloc(sizeof(MapObject));

    mapObject->colorPair = CONSOLECOLORPAIR_WHITEBLACK;
    mapObject->name = name;
    mapObject->flags = 0;
    mapObject->objectsCount = 0;
    mapObject->wchr = L' ';

    return mapObject;
}

MapObjectAction *MapObjectAction_Create(int type)
{
    MapObjectAction *action = malloc(sizeof(MapObjectAction));

    action->type = type;

    return action;
}

void MapObjectAction_Destroy(MapObjectAction *action)
{
    free(action);
}

void MapTile_AddObject(MapTile *tile, MapObject *mapObject)
{
    if(tile->objectsCount == 10) return;

    for(int i = 1; i <= tile->objectsCount; i++)
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