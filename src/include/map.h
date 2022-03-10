#ifndef MAP_H_Z45hKZ7EPET6Q3t2
#define MAP_H_Z45hKZ7EPET6Q3t2

#include "console.h"

#define MAPOBJECTACTIONTYPE_ATTACK          0
#define MAPOBJECTACTIONTYPE_DROP            1
#define MAPOBJECTACTIONTYPE_EQUIPUNEQUIP    2
#define MAPOBJECTACTIONTYPE_MOVE            3
#define MAPOBJECTACTIONTYPE_OPEN            4
#define MAPOBJECTACTIONTYPE_PICKUP          5
#define MAPOBJECTACTIONTYPE_USESTAIRS       6

#define MAPOBJECTEQUIPAT_BODY       0
#define MAPOBJECTEQUIPAT_WEAPON     1

#define MAPOBJECTFLAG_BLOCKSGAS             1 << 0
#define MAPOBJECTFLAG_BLOCKSLIGHT           1 << 1
#define MAPOBJECTFLAG_BLOCKSLIQUID          1 << 2
#define MAPOBJECTFLAG_BLOCKSSOLID           1 << 3
#define MAPOBJECTFLAG_CANATTACK             1 << 4
#define MAPOBJECTFLAG_CANMOVE               1 << 5
#define MAPOBJECTFLAG_CANOPEN               1 << 6
#define MAPOBJECTFLAG_HASINVENTORY          1 << 7
#define MAPOBJECTFLAG_ISEQUIPMENT           1 << 8
#define MAPOBJECTFLAG_ISHOSTILE             1 << 9
#define MAPOBJECTFLAG_ISITEM                1 << 10
#define MAPOBJECTFLAG_ISLIQUID              1 << 11
#define MAPOBJECTFLAG_ISLIQUIDSOURCE        1 << 12
#define MAPOBJECTFLAG_ISLIVING              1 << 13
#define MAPOBJECTFLAG_ISOPEN                1 << 14
#define MAPOBJECTFLAG_PLACEINDOORWAYS       1 << 15
#define MAPOBJECTFLAG_PLACEINROOM           1 << 16
#define MAPOBJECTFLAG_PLAYER                1 << 17
#define MAPOBJECTFLAG_STAIRS                1 << 18

#define MAPOBJECTID_PLAYER          0
#define MAPOBJECTID_BILGERAT       1
#define MAPOBJECTID_DIVEKNIFE       2
#define MAPOBJECTID_DOOR            3
#define MAPOBJECTID_LIFEVEST        4
#define MAPOBJECTID_STAIRS          5
#define MAPOBJECTID_WATER           6
#define MAPOBJECTID_WATERSOURCE     7

#define MAPOBJECTVIEW_UNSEEN        0
#define MAPOBJECTVIEW_SEEN          1
#define MAPOBJECTVIEW_VISIBLE       2

#define MAPTILEPASSABLE_SOLID       1 << 0
#define MAPTILEPASSABLE_LIQUID      1 << 1
#define MAPTILEPASSABLE_GAS         1 << 2
#define MAPTILEPASSABLE_LIGHT       1 << 3

#define MAPTILETYPE_EMPTY           0
#define MAPTILETYPE_FLOOR           1
#define MAPTILETYPE_WALL            2

typedef struct MapObjectAsItem
{
    int attack, attackToHit, defense;
    int colorPair;
    char *description;
    char *details;
    int equipAt;
    uint32_t flags;
    int id;
    char *name;
    wchar_t wchr, wchrAlt;
} MapObjectAsItem;

typedef struct MapObject
{
    int attackBase, attackToHitBase, defenseBase;
    int attack, attackToHit, defense;
    int attackDistance;
    int colorPair;
    char *description;
    char *details;
    int equipAt;
    MapObjectAsItem *equipment[2];
    uint32_t flags;
    int height;
    int hp, o2;
    size_t hpMax, o2Max;
    int id;
    int layer;
    int lastRoomIndex;
    char *name;
    Point2D position;
    MapObjectAsItem *items[10];
    size_t itemsCount;
    int turnTicks;
    size_t turnTicksSize;
    int *view;
    wchar_t wchr, wchrAlt;
} MapObject;

typedef struct MapObjectAction
{
    Direction2D direction;
    MapObject *object;
    bool result;
    char *resultMessage;
    int resultValueInt;
    MapObject *target;
    MapObjectAsItem *targetItem;
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

typedef struct Map
{
    int level;
    int levelFloodTimer;
    size_t levelFloodTimerSize;
    MapObject *player;
    Point2D renderOffset;
    Point2D roomDoorways[10];
    size_t roomDoorwaysCount;
    Rect2D *rooms[25];
    size_t roomsCount;
    Size2D size;
    MapTile **tiles;
} Map;

MapObjectAction *Map_AttemptObjectAction(Map *map, MapObjectAction *action);
void Map_Clear(Map *map);
void Map_ClearExcludePlayer(Map *map);
Map *Map_Create(Size2D size, Point2D renderOffset);
MapObject *Map_CreateObject(Map *map, uint16_t id);
void Map_Destroy(Map *map);
void Map_DestroyObject(Map* map, MapObject *mapObject);
void Map_Generate(Map *map);
int Map_GetObjectView(Map *map, MapObject *mapObject, Point2D point);
int Map_GetPointColorPair(Map *map, Point2D point);
char *Map_GetPointDescription(Map *map, Point2D point);
wchar_t Map_GetPointWChr(Map *map, Point2D point);
int Map_GetRoomIndexContaining(Map *map, Point2D point);
int Map_GetSimpleDistance(Map *map, Point2D from, Point2D to);
MapTile *Map_GetTile(Map *map, Point2D point);
void Map_MoveObject(Map *map, MapObject *mapObject, Point2D to);
MapObjectAction *Map_ObjectAttemptActionAsTarget(Map *map, MapObject *mapObject, MapObjectAction *action);
void Map_PlaceObject(Map *map, MapObject *mapObject);
void Map_Render(Map *map, MapObject *viewer, Console *console);
void Map_RenderForPlayer(Map *map, Console *console);
void Map_RenderRect(Map *map, MapObject *viewer, Console *console, Rect2D rect);
void Map_ResetObjectView(Map* map, MapObject *mapObject);
void Map_UpdateObjectView(Map* map, MapObject *mapObject);
void MapObject_AddItemToItems(MapObject *mapObject, MapObjectAsItem *item);
MapObject *MapObject_Copy(MapObject *mapObject);
MapObject *MapObject_Create(const char *name);
int MapObject_GetEquippedAt(MapObject *mapObject, MapObjectAsItem *item);
void MapObject_RemoveItemFromItems(MapObject *mapObject, MapObjectAsItem *item);
MapObjectAsItem *MapObject_ToItem(MapObject *mapObject);
void MapObject_UpdateAttributes(MapObject *mapObject);
MapObjectAction *MapObjectAction_Create(int type);
void MapObjectAction_Destroy(MapObjectAction *action);
void MapObjectAsItem_Destroy(MapObjectAsItem *item);
void MapTile_AddObject(MapTile *tile, MapObject *mapObject);
void MapTile_RemoveObject(MapTile *tile, MapObject *mapObject);
void MapTile_Destroy(MapTile *tile, Map *map);
void MapTile_DestroyObjects(MapTile *tile, Map *map);
void MapTile_DestroyObjectsExcludePlayer(MapTile *tile, Map *map);
MapObject *MapTile_GetObjectWithFlags(MapTile *tile, uint32_t flags);
bool MapTile_HasObject(MapTile *tile, MapObject *mapObject);
void MapTile_SetType(MapTile *tile, int type);
void MapTile_UpdatePassable(MapTile *tile);

#endif