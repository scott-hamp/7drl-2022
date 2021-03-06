#include "include/map.h"

MapObjectAction *Map_AttemptObjectAction(Map *map, MapObjectAction *action)
{
    if(action->type == MAPOBJECTACTIONTYPE_ATTACK)
    {
        if(action->object == action->target) return action;
        if(!(action->object->flags & MAPOBJECTFLAG_CANATTACK) || !(action->target->flags & MAPOBJECTFLAG_ISLIVING)) 
            return action;

        int dist = Map_GetSimpleDistance(map, action->object->position, action->target->position);
        int hit = action->object->attackToHit - (dist - 1) + (rand() % 20);

        if(action->objectItem != NULL)
        {
            if(action->objectItem->consumesItemWhenUsedID != -1)
            {
                MapObjectAsItem *ammo = MapObject_GetItemByID(action->object, action->objectItem->consumesItemWhenUsedID);
                MapObject_RemoveItemFromItems(action->object, ammo);
            }
        }
        
        if(hit <= action->target->defense) return action;

        action->resultValueInt = action->object->attack;
        action->target->hp -= action->resultValueInt;
        if(action->target->hp <= 0)
        {
            action->target->hp = 0;
            action->target->flags &= ~MAPOBJECTFLAG_ISLIVING;
            if(action->target != map->player)
            {
                MapTile_RemoveObject(Map_GetTile(map, action->target->position), action->target);
                MapObject_Destroy(action->target);
                action->target = NULL;
            }
        }

        action->result = true;
        return action;
    }

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
                mapObject->o2 = action->targetItem->o2;
                mapObject->o2Max = action->targetItem->o2Max;
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

        if(action->targetItem->flags & MAPOBJECTFLAG_ITEMINCREASE02)
            action->object->o2 += action->targetItem->o2;

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

        if(action->object->flags & MAPOBJECTFLAG_ISAQUATIC)
        {
            if(!(tile->passable & MAPTILEPASSABLE_LIQUID) || MapTile_GetObjectWithFlags(tile, MAPOBJECTFLAG_ISLIQUID) == NULL)
            {
                action->resultMessage = "Point does not have water.";
                return action;
            }
        }

        if(!(tile->passable & MAPTILEPASSABLE_SOLID))
        {
            MapObject *target = NULL;
            
            if(action->object == map->player)
            {
                target = MapTile_GetObjectWithFlags(tile, MAPOBJECTFLAG_ISHOSTILE);
                if(target != NULL)
                {
                    action->type = MAPOBJECTACTIONTYPE_ATTACK;
                    action->target = target;
                    return Map_AttemptObjectAction(map, action);
                }
            }

            if(action->object->flags & MAPOBJECTFLAG_CANOPENOTHER)
            {
                target = MapTile_GetObjectWithFlags(tile, MAPOBJECTFLAG_CANOPEN);
                if(target != NULL)
                {
                    action->type = MAPOBJECTACTIONTYPE_OPEN;
                    action->target = target;
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
        if(!(action->object->flags & MAPOBJECTFLAG_HASINVENTORY)) return action;

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
        MapObject_Destroy(action->target);
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
            MapTile_DestroyObjects(tile);
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

    if(id == MAPOBJECTID_BILGERAT) // Bilge rat
    {
        mapObject->attackBase = 1;
        mapObject->attackDistance = 1;
        mapObject->attackToHitBase = -2;
        mapObject->attackVerbs[0] = "bite";
        mapObject->attackVerbs[1] = "bites";
        mapObject->colorPair = CONSOLECOLORPAIR_YELLOWBLACK;
        mapObject->defenseBase = 1;
        mapObject->description = "A bilge rat.";
        mapObject->layer = 1;
        mapObject->name = "bilge rat";
        mapObject->flags |= (MAPOBJECTFLAG_BLOCKSSOLID | MAPOBJECTFLAG_CANATTACK | MAPOBJECTFLAG_CANMOVE | MAPOBJECTFLAG_ISHOSTILE | MAPOBJECTFLAG_ISLIVING | MAPOBJECTFLAG_PLACEINROOM);
        mapObject->hp = 3;
        mapObject->hpMaxBase = 3;
        mapObject->o2 = 5;
        mapObject->o2MaxBase = 5;
        mapObject->sight = 12;
        mapObject->turnTicks = 8;
        mapObject->turnTicksSize = 8;
        mapObject->wchr = L'r';
        mapObject->wchrAlt = L'r';
    }

    if(id == MAPOBJECTID_PLAYER) // PLAYER
    {
        mapObject->attackBase = 1;
        mapObject->attackToHitBase = 1;
        mapObject->attackVerbs[0] = "hit";
        mapObject->attackVerbs[1] = "hits";
        mapObject->defenseBase = 3;
        mapObject->description = "Yourself.";
        mapObject->hpRecoverTimer = 25;
        mapObject->hpRecoverTimerLength = 25;
        mapObject->layer = 0;
        mapObject->name = "Player";
        mapObject->flags |= (MAPOBJECTFLAG_BLOCKSSOLID | MAPOBJECTFLAG_CANATTACK | MAPOBJECTFLAG_CANMOVE | MAPOBJECTFLAG_CANOPENOTHER | MAPOBJECTFLAG_HASINVENTORY | MAPOBJECTFLAG_ISLIVING | MAPOBJECTFLAG_PLACEINROOM | MAPOBJECTFLAG_PLAYER);
        mapObject->hp = 10;
        mapObject->hpMaxBase = 10;
        mapObject->o2 = 30;
        mapObject->o2MaxBase = 30;
        mapObject->sight = 10;
        mapObject->turnTicks = 10;
        mapObject->turnTicksSize = 10;
        mapObject->wchr = L'@';
        mapObject->wchrAlt = L'@';
    }

    if(id == MAPOBJECTID_DOOR) // Door
    {
        mapObject->description = "A door.";
        mapObject->layer = 2;
        mapObject->name = "door";
        mapObject->flags |= (MAPOBJECTFLAG_CANOPEN | MAPOBJECTFLAG_BLOCKSGAS | MAPOBJECTFLAG_BLOCKSLIGHT | MAPOBJECTFLAG_BLOCKSLIQUID | MAPOBJECTFLAG_BLOCKSSOLID | MAPOBJECTFLAG_PLACEINDOORWAYS | MAPOBJECTFLAG_RENDERALWAYSAFTERSEEN);
        mapObject->wchr = L'+';
        mapObject->wchrAlt = L'`';
    }

    if(id == MAPOBJECTID_DIVEKNIFE) // Dive knife
    {
        mapObject->attack = 3;
        mapObject->attackToHit = 1;
        mapObject->description = "A dive knife.";
        mapObject->details = "[ATT: 3 +1]";
        mapObject->equipAt = MAPOBJECTEQUIPAT_WEAPON;
        mapObject->layer = 2;
        mapObject->name = "dive knife";
        mapObject->flags |= (MAPOBJECTFLAG_ISEQUIPMENT | MAPOBJECTFLAG_ISITEM | MAPOBJECTFLAG_PLACEINROOM);
        mapObject->wchr = L'/';
        mapObject->wchrAlt = L'/';
    }

    if(id == MAPOBJECTID_HARPOON) // Harpoon
    {
        mapObject->description = "A harpoon.";
        mapObject->details = "[AMMO]";
        mapObject->layer = 2;
        mapObject->name = "harpoon";
        mapObject->flags |= (MAPOBJECTFLAG_ISITEM | MAPOBJECTFLAG_PLACEINROOM);
        mapObject->wchr = L'{';
        mapObject->wchrAlt = L'{';
    }

    if(id == MAPOBJECTID_HARPOONGUN) // Harpoon gun
    {
        mapObject->attack = 2;
        mapObject->attackToHit = 1;
        mapObject->consumesItemWhenUsedID = MAPOBJECTID_HARPOON;
        mapObject->description = "A harpoon gun.";
        mapObject->details = "[ATT: 2 +1]";
        mapObject->equipAt = MAPOBJECTEQUIPAT_WEAPON;
        mapObject->layer = 2;
        mapObject->name = "harpoon gun";
        mapObject->flags |= (MAPOBJECTFLAG_ISEQUIPMENT | MAPOBJECTFLAG_ITEMISRANGED | MAPOBJECTFLAG_ISITEM | MAPOBJECTFLAG_PLACEINROOM);
        mapObject->wchr = L'}';
        mapObject->wchrAlt = L'}';
    }

    if(id == MAPOBJECTID_LIFEVEST) // Lifevest
    {
        mapObject->defense = 2;
        mapObject->description = "A lifevest.";
        mapObject->details = "[DEF: 2]";
        mapObject->equipAt = MAPOBJECTEQUIPAT_BODY;
        mapObject->layer = 2;
        mapObject->name = "lifevest";
        mapObject->flags |= (MAPOBJECTFLAG_ISEQUIPMENT | MAPOBJECTFLAG_ISITEM | MAPOBJECTFLAG_PLACEINROOM);
        mapObject->wchr = L'(';
        mapObject->wchrAlt = L'(';
    }

    if(id == MAPOBJECTID_MANGYDOG) // Mangy dog
    {
        mapObject->attackBase = 2;
        mapObject->attackDistance = 1;
        mapObject->attackToHitBase = -1;
        mapObject->attackVerbs[0] = "bite";
        mapObject->attackVerbs[1] = "bites";
        mapObject->colorPair = CONSOLECOLORPAIR_REDBLACK;
        mapObject->defenseBase = 2;
        mapObject->description = "A mangy dog.";
        mapObject->layer = 1;
        mapObject->name = "mangy dog";
        mapObject->flags |= (MAPOBJECTFLAG_BLOCKSSOLID | MAPOBJECTFLAG_CANATTACK | MAPOBJECTFLAG_CANMOVE | MAPOBJECTFLAG_ISHOSTILE | MAPOBJECTFLAG_ISLIVING | MAPOBJECTFLAG_PLACEINROOM);
        mapObject->hp = 4;
        mapObject->hpMaxBase = 4;
        mapObject->o2 = 8;
        mapObject->o2MaxBase = 8;
        mapObject->sight = 15;
        mapObject->turnTicks = 10;
        mapObject->turnTicksSize = 10;
        mapObject->wchr = L'd';
        mapObject->wchrAlt = L'd';
    }

    if(id == MAPOBJECTID_SCUBAMASK) // Scuba mask
    {
        mapObject->defense = 1;
        mapObject->description = "A scuba mask.";
        mapObject->details = "[DEF: 1, O2: +10]";
        mapObject->equipAt = MAPOBJECTEQUIPAT_FACE;
        mapObject->layer = 2;
        mapObject->name = "scuba mask";
        mapObject->o2 = 10;
        mapObject->flags |= (MAPOBJECTFLAG_ISEQUIPMENT | MAPOBJECTFLAG_ISITEM | MAPOBJECTFLAG_ITEMINCREASE02 | MAPOBJECTFLAG_PLACEINROOM);
        mapObject->wchr = L'(';
        mapObject->wchrAlt = L'(';
    }

    if(id == MAPOBJECTID_SCUBATANK) // Scuba tank
    {
        mapObject->defense = 1;
        mapObject->description = "A scuba tank.";
        mapObject->details = "[DEF: 1, O2: +%d / 500]";
        mapObject->equipAt = MAPOBJECTEQUIPAT_BACK;
        mapObject->layer = 2;
        mapObject->name = "scuba tank";
        mapObject->o2 = 500;
        mapObject->flags |= (MAPOBJECTFLAG_ISEQUIPMENT | MAPOBJECTFLAG_ISITEM | MAPOBJECTFLAG_ITEMINCREASE02 | MAPOBJECTFLAG_ITEMSUPPLY02 | MAPOBJECTFLAG_PLACEINROOM);
        mapObject->wchr = L'[';
        mapObject->wchrAlt = L'[';
    }

    if(id == MAPOBJECTID_SMALLEEL) // Small eel
    {
        mapObject->attackBase = 1;
        mapObject->attackDistance = 1;
        mapObject->attackToHitBase = 1;
        mapObject->attackVerbs[0] = "nip";
        mapObject->attackVerbs[1] = "nips";
        mapObject->colorPair = CONSOLECOLORPAIR_GREENBLACK;
        mapObject->defenseBase = 1;
        mapObject->description = "A small eel.";
        mapObject->layer = 1;
        mapObject->name = "small eel";
        mapObject->flags |= (MAPOBJECTFLAG_BLOCKSSOLID | MAPOBJECTFLAG_CANATTACK | MAPOBJECTFLAG_CANMOVE | MAPOBJECTFLAG_ISAQUATIC | MAPOBJECTFLAG_ISHOSTILE | MAPOBJECTFLAG_ISLIVING | MAPOBJECTFLAG_PLACEINWATER);
        mapObject->hp = 3;
        mapObject->hpMaxBase = 3;
        mapObject->sight = 10;
        mapObject->turnTicks = 9;
        mapObject->turnTicksSize = 9;
        mapObject->wchr = L'e';
        mapObject->wchrAlt = L'e';
    }

    if(id == MAPOBJECTID_TIGERFISH) // Tigerfish
    {
        mapObject->attackBase = 2;
        mapObject->attackDistance = 1;
        mapObject->attackToHitBase = 1;
        mapObject->attackVerbs[0] = "bite";
        mapObject->attackVerbs[1] = "bites";
        mapObject->colorPair = CONSOLECOLORPAIR_GREENBLACK;
        mapObject->defenseBase = 1;
        mapObject->description = "A tigerfish.";
        mapObject->layer = 1;
        mapObject->name = "tigerfish";
        mapObject->flags |= (MAPOBJECTFLAG_BLOCKSSOLID | MAPOBJECTFLAG_CANATTACK | MAPOBJECTFLAG_CANMOVE | MAPOBJECTFLAG_ISAQUATIC | MAPOBJECTFLAG_ISHOSTILE | MAPOBJECTFLAG_ISLIVING | MAPOBJECTFLAG_PLACEINWATER);
        mapObject->hp = 5;
        mapObject->hpMaxBase = 5;
        mapObject->sight = 10;
        mapObject->turnTicks = 12;
        mapObject->turnTicksSize = 12;
        mapObject->wchr = L'f';
        mapObject->wchrAlt = L'f';
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
        mapObject->wchrAlt = L'???';
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
        mapObject->wchr = L'???';
        mapObject->wchrAlt = L'???';
    }

    if(id == MAPOBJECTID_STAIRS) // Stairs
    {
        mapObject->description = "Stairs.";
        mapObject->layer = 2;
        mapObject->name = "stairs";
        mapObject->flags |= (MAPOBJECTFLAG_PLACEINROOM | MAPOBJECTFLAG_RENDERALWAYSAFTERSEEN | MAPOBJECTFLAG_STAIRS);
        mapObject->wchr = L'<';
        mapObject->wchrAlt = L'<';
    }

    if(mapObject->flags & MAPOBJECTFLAG_ISLIVING)
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
        MapTile_Destroy(map->tiles[i]);
    free(map->tiles);
}

Path *Map_FindPath(Map *map, Point2D from, Point2D to, uint8_t flags)
{
    Path *path = malloc(sizeof(Path));
    path->complete = false;
    path->length = 0;

    if(Map_GetTile(map, from) == NULL || Map_GetTile(map, to) == NULL)
        return path;
    if(from.x == to.x && from.y == to.y) return path;

    Direction2D directions[8];
    directions[0] = (Direction2D){ 0, -1 };
    directions[1] = (Direction2D){ 1, -1 };
    directions[2] = (Direction2D){ 1, 0 };
    directions[3] = (Direction2D){ 1, 1 };
    directions[4] = (Direction2D){ 0, 1 };
    directions[5] = (Direction2D){ -1, 1 };
    directions[6] = (Direction2D){ -1, 0 };
    directions[7] = (Direction2D){ -1, -1 };

    int timeout = 1000;
    while((from.x != to.x || from.y != to.y) && timeout > 0)
    {
        timeout--;

        path->nodes[path->length] = from;
        path->length++;
        if(path->length == 1000) return path;

        int directionIndex = -1;
        if(from.x == to.x && from.y > to.y) directionIndex = 0;
        if(from.x < to.x && from.y > to.y) directionIndex = 1;
        if(from.x < to.x && from.y == to.y) directionIndex = 2;
        if(from.x < to.x && from.y < to.y) directionIndex = 3;
        if(from.x == to.x && from.y < to.y) directionIndex = 4;
        if(from.x > to.x && from.y < to.y) directionIndex = 5;
        if(from.x > to.x && from.y == to.y) directionIndex = 6;
        if(from.x > to.x && from.y > to.y) directionIndex = 7;

        if(directionIndex == -1) return path;

        int directionIndices[2];
        directionIndices[0] = directionIndex;
        directionIndices[1] = directionIndex;
        Point2D nexts[2];
        nexts[0] = (Point2D){ -1, -1 };
        nexts[1] = (Point2D){ -1, -1 };
        int attemps[2];
        attemps[0] = 0;
        attemps[1] = 0;

        for(int i = 0; i < 2; i++)
        {
            for(int j = 0; j < 8; j++)
            {
                attemps[i]++;
                nexts[i] = (Point2D){ from.x + directions[directionIndices[i]].x, from.y + directions[directionIndices[i]].y };
                bool pass = true;

                if(Path_Find(path, nexts[i]) > -1) 
                    pass = false;
                else
                {
                    MapTile *tile = Map_GetTile(map, nexts[i]);
                    if(tile == NULL)
                        pass = false;
                    else
                    {
                        if(!(tile->passable & MAPTILEPASSABLE_SOLID))
                        {
                            if(!(nexts[i].x == to.x && nexts[i].y == to.y && (flags & PATHFINDINGFLAG_IGNORETOPASSABLE)))
                                pass = false;
                        }
                    }
                }

                if(pass) break;

                directionIndices[i] += (i == 0) ? 1 : -1;
                if(directionIndices[i] > 7) directionIndices[i] = 0;
                if(directionIndices[i] < 0) directionIndices[i] = 7;
                nexts[i] = (Point2D){ -1, -1 };
            }
        }

        if(nexts[0].x == -1 && nexts[1].x == -1) return path;
        if(nexts[0].x == -1)
            from = nexts[1];
        else
        {
            if(nexts[1].x == -1)
                from = nexts[0];
            else
            {
                if(attemps[0] <= attemps[1])
                    from = nexts[0];
                else
                    from = nexts[1];
            }
        }
    }

    path->complete = true;
    return path;
}

bool Map_LevelFloodTimerTick(Map *map)
{
    map->levelFloodTimer--;
    if(map->levelFloodTimer > 0) return false;

    map->levelFloodTimer = (int)map->levelFloodTimerSize;
    Map_PlaceObject(map, Map_CreateObject(map, MAPOBJECTID_WATERSOURCE));
    
    if(map->levelFloodTimerSize > 10) map->levelFloodTimerSize -= 7 + rand() % 3;
    map->levelFloodTimer = map->levelFloodTimerSize;

    int aquaticMonsterIDs[10];
    aquaticMonsterIDs[0] = MAPOBJECTID_SMALLEEL;
    aquaticMonsterIDs[1] = MAPOBJECTID_SMALLEEL;
    aquaticMonsterIDs[2] = MAPOBJECTID_SMALLEEL;
    aquaticMonsterIDs[3] = MAPOBJECTID_SMALLEEL;
    aquaticMonsterIDs[4] = MAPOBJECTID_SMALLEEL;
    aquaticMonsterIDs[5] = MAPOBJECTID_SMALLEEL;
    aquaticMonsterIDs[6] = MAPOBJECTID_TIGERFISH;
    aquaticMonsterIDs[7] = MAPOBJECTID_TIGERFISH;
    aquaticMonsterIDs[8] = MAPOBJECTID_TIGERFISH;
    aquaticMonsterIDs[9] = MAPOBJECTID_TIGERFISH;

    for(int i = 0; i < rand() % 3; i++)
        Map_PlaceObject(map, Map_CreateObject(map, aquaticMonsterIDs[rand() % 10]));

    return true;
}

void Map_Generate(Map *map)
{
    int fts = (120 + rand() % 10) - (map->level * 2);
    if(fts < 10) fts = 10;
    map->levelFloodTimerSize = (size_t)fts;
    map->levelFloodTimer = (int)map->levelFloodTimerSize;

    while(true)
    {
        bool generateEssentialComplete = false;
        int timeout = 1000;

        while(true)
        {
            for(int y = 0; y < map->size.height; y++)
            {
                for(int x = 0; x < map->size.width; x++)
                {
                    MapTile *tile = map->tiles[(y * map->size.width) + x];
                    MapTile_SetType(tile, MAPTILETYPE_WALL);
                    MapTile_DestroyObjects(tile);
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
            }

            bool toContinue = false;
            for(int i = 0; i < map->roomsCount; i++)
            {
                Rect2D *room = map->rooms[i];
                if(room->position.x < 2 || room->position.y < 2 || room->position.x + room->size.width >= map->size.width - 2 || room->position.y + room->size.height >= map->size.height - 2 || room->size.width < 3 || room->size.height < 3)
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

                /*
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
                */

                if(at.x == to.x)
                {
                    if(to.y > at.y) at.y++;
                    if(to.y < at.y) at.y--;
                }
                else
                {
                    if(at.x == to.x)
                    {
                        if(to.x > at.x) at.x++;
                        if(to.x < at.x) at.x--;
                    }
                    else
                    {
                        if(rand() % 10 >= 5)
                        {
                            if(to.x > at.x) at.x++;
                            if(to.x < at.x) at.x--;
                        }
                        else
                        {
                            if(to.y > at.y) at.y++;
                            if(to.y < at.y) at.y--;
                        }
                    }
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

        if(map->player == NULL)
        {
            map->player = Map_CreateObject(map, MAPOBJECTID_PLAYER);
            Map_PlaceObject(map, map->player);
        }

        for(int i = 0; i < 4 + rand() % 2; i++)
            Map_PlaceObject(map, Map_CreateObject(map, MAPOBJECTID_DOOR));

        MapObject *stairs = Map_CreateObject(map, MAPOBJECTID_STAIRS);
        Map_PlaceObject(map, stairs);

        while(timeout > 0)
        {
            timeout--;

            Path *path = Map_FindPath(map, map->player->position, stairs->position, 0);

            if(path->complete)
            {
                generateEssentialComplete = true;
                free(path);
                break;
            }

            free(path);
            MapTile_RemoveObject(Map_GetTile(map, stairs->position), stairs);
            Map_PlaceObject(map, stairs);
        }

        if(generateEssentialComplete) break;
    }

    int monsterIDs[10];
    monsterIDs[0] = MAPOBJECTID_BILGERAT;
    monsterIDs[1] = MAPOBJECTID_BILGERAT;
    monsterIDs[2] = MAPOBJECTID_BILGERAT;
    monsterIDs[3] = MAPOBJECTID_BILGERAT;
    monsterIDs[4] = MAPOBJECTID_BILGERAT;
    monsterIDs[5] = MAPOBJECTID_MANGYDOG;
    monsterIDs[6] = MAPOBJECTID_MANGYDOG;
    monsterIDs[7] = MAPOBJECTID_MANGYDOG;
    monsterIDs[8] = MAPOBJECTID_MANGYDOG;
    monsterIDs[9] = MAPOBJECTID_MANGYDOG;

    for(int i = 0; i < 3 + rand() % 2; i++)
        Map_PlaceObject(map, Map_CreateObject(map, monsterIDs[rand() % 10]));

    int itemIDs[10];
    itemIDs[0] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[1] = MAPOBJECTID_DIVEKNIFE;
    itemIDs[2] = MAPOBJECTID_HARPOON;
    itemIDs[3] = MAPOBJECTID_LIFEVEST;
    itemIDs[4] = MAPOBJECTID_LIFEVEST;
    itemIDs[5] = MAPOBJECTID_HARPOONGUN;
    itemIDs[6] = MAPOBJECTID_SCUBAMASK;
    itemIDs[7] = MAPOBJECTID_SCUBAMASK;
    itemIDs[8] = MAPOBJECTID_HARPOON;
    itemIDs[9] = MAPOBJECTID_SCUBATANK;

    for(int i = 0; i < 3 + rand() % 2; i++)
        Map_PlaceObject(map, Map_CreateObject(map, itemIDs[rand() % 10]));
}

MapObject *Map_GetClosestObjectWithFlags(Map *map, Point2D to, uint32_t flags)
{
    MapObject *closest = NULL;
    int dist = 1000;

    for(int y = 0; y < map->size.height; y++)
    {
        for(int x = 0; x < map->size.width; x++)
        {
            MapObject *obj = MapTile_GetObjectWithFlags(Map_GetTile(map, (Point2D){ x, y }), flags);
            if(obj == NULL) continue;
            int d = Map_GetSimpleDistance(map, obj->position, to);
            if(d >= dist) continue;

            closest = obj;
            dist = d;
        }
    }

    return closest;
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
    
    //????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????

    if(tile->type == MAPTILETYPE_FLOOR)
        //return (Map_GetRoomIndexContaining(map, (Point2D){ point.x, point.y }) > -1) ? L'.' : L'???';
        return L'.';
    if(tile->type == MAPTILETYPE_WALL)
    {
        wchar_t wchr = L' ';

        if(map->tiles[((point.y + 1) * map->size.width) + (point.x + 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x + 1, point.y + 1 }) > -1)
            wchr = L'???';
        if(map->tiles[((point.y + 1) * map->size.width) + (point.x - 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x - 1, point.y + 1 }) > -1)
            wchr = L'???';
        if(map->tiles[((point.y - 1) * map->size.width) + (point.x + 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x + 1, point.y - 1 }) > -1)
            wchr = L'???';
        if(map->tiles[((point.y - 1) * map->size.width) + (point.x - 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x - 1, point.y - 1 }) > -1)
            wchr = L'???';
        if((map->tiles[(point.y * map->size.width) + (point.x - 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x - 1, point.y }) > -1) || (map->tiles[(point.y * map->size.width) + (point.x + 1)]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x + 1, point.y }) > -1))
            wchr = L'???';
        if((map->tiles[((point.y - 1) * map->size.width) + point.x]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x , point.y - 1 }) > -1) || (map->tiles[((point.y + 1) * map->size.width) + point.x]->type == MAPTILETYPE_FLOOR && Map_GetRoomIndexContaining(map, (Point2D){ point.x , point.y + 1 }) > -1))
            wchr = L'???';

        return wchr;
    }

    return L' ';
}

wchar_t Map_GetPointWChrObjectFlags(Map *map, Point2D point, uint32_t flags)
{
    MapTile *tile = Map_GetTile(map, point);
    if(tile == NULL) return L' ';

    if((int)tile->objectsCount == 0)
        return Map_GetPointWChr(map, point);

    wchar_t wchr = L' ';
    int layer = 1000;

    for(int i = 0; i < (int)tile->objectsCount; i++)
    {
        MapObject *obj = tile->objects[i];

        if(!(obj->flags & flags)) continue;
        if(obj->layer >= layer) continue;

        wchr = obj->wchr;
        layer = obj->layer;
    }

    return wchr;
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

int Map_GetRoomIndexContainingBorder(Map *map, Point2D point, int border)
{
    for(int i = 0; i < map->roomsCount; i++)
    {
        if(point.x < map->rooms[i]->position.x - border || point.y < map->rooms[i]->position.y - border || point.x >= map->rooms[i]->position.x + map->rooms[i]->size.width + border || point.y >= map->rooms[i]->position.y + map->rooms[i]->size.height + border)
            continue;
        return i;
    }

    return -1;
}

int Map_GetSimpleDistance(Map *map, Point2D from, Point2D to)
{
    int distance = 0;
    while(from.x != to.x || from.y != to.y)
    {
        if(to.x > from.x) from.x++;
        if(to.x < from.x) from.x--;
        if(to.y > from.y) from.y++;
        if(to.y < from.y) from.y--;
        distance++;
    }

    return distance;
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
    mapObject->position = (Point2D){ -1, -1 };
    int timeout = 1000;

    while(timeout > 0)
    {
        timeout--;
        
        Rect2D *room = map->rooms[rand() % map->roomsCount];

        Point2D point = (Point2D){ -1, -1 };
        MapTile *tile = NULL;

        if(mapObject->flags & MAPOBJECTFLAG_PLACEINDOORWAYS)
        {
            point = (Point2D){ 1 + rand() % (map->size.width - 2), 1 + rand() % (map->size.height - 2) };
            if(Map_GetRoomIndexContainingBorder(map, point, 1) == -1) continue;
            tile = Map_GetTile(map, point);

            if(!(tile->passable & MAPTILEPASSABLE_SOLID)) continue;
            if(!((Map_GetTile(map, (Point2D){ point.x - 1, point.y })->type == MAPTILETYPE_WALL && Map_GetTile(map, (Point2D){ point.x + 1, point.y })->type == MAPTILETYPE_WALL) || (Map_GetTile(map, (Point2D){ point.x, point.y - 1 })->type == MAPTILETYPE_WALL && Map_GetTile(map, (Point2D){ point.x, point.y + 1 })->type == MAPTILETYPE_WALL)))
                continue;
        }
        else
        {
            if(mapObject->flags & MAPOBJECTFLAG_PLACEINROOM)
            {
                point = (Point2D){ room->position.x + 1 + rand() % (room->size.width - 2), room->position.y + 1 + rand() % (room->size.height - 2) };
            
                if(point.x < 1 || point.y < 1 || point.x >= map->size.width - 1 || point.y >= map->size.height - 1) 
                    continue;

                tile = Map_GetTile(map, point);
                
                if(!(tile->passable & MAPTILEPASSABLE_SOLID)) continue;
            }

            if(mapObject->flags & MAPOBJECTFLAG_PLACEINWATER)
            {
                if(tile == NULL)
                {
                    point = (Point2D){ 1 + rand() % (map->size.width - 2), 1 + rand() % (map->size.height - 2) };
                    tile = Map_GetTile(map, point);
                }
                
                if(!(tile->passable & MAPTILEPASSABLE_LIQUID)) continue;
                if(MapTile_GetObjectWithFlags(tile, MAPOBJECTFLAG_ISLIQUID) == NULL) continue;
            }
        }

        if(tile == NULL)
        {
            point = (Point2D){ 1 + rand() % (map->size.width - 2), 1 + rand() % (map->size.height - 2) };
            tile = Map_GetTile(map, point);
            if(!(tile->passable & MAPTILEPASSABLE_SOLID)) continue;
        }

        if(mapObject->flags & MAPOBJECTFLAG_ISHOSTILE)
        {
            if(Map_GetSimpleDistance(map, point, map->player->position) < 5)
                continue;
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
    //????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????????

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
                        {
                            if((int)tile->objectsCount > 0)
                            {
                                wchr = Map_GetPointWChrObjectFlags(map, mapPoint, MAPOBJECTFLAG_RENDERALWAYSAFTERSEEN);
                                if(wchr == L' ')
                                    wchr = (Map_GetRoomIndexContaining(map, mapPoint) > -1) ? L' ' : L'???';
                            }
                            else
                               wchr = (Map_GetRoomIndexContaining(map, mapPoint) > -1) ? L' ' : L'???'; 
                        }
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

    // CRUDE RAYCASTING SOLUTION:

    int viewLength = mapObject->sight * 2;

    for(int d = 0; d < 360; d++)
    {
        int vl = viewLength;
        for(int l = 0; l < vl; l++)
        {
            Point2D point = (Point2D){ round(mapObject->position.x + sin(d) * l), round(mapObject->position.y + cos(d) * l) };
            MapTile *tile = Map_GetTile(map, point);
            if(tile == NULL) break;

            if(Map_GetRoomIndexContaining(map, point) == -1) vl /= 2;

            mapObject->view[(point.y * map->size.width) + point.x] = MAPOBJECTVIEW_VISIBLE;

            if(!(tile->passable & MAPTILEPASSABLE_LIGHT)) break;
        }
    }

    /*
    // SIMPLE ROOM-BASED SOLUTION:

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
    mapObject->attackDistance = 0;
    mapObject->attackToHit = 0;
    mapObject->attackToHitBase = 0;
    mapObject->colorPair = CONSOLECOLORPAIR_WHITEBLACK;
    mapObject->consumesItemWhenUsedID = -1;
    mapObject->defense = 0;
    mapObject->defenseBase = 0;
    mapObject->description = "Nothing.";
    mapObject->details = "";
    mapObject->equipAt = -1;
    for(int i = 0; i < 2; i++)
        mapObject->equipment[i] = NULL;
    mapObject->flags = 0;
    mapObject->hp = 0;
    mapObject->hpMax = 0;
    mapObject->hpMaxBase = 0;
    mapObject->hpRecoverTimer = 0;
    mapObject->hpRecoverTimerLength = 0;
    for(int i = 0; i < 10; i++)
        mapObject->items[i] = NULL;
    mapObject->itemsCount = 0;
    mapObject->lastRoomIndex = -1;
    mapObject->name = name;
    mapObject->o2 = 0;
    mapObject->o2Max = 0;
    mapObject->o2MaxBase = 0;
    mapObject->sight = 0;
    mapObject->turnTicks = 0;
    mapObject->turnTicksSize = 0;
    mapObject->wchr = L' ';

    return mapObject;
}

void MapObject_Destroy(MapObject *mapObject)
{
    if(mapObject->flags & MAPOBJECTFLAG_HASINVENTORY)
    {
        for(int i = 0; i < (int)mapObject->itemsCount; i++)
            MapObjectAsItem_Destroy(mapObject->items[i]);
    }

    if(mapObject->flags & MAPOBJECTFLAG_ISLIVING)
    {
        if(mapObject->view != NULL)
            free(mapObject->view);
    }

    free(mapObject);
}

int MapObject_GetEquippedAt(MapObject *mapObject, MapObjectAsItem *item)
{
    for(int i = 0; i < MAPOBJECTEQUIPAT_SLOTSCOUNT; i++)
    {
        if(mapObject->equipment[i] != item) continue;

        return i;
    }

    return -1;
}

MapObjectAsItem *MapObject_GetItemByID(MapObject *mapObject, int itemID)
{
    for(int i = 0; i < mapObject->itemsCount; i++)
    {
        if(mapObject->items[i]->id != itemID) continue;
        return mapObject->items[i];
    }

    return NULL;
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
    item->consumesItemWhenUsedID = mapObject->consumesItemWhenUsedID;
    item->defense = mapObject->defense;

    item->description = malloc(sizeof(char) * (strlen(mapObject->description) + 1));
    memset(item->description, 0, (strlen(mapObject->description) + 1));
    strcpy(item->description, mapObject->description);

    item->details = malloc(sizeof(char) * (strlen(mapObject->details) + 1));
    memset(item->details, 0, (strlen(mapObject->details) + 1));
    strcpy(item->details, mapObject->details);

    item->equipAt = mapObject->equipAt;
    item->flags = mapObject->flags;
    item->hp = mapObject->hp;
    item->hpMax = mapObject->hpMax;
    item->id = mapObject->id;

    item->name = malloc(sizeof(char) * (strlen(mapObject->name) + 1));
    memset(item->name, 0, (strlen(mapObject->name) + 1));
    strcpy(item->name, mapObject->name);

    item->o2 = mapObject->o2;
    item->o2Max = mapObject->o2Max;

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
    mapObject->hpMax = mapObject->hpMaxBase;
    mapObject->o2Max = mapObject->o2MaxBase;

    for(int i = 0; i < mapObject->itemsCount; i++)
    {
        int equippedAt = MapObject_GetEquippedAt(mapObject, mapObject->items[i]);
        if(equippedAt == -1) continue;

        mapObject->attack += mapObject->items[i]->attack;
        mapObject->attackToHit += mapObject->items[i]->attackToHit;
        mapObject->defense += mapObject->items[i]->defense;
        mapObject->hpMax += mapObject->items[i]->hp;
        mapObject->o2Max += mapObject->items[i]->o2;
    }

    if(mapObject->hp > mapObject->hpMax) mapObject->hp = mapObject->hpMax;
    if(mapObject->o2 > mapObject->o2Max) mapObject->o2 = mapObject->o2Max;
}

void MapObject_UpdateAttributesExcludeItemsWithFlags(MapObject *mapObject, uint32_t flags)
{
    if(mapObject->flags & MAPOBJECTFLAG_ISITEM) return;

    mapObject->attack = mapObject->attackBase;
    mapObject->attackToHit = mapObject->attackToHitBase;
    mapObject->defense = mapObject->defenseBase;
    mapObject->hpMax = mapObject->hpMaxBase;
    mapObject->o2Max = mapObject->o2MaxBase;

    for(int i = 0; i < mapObject->itemsCount; i++)
    {
        int equippedAt = MapObject_GetEquippedAt(mapObject, mapObject->items[i]);
        if(equippedAt == -1) continue;
        if(mapObject->items[i]->flags & flags) continue;

        mapObject->attack += mapObject->items[i]->attack;
        mapObject->attackToHit += mapObject->items[i]->attackToHit;
        mapObject->defense += mapObject->items[i]->defense;
        mapObject->hpMax += mapObject->items[i]->hp;
        mapObject->o2Max += mapObject->items[i]->o2;
    }

    if(mapObject->hp > mapObject->hpMax) mapObject->hp = mapObject->hpMax;
    if(mapObject->o2 > mapObject->o2Max) mapObject->o2 = mapObject->o2Max;
}

void MapObject_UpdateItems(MapObject *mapObject)
{
    if(!(mapObject->flags & MAPOBJECTFLAG_HASINVENTORY)) return;

    for(int i = 0; i < mapObject->itemsCount; i++)
    {
        MapObjectAsItem *item = mapObject->items[i];
        if(item->flags & MAPOBJECTFLAG_ITEMSUPPLY02 && MapObject_GetEquippedAt(mapObject, item) != -1)
        {
            if(item->o2 > 0) item->o2--;
        }
    }
}

MapObjectAction *MapObjectAction_Create(int type)
{
    MapObjectAction *action = malloc(sizeof(MapObjectAction));

    action->object = NULL;
    action->objectItem = NULL;
    action->result = false;
    action->resultMessage = "";
    action->resultValueInt = 0;
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
    free(item->details);
    free(item);
}

void MapTile_AddObject(MapTile *tile, MapObject *mapObject)
{
    if(tile == NULL) return;
    if(tile->objectsCount == 10) return;

    for(int i = (int)tile->objectsCount; i > 0; i--)
        tile->objects[i] = tile->objects[i - 1];

    tile->objects[0] = mapObject;
    tile->objectsCount++;

    MapTile_UpdatePassable(tile);
}

void MapTile_Destroy(MapTile *tile)
{
    MapTile_DestroyObjects(tile);
    free(tile);
}

void MapTile_DestroyObject(MapTile *tile, MapObject *mapObject)
{
    int index = -1;
    for(int i = 0; i < tile->objectsCount; i++)
    {
        if(tile->objects[i] != mapObject) continue;
        index = i;
        break;
    }

    if(index == -1) return;

    MapObject *object = tile->objects[index];
    MapTile_RemoveObject(tile, object);
    MapObject_Destroy(object);
}

void MapTile_DestroyObjects(MapTile *tile)
{
    for(int i = 0; i < tile->objectsCount; i++)
        MapObject_Destroy(tile->objects[i]);
    tile->objectsCount = 0;
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
    if(tile == NULL) return;

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

int Path_Find(Path *path, Point2D point)
{
    for(int i = 0; i < (int)path->length; i++)
    {
        if(path->nodes[i].x == point.x && path->nodes[i].y == point.y)
            return i;
    }

    return -1;
}