#ifndef CHARACTERS_H
#define CHARACTERS_H

#include "../include/sys.h"

typedef struct {
	int strength;
	int mystic;
	int dexterity;
	int resistance;
	int charisma;
	int insight;
} Skills; 

typedef struct {
	char name[16];
	int stamina;
	bool sex;
	int location;
	int currentRoom;
	char locationChar[16];
	Skills skills;
	Item inventory[MAX_ITEMS];
} Character;

typedef struct {
	char name[16];
	int stamina;
	bool sex;
	Skills skills;
	Item inventory;
} Enemy;

#endif