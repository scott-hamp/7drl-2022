#include "include/map.h"

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

void Map_Destroy(Map *map)
{
    for(int i = 0; i < (map->size.width * map->size.height); i++)
    {
        for(int j = 0; j < map->tiles[i]->objectsCount; j++)
            MapObject_Destroy(map->tiles[i]->objects[j]);
        free(map->tiles[i]);
    }
    free(map->tiles);
}

void Map_Generate(Map *map)
{
    for(int y = 0; y < map->size.height; y++)
    {
        for(int x = 0; x < map->size.width; x++)
            map->tiles[(y * map->size.width) + x]->type = MAPTILETYPE_WALL;
    }

    for(int i = 0; i < map->roomsCount; i++)
        free(map->rooms[i]);
    map->roomsCount = 0;

    Point2D divisions = (Point2D){ 3, 3 };

    for(int y = 1; y < map->size.height - 1; y += map->size.height / divisions.y)
    {
        for(int x = 1; x < map->size.width - 1; x += map->size.width / divisions.x)
        {
            if(rand() % 100 >= 90) continue;

            map->rooms[map->roomsCount] = malloc(sizeof(Rect2D));
            map->rooms[map->roomsCount]->position = (Point2D){ x, y };
            map->rooms[map->roomsCount]->size = (Size2D){ (map->size.width / divisions.x) - 1, (map->size.height / divisions.y) - 1 };
            map->roomsCount++;
        }
    }

    for(int i = 0; i < map->roomsCount; i++)
    {
        Point2D offset = (Point2D){ 1 + rand() % 3, 1 + rand() % 3 };
        map->rooms[i]->position.x += offset.x;
        map->rooms[i]->position.y += offset.y;
        map->rooms[i]->size.width -= ((offset.x * 2) + rand() % 4);
        map->rooms[i]->size.height -= ((offset.y * 2) + rand() % 2);
        if(map->rooms[i]->size.width < 3)
            map->rooms[i]->size.width++;
        if(map->rooms[i]->size.height < 3)
            map->rooms[i]->size.height++;
        if(map->rooms[i]->position.y + map->rooms[i]->size.width > map->size.width - 2)
            map->rooms[i]->size.width--;
        if(map->rooms[i]->position.y + map->rooms[i]->size.height > map->size.height - 2)
            map->rooms[i]->size.height--;

        for(int y = 0; y < map->rooms[i]->size.height; y++)
        {
            for(int x = 0; x < map->rooms[i]->size.width; x++)
            {
                Point2D point = (Point2D){ map->rooms[i]->position.x + x, map->rooms[i]->position.y + y };
                if(point.x < 1 || point.y < 1 || point.x >= map->size.width - 1 || point.y >= map->size.height - 1) 
                    continue;
                map->tiles[(point.y * map->size.width) + point.x]->type = MAPTILETYPE_FLOOR;
            }
        }
    }

    return;

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
            /*
            int ric = Map_GetRoomIndexContaining(map, at);
            if(ric != atRoomIndex && ric > -1)
            {
                if(roomConnected[ric]) break;
            }
            */

            if(at.x > 0 && at.y > 0 && at.x < map->size.width - 1 && at.y < map->size.height - 1)
                map->tiles[(at.y * map->size.width) + at.x]->type = MAPTILETYPE_FLOOR;

            if(rand() % 10 >= 7) 
                at.x += -1 + rand() % 2;
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

void Map_Render(Map *map, Console *console)
{
    for(int y = 0; y < map->size.height; y++)
    {
        for(int x = 0; x < map->size.width; x++)
        {
            MapTile *tile = map->tiles[(y * map->size.width) + x];

            wchar_t wchr = L' ';

            if(x > 0 && y > 0 && x < map->size.width - 1 && y < map->size.height - 1)
            {
                //░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀

                if(tile->type == MAPTILETYPE_FLOOR) 
                    wchr = (Map_GetRoomIndexContaining(map, (Point2D){ x, y }) > -1) ? L'.' : L'░';
                if(tile->type == MAPTILETYPE_WALL)
                {
                    //wchr = L'▓';
                    if(map->tiles[((y + 1) * map->size.width) + (x + 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ x + 1, y + 1 }) > -1)
                        wchr = L'┌';
                    if(map->tiles[((y + 1) * map->size.width) + (x - 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ x - 1, y + 1 }) > -1)
                        wchr = L'┐';
                    if(map->tiles[((y - 1) * map->size.width) + (x + 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ x + 1, y - 1 }) > -1)
                        wchr = L'└';
                    if(map->tiles[((y - 1) * map->size.width) + (x - 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ x - 1, y - 1 }) > -1)
                        wchr = L'┘';
                    if((map->tiles[(y * map->size.width) + (x - 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ x - 1, y }) > -1) || (map->tiles[(y * map->size.width) + (x + 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ x + 1, y }) > -1))
                        wchr = L'│';
                    if((map->tiles[((y - 1) * map->size.width) + x]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ x , y - 1 }) > -1) || (map->tiles[((y + 1) * map->size.width) + x]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ x , y + 1 }) > -1))
                        wchr = L'─';
                }
            }

            Point2D point = (Point2D){ map->renderOffset.x + x, map->renderOffset.y + y };
            Console_SetCharW(console, point.y, point.x, wchr, 0, 0);
        }
    }
}

MapObject *MapObject_Create(uint16_t id)
{

}

void MapObject_Destroy(MapObject *mapObject)
{
    for(int i = 0; i < mapObject->objectsCount; i++)
        MapObject_Destroy(mapObject->objects[i]);
    free(mapObject);
}