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

#define MAPOBJECTEQUIPAT_SLOTSCOUNT     4
#define MAPOBJECTEQUIPAT_BACK           0
#define MAPOBJECTEQUIPAT_BODY           1
#define MAPOBJECTEQUIPAT_FACE           2
#define MAPOBJECTEQUIPAT_WEAPON         3

#define MAPOBJECTFLAG_BLOCKSGAS             1 << 0
#define MAPOBJECTFLAG_BLOCKSLIGHT           1 << 1
#define MAPOBJECTFLAG_BLOCKSLIQUID          1 << 2
#define MAPOBJECTFLAG_BLOCKSSOLID           1 << 3
#define MAPOBJECTFLAG_CANATTACK             1 << 4
#define MAPOBJECTFLAG_CANMOVE               1 << 5
#define MAPOBJECTFLAG_CANOPEN               1 << 6
#define MAPOBJECTFLAG_CANOPENOTHER          1 << 7
#define MAPOBJECTFLAG_HASINVENTORY          1 << 8
#define MAPOBJECTFLAG_ISAQUATIC             1 << 9
#define MAPOBJECTFLAG_ISEQUIPMENT           1 << 10
#define MAPOBJECTFLAG_ISHOSTILE             1 << 11
#define MAPOBJECTFLAG_ISITEM                1 << 12
#define MAPOBJECTFLAG_ISLIQUID              1 << 13
#define MAPOBJECTFLAG_ISLIQUIDSOURCE        1 << 14
#define MAPOBJECTFLAG_ISLIVING              1 << 15
#define MAPOBJECTFLAG_ISOPEN                1 << 16
#define MAPOBJECTFLAG_ITEMINCREASE02        1 << 17
#define MAPOBJECTFLAG_ITEMISRANGED          1 << 18
#define MAPOBJECTFLAG_ITEMSUPPLY02          1 << 19
#define MAPOBJECTFLAG_PLACEINDOORWAYS       1 << 20
#define MAPOBJECTFLAG_PLACEINROOM           1 << 21
#define MAPOBJECTFLAG_PLACEINWATER          1 << 22
#define MAPOBJECTFLAG_PLAYER                1 << 23
#define MAPOBJECTFLAG_STAIRS                1 << 24

#define MAPOBJECTID_PLAYER          0
#define MAPOBJECTID_BILGERAT        1
#define MAPOBJECTID_DIVEKNIFE       2
#define MAPOBJECTID_DOOR            3
#define MAPOBJECTID_HARPOON         4
#define MAPOBJECTID_HARPOONGUN      5
#define MAPOBJECTID_LIFEVEST        6
#define MAPOBJECTID_MANGYDOG        7
#define MAPOBJECTID_SCUBAMASK       8
#define MAPOBJECTID_SCUBATANK       9
#define MAPOBJECTID_SMALLEEL        10
#define MAPOBJECTID_STAIRS          11
#define MAPOBJECTID_TIGERFISH       12
#define MAPOBJECTID_WATER           13
#define MAPOBJECTID_WATERSOURCE     14

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
    int consumesItemWhenUsedID;
    char *description;
    char *details;
    int equipAt;
    uint32_t flags;
    int id;
    char *name;
    int hp, hpMax;
    int o2, o2Max;
    wchar_t wchr, wchrAlt;
} MapObjectAsItem;

typedef struct MapObject
{
    int attackBase, attackToHitBase, defenseBase;
    int attack, attackToHit, defense;
    int attackDistance;
    char *attackVerbs[2];
    int colorPair;
    int consumesItemWhenUsedID;
    char *description;
    char *details;
    int equipAt;
    MapObjectAsItem *equipment[MAPOBJECTEQUIPAT_SLOTSCOUNT];
    uint32_t flags;
    int height;
    int hp, o2;
    size_t hpMax, o2Max;
    int hpMaxBase, o2MaxBase;
    int hpRecoverTimer, hpRecoverTimerLength;
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
    MapObjectAsItem *objectItem;
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
    Rect2D *rooms[25];
    size_t roomsCount;
    Size2D size;
    MapTile **tiles;
} Map;

MapObjectAction *Map_AttemptObjectAction(Map *map, MapObjectAction *action);
void Map_Clear(Map *map);
Map *Map_Create(Size2D size, Point2D renderOffset);
MapObject *Map_CreateObject(Map *map, uint16_t id);
void Map_Destroy(Map *map);
void Map_Generate(Map *map);
MapObject *Map_GetClosestObjectWithFlags(Map *map, Point2D to, uint32_t flags);
int Map_GetObjectView(Map *map, MapObject *mapObject, Point2D point);
int Map_GetPointColorPair(Map *map, Point2D point);
char *Map_GetPointDescription(Map *map, Point2D point);
wchar_t Map_GetPointWChr(Map *map, Point2D point);
int Map_GetRoomIndexContaining(Map *map, Point2D point);
int Map_GetRoomIndexContainingBorder(Map *map, Point2D point, int border);
int Map_GetSimpleDistance(Map *map, Point2D from, Point2D to);
MapTile *Map_GetTile(Map *map, Point2D point);
bool Map_LevelFloodTimerTick(Map *map);
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
void MapObject_Destroy(MapObject *mapObject);
int MapObject_GetEquippedAt(MapObject *mapObject, MapObjectAsItem *item);
MapObjectAsItem *MapObject_GetItemByID(MapObject *mapObject, int itemID);
void MapObject_RemoveItemFromItems(MapObject *mapObject, MapObjectAsItem *item);
MapObjectAsItem *MapObject_ToItem(MapObject *mapObject);
void MapObject_UpdateAttributes(MapObject *mapObject);
void MapObject_UpdateAttributesExcludeItemsWithFlags(MapObject *mapObject, uint32_t flags);
void MapObject_UpdateItems(MapObject *mapObject);
MapObjectAction *MapObjectAction_Create(int type);
void MapObjectAction_Destroy(MapObjectAction *action);
void MapObjectAsItem_Destroy(MapObjectAsItem *item);
void MapTile_AddObject(MapTile *tile, MapObject *mapObject);
void MapTile_RemoveObject(MapTile *tile, MapObject *mapObject);
void MapTile_Destroy(MapTile *tile);
void MapTile_DestroyObject(MapTile *tile, MapObject *mapObject);
void MapTile_DestroyObjects(MapTile *tile);
MapObject *MapTile_GetObjectWithFlags(MapTile *tile, uint32_t flags);
bool MapTile_HasObject(MapTile *tile, MapObject *mapObject);
void MapTile_SetType(MapTile *tile, int type);
void MapTile_UpdatePassable(MapTile *tile);

#endif