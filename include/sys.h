#ifndef SYS_H
#define SYS_H

#define MAX_ITEMS 8
#define MAX_NAME_LENGTH 24

#define ROOM(n) n-1

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	const char name[MAX_NAME_LENGTH];
	const char version[8];
	const char title[24];
} ProgramData;

typedef struct {
	bool gameMode; // Activates hidden mode ;)
	uint8_t screen;
} SystemVars;

typedef struct {
	char name[MAX_NAME_LENGTH];
	int value;
	int quantity;
	int ID;
} Item;

int initialiseRooms(void);

void setupUi(bool clearing);

void addItem(const char *name, int price, int quantity);

void gameFinish(void);

#endif
