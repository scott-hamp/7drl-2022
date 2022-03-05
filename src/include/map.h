#ifndef MAP_H_Z45hKZ7EPET6Q3t2
#define MAP_H_Z45hKZ7EPET6Q3t2

#include "console.h"

#define MAPTILETYPE_EMPTY   0
#define MAPTILETYPE_FLOOR   1
#define MAPTILETYPE_WALL    2

typedef struct MapObjectAsItem
{
    uint16_t flags;
    char *name;
} MapObjectAsItem;

typedef struct MapObject
{
    uint16_t flags;
    char *name;
    MapObjectAsItem *objects[10];
    size_t objectsCount;
} MapObject;

typedef struct MapTile
{
    MapObject *objects[10];
    size_t objectsCount;
    int type;
} MapTile;

typedef struct Map
{
    MapTile **tiles;
    Point2D renderOffset;
    Size2D size;
} Map;

Map *Map_Create(Size2D size, Point2D renderOffset);
void Map_Destroy(Map *map);
void Map_Generate(Map *map);
void Map_Render(Map *map, Console *console);
MapObject *MapObject_Create(uint16_t id);
void MapObject_Destroy(MapObject *mapObject);

#endif