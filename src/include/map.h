#ifndef MAP_H_Z45hKZ7EPET6Q3t2
#define MAP_H_Z45hKZ7EPET6Q3t2

#include "console.h"

#define MAPOBJECTACTIONTYPE_MOVE    0
#define MAPOBJECTACTIONTYPE_OPEN    1

#define MAPOBJECTID_PLAYER          0
#define MAPOBJECTID_DOOR            1
#define MAPOBJECTID_WATER           2
#define MAPOBJECTID_WATERSOURCE     3

#define MAPOBJECTFLAG_BLOCKSGAS             1 << 0
#define MAPOBJECTFLAG_BLOCKSLIGHT           1 << 1
#define MAPOBJECTFLAG_BLOCKSLIQUID          1 << 2
#define MAPOBJECTFLAG_BLOCKSSOLID           1 << 3
#define MAPOBJECTFLAG_CANMOVE               1 << 4
#define MAPOBJECTFLAG_CANOPEN               1 << 5
#define MAPOBJECTFLAG_ISLIQUID              1 << 6
#define MAPOBJECTFLAG_ISLIQUIDSOURCE        1 << 7
#define MAPOBJECTFLAG_ISLIVING              1 << 8
#define MAPOBJECTFLAG_ISOPEN                1 << 9
#define MAPOBJECTFLAG_PLACEINDOORWAYS       1 << 10
#define MAPOBJECTFLAG_PLAYER                1 << 11

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

typedef struct MapObjectView
{
    short state;
    wchar_t wchr;
} MapObjectView;

typedef struct MapObjectAsItem
{
    int colorPair;
    uint32_t flags;
    char *name;
    wchar_t wchr, wchrAlt;
} MapObjectAsItem;

typedef struct MapObject
{
    int colorPair;
    uint32_t flags;
    int height;
    int hp, o2;
    size_t hpMax, o2Max;
    char *name;
    Point2D position;
    MapObjectAsItem *objects[10];
    size_t objectsCount;
    int turnTicks;
    size_t turnTicksSize;
    MapObjectView **view;
    wchar_t wchr, wchrAlt;
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
    int levelFloodTimer;
    size_t levelFloodTimerSize;
    MapObject *player;
    Point2D renderOffset;
    Point2D *roomDoorways[10];
    size_t roomDoorwaysCount;
    Rect2D *rooms[25];
    size_t roomsCount;
    Size2D size;
    MapTile **tiles;
} Map;

MapObjectAction *Map_AttemptObjectAction(Map *map, MapObjectAction *action);
Map *Map_Create(Size2D size, Point2D renderOffset);
MapObject *Map_CreateObject(Map *map, uint16_t id);
void Map_Destroy(Map *map);
void Map_DestroyObject(Map* map, MapObject *mapObject);
void Map_Generate(Map *map);
int Map_GetRoomIndexContaining(Map *map, Point2D point);
MapTile *Map_GetTile(Map *map, Point2D point);
MapTileVisual Map_GetTileVisual(Map *map, Point2D point);
MapObjectAction *Map_MapObjectAttemptActionAsTarget(Map *map, MapObject *mapObject, MapObjectAction *action);
void Map_MoveObject(Map *map, MapObject *mapObject, Point2D to);
void Map_PlaceObject(Map *map, MapObject *mapObject);
void Map_Render(Map *map, MapObject *viewer, Console *console);
void Map_ResetObjectView(Map* map, MapObject *mapObject);
void Map_UpdateObjectView(Map* map, MapObject *mapObject);
MapObject *MapObject_Copy(MapObject *mapObject);
MapObject *MapObject_Create(const char *name);
MapObjectAction *MapObjectAction_Create(int type);
void MapObjectAction_Destroy(MapObjectAction *action);
void MapTile_AddObject(MapTile *tile, MapObject *mapObject);
void MapTile_RemoveObject(MapTile *tile, MapObject *mapObject);
void MapTile_Destroy(MapTile *tile, Map *map);
void MapTile_DestroyObjects(MapTile *tile, Map *map);
bool MapTile_HasObject(MapTile *tile, MapObject *mapObject);
void MapTile_SetType(MapTile *tile, int type);
void MapTile_UpdatePassable(MapTile *tile);

#endif