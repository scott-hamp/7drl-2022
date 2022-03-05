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
        {
            map->tiles[(y * map->size.width) + x]->type = rand() % 3;
        }
    }
}

void Map_Render(Map *map, Console *console)
{
    for(int y = 0; y < map->size.height; y++)
    {
        for(int x = 0; x < map->size.width; x++)
        {
            MapTile *tile = map->tiles[(y * map->size.width) + x];

            wchar_t wchr = L' ';
            if(tile->type == MAPTILETYPE_FLOOR)
                wchr = L'.';
            if(tile->type == MAPTILETYPE_WALL)
                wchr = L'▓';

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