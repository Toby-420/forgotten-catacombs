#include "../include/sys.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include "../include/game.h"

extern WINDOW *inventoryScreen;
extern WINDOW *information;
extern WINDOW *mainScreen;
extern Area area[NUM_AREAS];
extern Character mainCharacter;
extern int enemiesKilled;

const char enemyNames[6 /* Number of areas */][5 /* Number of enemy names per area */][24 /* Max characters per name */] = {
    {
        "Goblin",
        "Dire Rat",
        "Kobold",
        "Skeleton",
        "Minotaur"
    },
    {
        "Giant Lizard",
        "Hobgoblin",
        "Demon",
        "Ghoul",
        "Gnoll"
    },
    {
        "Zombie",
        "Gargoyle",
        "Brute",
        "Giant Spider",
        "Rogue"
    },
    {
        "Drow",
        "Troll",
        "Mummy",
        "Duergar",
        "Hill Giant"
    },
    {
        "Vampire",
        "Fire Giant",
        "Purple Worm",
        "Black Pudding",
        "Gelatinous Cube"
    },
    {
        "Carrion Crawler",
        "Black Dragon",
        "Blue Dragon",
        "Red Dragon",
        "Succubus"
    }
};
const char roomEntryLines[10][150] = {
    "As you step into room %d, a fierce %s confronts you, launching an attack.",
    "Upon entering room %d, a menacing %s fixes its gaze on you, initiating an assault.",
    "Room %d reveals a lurking %s that swiftly attacks as you enter.",
    "Stepping into room %d, you're met with a hostile %s ready to strike.",
    "Room %d harbors a threatening %s that lunges at you upon arrival.",
    "You enter room %d, only to face a fearsome %s attacking without warning.",
    "In room %d, a dangerous %s stands before you, prepared to attack.",
    "As you arrive in room %d, a %s immediately attacks, catching you off guard.",
    "Entering room %d, a hostile %s rushes towards you, intent on attacking.",
    "Room %d greets you with a surprise as a %s launches an ambush."
};
const char itemNames[5][12] = {"Coin", "Small Gem", "Medium Gem", "Large Gem", "Key"};
const int itemValues[5] = {1, 2, 3, 4, 0};
const char locationLines[10][80] = {
	"You stand in the %s, facing an array of doors encircling you.",
	"Cautiously, you explore the %s, searching for potential dangers.",
	"Odd noises echo from a door within the %s, sending chills down your spine.",
	"The air feels tense in the %s as you examine the numerous surrounding doors.",
	"Surveying the %s, you notice a dim light flickering from one of the doors.",
	"Within the %s, a musty scent lingers, adding to the sense of mystery.",
	"Your footsteps echo in the vastness of the %s, emphasizing its emptiness.",
	"The %s feels foreboding as you inspect the doors scattered around.",
	"You cautiously step deeper into the %s, a sense of unease creeping in.",
	"Shadows dance across the walls of the %s, creating an eerie ambiance."
};

extern FILE *logFile;

ProgramData program = {
	"The Forgotten Catacombs",
	"v0.2",
	"The Forgotten Catacombs"
};

int initialiseRooms(void) {
	srand(time(NULL));
	for (int areaNumber = 0; areaNumber < NUM_AREAS; areaNumber++) {
		switch (areaNumber) {
			case GREAT_HALL:
				area[areaNumber].level = 1;
				strcpy(area[areaNumber].name, "Great Hall");
				break;
			case GUARD_ROOM:
				area[areaNumber].level = 2;
				strcpy(area[areaNumber].name, "Guard Room");
				break;
			case KITCHEN:
				area[areaNumber].level = 2;
				strcpy(area[areaNumber].name, "Kitchen");
				break;
			case PANTRY:
				area[areaNumber].level = 3;
				strcpy(area[areaNumber].name, "Pantry");
				break;
			case ARMORY:
				area[areaNumber].level = 3;
				strcpy(area[areaNumber].name, "Armory");
				break;
			case CELLS:
				area[areaNumber].level = 4;
				strcpy(area[areaNumber].name, "Cells");
				break;
			case CHAPEL:
				area[areaNumber].level = 4;
				strcpy(area[areaNumber].name, "Chapel");
				break;
			case THE_HOLE:
				area[areaNumber].level = 5;
				strcpy(area[areaNumber].name, "'The Hole'");
				break;
			case TORTURE_CHAMBER:
				area[areaNumber].level = 5;
				strcpy(area[areaNumber].name, "Torture Chamber");
				break;
			case LABORATORY:
				area[areaNumber].level = 5;
				strcpy(area[areaNumber].name, "Laboratory");
				break;
			case CRYPT:
				area[areaNumber].level = 5;
				strcpy(area[areaNumber].name, "Crypt");
				break;
			case THE_LAIR:
				area[areaNumber].level = 6;
				strcpy(area[areaNumber].name, "The Lair");
				break;
			case THE_BURROW:
				area[areaNumber].level = 6;
				strcpy(area[areaNumber].name, "The Burrow");
				break;
			default:
				fprintf(logFile, "Area not available: %d. Last area number: %d\n", areaNumber, THE_BURROW);
				break;
		}
		
		for (int j = 0; j < 8; j++) {
			int randomItem = rand() % 4;
			
			area[areaNumber].room[j].hasTreasure 					= true;
			area[areaNumber].room[j].hasEnemy 						= true;
			
			strcpy(area[areaNumber].room[j].roomEnemy.name, enemyNames[area[areaNumber].level - 1][rand() % 5]);

			area[areaNumber].room[j].roomEnemy.stamina 		= j + 5;
			area[areaNumber].room[j].roomEnemy.sex 				= rand() % 2;
			
			area[areaNumber].room[j].roomEnemy.skills 		= (Skills){(rand() % (j + 1)) + 4, (rand() % (j + 1)) + 4, (rand() % (j + 1)) + 4, (rand() % (j + 1)) + 4, (rand() % (j + 1)) + 4, (rand() % (j + 1)) + 4};
			
			strcpy(area[areaNumber].room[j].roomEnemy.inventory.name, itemNames[randomItem]);
			
			area[areaNumber].room[j].roomEnemy.inventory.value	  = itemValues[randomItem];
			area[areaNumber].room[j].roomEnemy.inventory.quantity = (rand() % 3) + 1;
			area[areaNumber].room[j].roomEnemy.inventory.ID 			= randomItem;
		}
	}

	int num = rand() % 8;
	strcpy(area[THE_BURROW].room[num].roomEnemy.inventory.name, itemNames[KEY]);
	area[THE_BURROW].room[num].roomEnemy.inventory.value 		 = itemValues[KEY];
	area[THE_BURROW].room[num].roomEnemy.inventory.quantity  = 1;
	area[THE_BURROW].room[num].roomEnemy.inventory.ID 			 = KEY;

	return 0;
}

void setupUi(bool clearing) {
	if (clearing) {
		wclear(mainScreen);
		wclear(inventoryScreen);
		wclear(information);
		clear();
		refresh();
	}
	
	mvwprintw(information, 1, 2, "NAME: %s\n  LOCATION: %s\n  SEX: %s\n  STAMINA: %d\n  STR: %d\n  MYS: %d\n  DEX: %d\n  RES: %d\n  CHA: %d\n  INS: %d\n", mainCharacter.name, mainCharacter.locationChar, mainCharacter.sex ? "Female" : "Male", mainCharacter.stamina,  mainCharacter.skills.strength, mainCharacter.skills.mystic, mainCharacter.skills.dexterity, mainCharacter.skills.resistance, mainCharacter.skills.charisma, mainCharacter.skills.insight);
	mvwprintw(inventoryScreen, 1, 2, "ITEM\t\t\t\tVALUE\t\tQUANTITY");
	int inventoryRow = 0; // Track the row position separately

	for (int i = 0; i < MAX_ITEMS; i++) {
		if (mainCharacter.inventory[i].quantity != 0) {
			wmove(inventoryScreen, inventoryRow + 2, 2);
			wprintw(inventoryScreen, "%s", mainCharacter.inventory[i].name);
			wmove(inventoryScreen, inventoryRow + 2, 14);
			wprintw(inventoryScreen, "%d", mainCharacter.inventory[i].value);
			wmove(inventoryScreen, inventoryRow + 2, 22);
			wprintw(inventoryScreen, "%d", mainCharacter.inventory[i].quantity);

			inventoryRow++; // Increment row only when printing an item
		}
	}


	box(mainScreen, 0, 0);
	mvwprintw(mainScreen, 0, 5, "MAIN GAME SCREEN");
	wrefresh(mainScreen);
	box(inventoryScreen, 0, 0);
	mvwprintw(inventoryScreen, 0, 5, "INVENTORY");
	wrefresh(inventoryScreen);
	box(information, 0, 0);
	mvwprintw(information, 0, 5, "INFORMATION");
	wrefresh(information);

	return;
}

void gameFinish(void) {
	setupUi(true);
	mvwprintw(mainScreen, 2, 3, "You finished the game");
	mvwprintw(mainScreen, 3, 3, "Press enter to continue...");
	wrefresh(mainScreen);
	getch();
	mvwprintw(mainScreen, 2, 3, "You acquired (and kept):\n   %d Coins\n   %d Small Gems\n   %d Medium Gems\n   %d Large Gems\n   And a Key\n\n   You killed %d enemies along the way", mainCharacter.inventory[COIN].quantity, mainCharacter.inventory[SMALL_GEM].quantity, mainCharacter.inventory[MEDIUM_GEM].quantity, mainCharacter.inventory[LARGE_GEM].quantity, enemiesKilled);
	setupUi(false);
	wrefresh(mainScreen);
	getch();
}