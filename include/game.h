#ifndef GAME_H
#define GAME_H

#include "../include/characters.h"

#define NO_ROOM -1

typedef enum { GREAT_HALL, GUARD_ROOM, KITCHEN, PANTRY, ARMORY, CELLS, CHAPEL, THE_HOLE, TORTURE_CHAMBER, LABORATORY, CRYPT, THE_LAIR, THE_BURROW, NUM_AREAS } AREAS;
typedef enum { COIN, SMALL_GEM, MEDIUM_GEM, LARGE_GEM, KEY } ITEMS;
typedef enum { MALE, FEMALE } SEXES;


typedef struct {
	bool hasTreasure;
	bool hasEnemy;
	Enemy roomEnemy;
} Room;

typedef struct {
	char name[16];
	int level;
	Room room[8];
} Area;

#endif
