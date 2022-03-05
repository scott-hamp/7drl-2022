#ifndef MAP_H_Z45hKZ7EPET6Q3t2
#define MAP_H_Z45hKZ7EPET6Q3t2

#include "console.h"

#define MAPOBJECTACTIONTYPE_MOVE    0

#define MAPOBJECTID_PLAYER          0

#define MAPOBJECTFLAG_CANMOVE       1 << 0
#define MAPOBJECTFLAG_PLAYER        1 << 1

#define MAPOBJECTVIEWSTATE_UNSEEN   0
#define MAPOBJECTVIEWSTATE_SEEN     1
#define MAPOBJECTVIEWSTATE_VISIBLE  2

#define MAPTILEPASSABLE_SOLID       1 << 0
#define MAPTILEPASSABLE_LIQUID      1 << 1
#define MAPTILEPASSABLE_GAS         1 << 2
#define MAPTILEPASSABLE_LIGHT       1 << 3

#define MAPTILETYPE_EMPTY           0
#define MAPTILETYPE_FLOOR           1
#define MAPTILETYPE_WALL            2

typedef struct MapObjectAsItem
{
    int colorPair;
    uint32_t flags;
    char *name;
    wchar_t wchr;
} MapObjectAsItem;

typedef struct MapObjectView
{
    short state;
    wchar_t wchr;
} MapObjectView;

typedef struct MapObject
{
    int colorPair;
    uint32_t flags;
    char *name;
    Point2D position;
    MapObjectAsItem *objects[10];
    size_t objectsCount;
    MapObjectView **view;
    wchar_t wchr;
} MapObject;

typedef struct MapObjectAction
{
    Direction2D direction;
    MapObject *object;
    bool result;
    char *resultMessage;
    MapObject *target;
    int type;
    Point2D to;
} MapObjectAction;

typedef struct MapTile
{
    MapObject *objects[10];
    size_t objectsCount;
    uint8_t passable;
    int type;
} MapTile;

typedef struct MapTileVisual
{
    int colorPair;
    wchar_t wchr;
} MapTileVisual;

typedef struct Map
{
    MapObject *player;
    Point2D renderOffset;
    Rect2D *rooms[25];
    size_t roomsCount;
    Size2D size;
    MapTile **tiles;
} Map;

MapObjectAction *Map_AttemptObjectAction(Map *map, MapObjectAction *action);
Map *Map_Create(Size2D size, Point2D renderOffset);
MapObject *Map_CreateObject(Map *map, uint16_t id);
void Map_Destroy(Map *map);
void Map_Generate(Map *map);
int Map_GetRoomIndexContaining(Map *map, Point2D point);
MapTile *Map_GetTile(Map *map, Point2D point);
MapTileVisual Map_GetTileVisual(Map *map, Point2D point);
void Map_MoveObject(Map *map, MapObject *mapObject, Point2D to);
void Map_PlaceObject(Map *map, MapObject *mapObject);
void Map_Render(Map *map, MapObject *viewer, Console *console);
MapObject *MapObject_Create(const char *name);
void MapObject_Destroy(MapObject *mapObject, Map* map);
void MapObject_ResetView(MapObject *mapObject, Map* map);
void MapObject_UpdateView(MapObject *mapObject, Map* map);
MapObjectAction *MapObjectAction_Create(int type);
void MapObjectAction_Destroy(MapObjectAction *action);
void MapTile_AddObject(MapTile *tile, MapObject *mapObject);
void MapTile_RemoveObject(MapTile *tile, MapObject *mapObject);
void MapTile_Destroy(MapTile *tile, Map *map);
void MapTile_DestroyObjects(MapTile *tile, Map *map);
bool MapTile_HasObject(MapTile *tile, MapObject *mapObject);
void MapTile_SetType(MapTile *tile, int type);

#endif