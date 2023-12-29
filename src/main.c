#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <SDL2/SDL_mixer.h>
#include <ctype.h>
#include <windows.h>

#include "../include/sys.h"
#include "../include/characters.h"
#include "../include/game.h"


Mix_Music *backgroundMusic = NULL;

Mix_Chunk *buttonMoveSound = NULL;
Mix_Chunk *buttonSelectSound = NULL;
Mix_Chunk *upgradeCharacterSound = NULL;

WINDOW *inventoryScreen;
WINDOW *information;
WINDOW *mainScreen;

Area area[NUM_AREAS];

FILE *logFile = NULL;

int itemCount = 0;
int option;
int enemiesKilled = 0;

Character mainCharacter;
SystemVars gameSystem;

extern ProgramData program;

extern const char locationLines[10][80];
extern const char roomEntryLines[10][150];

int main(int argc, char *argv[]) {
  HWND consoleWindow = GetConsoleWindow();
  ShowWindow(consoleWindow, SW_MAXIMIZE);
	
	LONG_PTR style = GetWindowLongPtr(consoleWindow, GWL_STYLE);
	style &= ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX);

	SetWindowLongPtr(consoleWindow, GWL_STYLE, style); // Set the modified style

	SetWindowPos(consoleWindow, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

  SetConsoleTitle(program.title);
	
	logFile = fopen("log.txt", "a");
	
	initscr();
  keypad(stdscr, TRUE);
  curs_set(1);
  set_tabsize(2);
	
	inventoryScreen = newwin(LINES / 3 + 3, COLS / 4, 0, (COLS * 3 / 4) - 1);
	information 		= newwin(LINES / 3 + 3, COLS / 4, LINES / 3 + 3, (COLS * 6 / 8) - 1);
	mainScreen 			= newwin(LINES, COLS / 1.5, 0, 0);

	setupUi(true);
	
	
	int chosenOption = -1;
	int currentSelection = 0;
	bool exitFlag = false;
	uint8_t inputRow = 0;
	strcpy(mainCharacter.locationChar, "Great Hall");
	
	if (initialiseRooms() == 0) {
		fprintf(logFile, "Room initialisation complete\n");
	} else {
		fprintf(logFile, "Room initialisation failed\n");
	}
	
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		fprintf(logFile, "SDL_Mixer failed to load\n");
    return 1;
  } else {
		fprintf(logFile, "SDL_Mixer loaded without issue\n");
	}
	
  backgroundMusic 		= Mix_LoadMUS("assets/audio/backgroundMusic.mp3");
  buttonMoveSound 		= Mix_LoadWAV("assets/audio/buttonMove.mp3");
  buttonSelectSound 	= Mix_LoadWAV("assets/audio/buttonSelect.mp3");
  upgradeCharacterSound 	= Mix_LoadWAV("assets/audio/upgradeCharacter.mp3");
	
	if (backgroundMusic == NULL || buttonMoveSound == NULL || buttonSelectSound == NULL || upgradeCharacterSound == NULL) {
		fprintf(logFile, "At least one sound file failed to load\n");
		return 1;
  } else {
		fprintf(logFile, "All audio loaded successfully\n");
	}
	
	mvwprintw(mainScreen, 2, 3, "Use left arrow key to remove characters");
	
	while (1) {
		mvwprintw(mainScreen, 3, 3, "What do you want to be called? ");
		wgetnstr(mainScreen, mainCharacter.name, 15);
		noecho();
		curs_set(false);
		wrefresh(mainScreen);

		if (mainCharacter.name[0] == '\0' || isspace(mainCharacter.name[0])) {} 
		else {
			Mix_PlayChannel(-1, buttonSelectSound, 0);
			break;
		}
	}
	
	// Set only first character in name to be uppercase
	mainCharacter.name[0] = toupper(mainCharacter.name[0]);
	
	for (int i = 1; i < (sizeof(mainCharacter.name) / sizeof(char)) - 1; i++) {
		// Set every character in the name other than first to lowercase
		mainCharacter.name[i] = tolower(mainCharacter.name[i]);
	}
	
	
	// Small arrow key based selection
	mvwprintw(stdscr, 6, 3, "Select character sex:");
	while (chosenOption == -1) {
		for (int i = 0; i < 2; i++) {
			if (i == currentSelection) {
				attron(A_UNDERLINE);
			}

			if (i == 0) {
				mvwprintw(stdscr, i + 7, 3, "Male");
			} else if (i == 1) {
				mvwprintw(stdscr, i + 7, 3, "Female");
			}
			attroff(A_UNDERLINE);
		}

		int ch = getch();

		switch (ch) {
			case KEY_UP:
        Mix_PlayChannel(-1, buttonMoveSound, 0);
				currentSelection = (currentSelection + 1) % 2;
				break;
			case KEY_DOWN:
        Mix_PlayChannel(-1, buttonMoveSound, 0);
				currentSelection = (currentSelection + 1) % 2;
				break;
			case '\n':
        Mix_PlayChannel(-1, buttonSelectSound, 0);
				mainCharacter.sex = currentSelection;
				chosenOption = currentSelection;
				break;
			default:
        Mix_PlayChannel(-1, buttonMoveSound, 0);
				break;
		}
	}
		
	mainCharacter.skills 	= (Skills){(rand() % 5) + 5, (rand() % 5) + 5, (rand() % 5) + 5, (rand() % 5) + 5, (rand() % 5) + 5, (rand() % 5) + 5};
	mainCharacter.stamina	= 50;
	mainCharacter.location = GREAT_HALL;
	mainCharacter.currentRoom = NO_ROOM;
	chosenOption = -1;
	currentSelection = 0;
  
	Mix_PlayMusic(backgroundMusic, -1);

	while (exitFlag != true) {
entryPoint:
		currentSelection = 0;
		setupUi(true);
		inputRow = 2;
		int randomLine = rand() % 10;
		mvwprintw(mainScreen, inputRow, 3, locationLines[randomLine], area[mainCharacter.location].name);
		wrefresh(mainScreen);
		
		mvwprintw(stdscr, 6, 3, "What do you do?");
		while (chosenOption == -1) {
			for (int i = 0; i < 4; i++) {
				if (i == currentSelection) {
					attron(A_UNDERLINE);
				}

				if (i == 0) {
					mvwprintw(stdscr, i + 7, 3, "Explore the areas");
				} else if (i == 1) {
					mvwprintw(stdscr, i + 7, 3, "Enter a room");
				} else if (i == 2) {
					mvwprintw(stdscr, i + 7, 3, "Upgrade character");
				} else if (i == 3 && mainCharacter.location == GREAT_HALL) {
					mvwprintw(stdscr, i + 7, 3, "Exit (Requires: Key)");
				} else if (i == 3 && mainCharacter.location != GREAT_HALL) {
					mvwprintw(stdscr, i + 7, 3, "Go to Great Hall to exit");
				}
				attroff(A_UNDERLINE);
			}

			int ch = getch();

			switch (ch) {
				case KEY_UP:
					Mix_PlayChannel(-1, buttonMoveSound, 0);
					currentSelection = (currentSelection - 1) % 4;
					if (currentSelection < 0) {
						currentSelection = 3;
					}
					break;
				case KEY_DOWN:
					Mix_PlayChannel(-1, buttonMoveSound, 0);
					currentSelection = (currentSelection + 1) % 4;
					break;
				case '\n':
					Mix_PlayChannel(-1, buttonSelectSound, 0);
					option = currentSelection;
					chosenOption = currentSelection;
					if (option == 3 && mainCharacter.inventory[KEY].quantity != 1 && mainCharacter.location == GREAT_HALL) {
						mvwprintw(stdscr, 11, 3, "At least you tried, right?");
						chosenOption = -1;
					} if (option == 3 && mainCharacter.inventory[KEY].quantity != 1 && mainCharacter.location != GREAT_HALL) {
						chosenOption = -1;
					} else if (option == 3 && mainCharacter.inventory[KEY].quantity == 1 && mainCharacter.location == GREAT_HALL) {
						chosenOption = -1;
						gameFinish();
						goto exitSequence;
					}
					break;
				default:
					Mix_PlayChannel(-1, buttonMoveSound, 0);
					break;
			}
		}
		
		chosenOption = -1;
		setupUi(true);
		mvwprintw(mainScreen, inputRow, 3, locationLines[randomLine], area[mainCharacter.location].name);
		wrefresh(mainScreen);
		mvwprintw(stdscr, 6, 3, "Where do you go?");
		switch (option) {
			case 0: // To explore areas in the dungeon 
				currentSelection = 0;
				while (chosenOption == -1) {
					for (int i = 0; i < NUM_AREAS + 1; i++) {
						if (i == currentSelection) {
							attron(A_UNDERLINE);
						}
						
						switch (i) {
							case GREAT_HALL:
								mvwprintw(stdscr, i + 7, 3, "Great Hall");
								break;
							case GUARD_ROOM:
								mvwprintw(stdscr, i + 7, 3, "Guard Room");
								break;
							case KITCHEN:
								mvwprintw(stdscr, i + 7, 3, "Kitchen");
								break;
							case PANTRY:
								mvwprintw(stdscr, i + 7, 3, "Pantry");
								break;
							case ARMORY:
								mvwprintw(stdscr, i + 7, 3, "Armory");
								break;
							case CELLS:
								mvwprintw(stdscr, i + 7, 3, "Cells");
								break;
							case CHAPEL:
								mvwprintw(stdscr, i + 7, 3, "Chapel");
								break;
							case THE_HOLE:
								mvwprintw(stdscr, i + 7, 3, "'The Hole'");
								break;
							case TORTURE_CHAMBER:
								mvwprintw(stdscr, i + 7, 3, "Torture Chamber");
								break;
							case LABORATORY:
								mvwprintw(stdscr, i + 7, 3, "Laboratory");
								break;
							case CRYPT:
								mvwprintw(stdscr, i + 7, 3, "Crypt");
								break;
							case THE_LAIR:
								mvwprintw(stdscr, i + 7, 3, "The Lair");
								break;
							case THE_BURROW:
								mvwprintw(stdscr, i + 7, 3, "The Burrow");
								break;
							case 13:
								mvwprintw(stdscr, i + 7, 3, "Back");
								break;
							default:
								break;
						}
						attroff(A_UNDERLINE);
					}

					int ch = getch();

					switch (ch) {
						case KEY_UP:
							Mix_PlayChannel(-1, buttonMoveSound, 0);
							currentSelection = (currentSelection - 1) % NUM_AREAS;
							if (currentSelection < 0) {
								currentSelection = NUM_AREAS;
							}
							break;
						case KEY_DOWN:
							Mix_PlayChannel(-1, buttonMoveSound, 0);
							currentSelection = (currentSelection + 1) % (NUM_AREAS + 1);
							break;
						case '\n':
							Mix_PlayChannel(-1, buttonSelectSound, 0);
							chosenOption = currentSelection;
							if (chosenOption == 13) {
								chosenOption = -1;
								goto entryPoint;
							}	else if (chosenOption == mainCharacter.location) {
								mvwprintw(stdscr, 22, 3, "Cannot travel to current location");
								chosenOption = -1; // Cannot travel to current location
							}
							break;
						default:
							Mix_PlayChannel(-1, buttonMoveSound, 0);
							break;
					}
			}
				mainCharacter.location = chosenOption;
				break;
			case 1:
				currentSelection = 0;
				while (chosenOption == -1) {
					for (int i = 0; i < 8 + 1; i++) {
						if (i == currentSelection) {
							attron(A_UNDERLINE);
						}
						
						switch (i) {
							case ROOM(1):
								mvwprintw(stdscr, i + 7, 3, "Room 1");
								break;
							case ROOM(2):
								mvwprintw(stdscr, i + 7, 3, "Room 2");
								break;
							case ROOM(3):
								mvwprintw(stdscr, i + 7, 3, "Room 3");
								break;
							case ROOM(4):
								mvwprintw(stdscr, i + 7, 3, "Room 4");
								break;
							case ROOM(5):
								mvwprintw(stdscr, i + 7, 3, "Room 5");
								break;
							case ROOM(6):
								mvwprintw(stdscr, i + 7, 3, "Room 6");
								break;
							case ROOM(7):
								mvwprintw(stdscr, i + 7, 3, "Room 7");
								break;
							case ROOM(8):
								mvwprintw(stdscr, i + 7, 3, "Room 8");
								break;
							case 8:
								mvwprintw(stdscr, i + 7, 3, "Back");
								break;
							default:
								break;
						}
						attroff(A_UNDERLINE);
					}

					int ch = getch();

					switch (ch) {
						case KEY_UP:
							Mix_PlayChannel(-1, buttonMoveSound, 0);
							currentSelection = (currentSelection - 1) % 9;
							if (currentSelection < 0) {
								currentSelection = 8;
							}
							break;
						case KEY_DOWN:
							Mix_PlayChannel(-1, buttonMoveSound, 0);
							currentSelection = (currentSelection + 1) % 9;
							break;
						case '\n':
							Mix_PlayChannel(-1, buttonSelectSound, 0);
							chosenOption = currentSelection;
							mainCharacter.currentRoom = currentSelection + 1;
							if (chosenOption == 8) {
								chosenOption = -1;
								goto entryPoint;
							}	else if (area[mainCharacter.location].room[chosenOption].roomEnemy.stamina == 0) {
								mvwprintw(stdscr, 18, 3, "Room already pilfered");
								chosenOption = -1; // Cannot travel to current location
								mainCharacter.currentRoom = -1;
							}
							break;
						default:
							Mix_PlayChannel(-1, buttonMoveSound, 0);
							break;
					}
				}
				// Enemy in the room fight
				setupUi(true);
				int randomEntryLine = rand() % 10;
				mvwprintw(mainScreen, inputRow, 3, roomEntryLines[randomEntryLine], mainCharacter.currentRoom, area[mainCharacter.location].room[mainCharacter.currentRoom].roomEnemy.name);
				mvwprintw(mainScreen, 6, 3, "What do you do?");
				wrefresh(mainScreen);
				if (chosenOption != 8) {
				chosenOption = -1;
				currentSelection = 0;
				while (chosenOption == -1) {
						for (int i = 0; i < 2; i++) {
							if (i == currentSelection) {
								attron(A_UNDERLINE);
							}

							switch (i) {
								case 0:
									mvwprintw(stdscr, i + 7, 3, "Fight (Pick stat)");
									break;
								case 1:
									mvwprintw(stdscr, i + 7, 3, "Run (DEX 7+)");
									break;
								default:
									break;
							}
							attroff(A_UNDERLINE);
						}

						int ch = getch();

						switch (ch) {
							case KEY_UP:
								Mix_PlayChannel(-1, buttonMoveSound, 0);
								currentSelection = (currentSelection + 1) % 2;
								break;
							case KEY_DOWN:
								Mix_PlayChannel(-1, buttonMoveSound, 0);
								currentSelection = (currentSelection + 1) % 2;
								break;
							case '\n':
								Mix_PlayChannel(-1, buttonSelectSound, 0);
								chosenOption = currentSelection;
								if (chosenOption == 1 && mainCharacter.skills.dexterity < 7) {
									mvwprintw(stdscr, 10, 3, "DEX not high enough");
									chosenOption = -1;
								}
								break;
							default:
								Mix_PlayChannel(-1, buttonMoveSound, 0);
								break;
						}
				}
				
				switch (chosenOption) {
					case 0:
						mvwprintw(mainScreen, 6, 3, "Battle with which stat?");
						wrefresh(mainScreen);
						chosenOption = -1;
						currentSelection = 0;
						while (chosenOption == -1) {
							for (int i = 0; i < 6; i++) {
								if (i == currentSelection) {
									attron(A_UNDERLINE);
								}
								

								switch (i) {
									case 0:
										mvwprintw(stdscr, i + 7, 3, "Beat with Strength");
										break;
									case 1:
										mvwprintw(stdscr, i + 7, 3, "Entrance with Mystic");
										break;
									case 2:
										mvwprintw(stdscr, i + 7, 3, "Confuse with Dexterity");
										break;
									case 3:
										mvwprintw(stdscr, i + 7, 3, "Tire with Resistance");
										break;
									case 4:
										mvwprintw(stdscr, i + 7, 3, "Fluster with Charisma");
										break;
									case 5:
										mvwprintw(stdscr, i + 7, 3, "Trick with insight");
										break;
									default:
										break;
								}
								attroff(A_UNDERLINE);
							}

							int ch = getch();

							switch (ch) {
								case KEY_UP:
									Mix_PlayChannel(-1, buttonMoveSound, 0);
									currentSelection = (currentSelection - 1) % 6;
									if (currentSelection < 0) {
										currentSelection = 5;
									}
									break;
								case KEY_DOWN:
									Mix_PlayChannel(-1, buttonMoveSound, 0);
									currentSelection = (currentSelection + 1) % 6;
									break;
								case '\n':
									Mix_PlayChannel(-1, buttonSelectSound, 0);
									chosenOption = currentSelection;
									
									switch (chosenOption) {
										case 0:
											if (mainCharacter.skills.strength <= area[mainCharacter.location].room[mainCharacter.currentRoom].roomEnemy.skills.strength) {
												mvwprintw(mainScreen, 14, 3, "Your strength is not high enough. Stamina -5");
												mainCharacter.stamina -= 5;
											} else {
												enemiesKilled += 1;
												mvwprintw(mainScreen, 14, 3, "You won the battle and stole items");
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.stamina = 0;
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].hasEnemy = 0;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].quantity += area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.quantity;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].value = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.value;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].ID = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID;
												strcpy(mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].name, area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.name);
											}
											break;
										case 1:
											if (mainCharacter.skills.mystic <= area[mainCharacter.location].room[mainCharacter.currentRoom].roomEnemy.skills.mystic) {
												mvwprintw(mainScreen, 14, 3, "Your mystic is not high enough. Stamina -5");
												mainCharacter.stamina -= 5;
											} else {
												enemiesKilled += 1;
												mvwprintw(mainScreen, 14, 3, "You won the battle and stole items");
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.stamina = 0;
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].hasEnemy = 0;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].quantity += area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.quantity;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].value = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.value;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].ID = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID;
												strcpy(mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].name, area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.name);
											}
											break;
										case 2:
											if (mainCharacter.skills.dexterity <= area[mainCharacter.location].room[mainCharacter.currentRoom].roomEnemy.skills.dexterity) {
												mvwprintw(mainScreen, 14, 3, "Your dexterity is not high enough. Stamina -5");
												mainCharacter.stamina -= 5;
											} else {
												enemiesKilled += 1;
												mvwprintw(mainScreen, 14, 3, "You won the battle and stole items");
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.stamina = 0;
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].hasEnemy = 0;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].quantity += area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.quantity;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].value = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.value;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].ID = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID;
												strcpy(mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].name, area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.name);
											}
											break;
										case 3:
											if (mainCharacter.skills.resistance <= area[mainCharacter.location].room[mainCharacter.currentRoom].roomEnemy.skills.resistance) {
												mvwprintw(mainScreen, 14, 3, "Your resistance is not high enough. Stamina -5");
												mainCharacter.stamina -= 5;
											} else {
												enemiesKilled += 1;
												mvwprintw(mainScreen, 14, 3, "You won the battle and stole items");
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.stamina = 0;
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].hasEnemy = 0;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].quantity += area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.quantity;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].value = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.value;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].ID = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID;
												strcpy(mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].name, area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.name);
											}
											break;
										case 4:
											if (mainCharacter.skills.charisma <= area[mainCharacter.location].room[mainCharacter.currentRoom].roomEnemy.skills.charisma) {
												mvwprintw(mainScreen, 14, 3, "Your charisma is not high enough. Stamina -5");
												mainCharacter.stamina -= 5;
											} else {
												enemiesKilled += 1;
												mvwprintw(mainScreen, 14, 3, "You won the battle and stole items");
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.stamina = 0;
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].hasEnemy = 0;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].quantity += area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.quantity;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].value = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.value;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].ID = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID;
												strcpy(mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].name, area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.name);
											}
											break;
										case 5:
											if (mainCharacter.skills.insight <= area[mainCharacter.location].room[mainCharacter.currentRoom].roomEnemy.skills.insight) {
												mvwprintw(mainScreen, 14, 3, "Your insight is not high enough. Stamina -5");
												mainCharacter.stamina -= 5;
											} else {
												enemiesKilled += 1;
												mvwprintw(mainScreen, 14, 3, "You won the battle and stole items");
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.stamina = 0;
												area[mainCharacter.location].room[mainCharacter.currentRoom - 1].hasEnemy = 0;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].quantity += area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.quantity;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].value = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.value;
												mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].ID = area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID;
												strcpy(mainCharacter.inventory[area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.ID].name, area[mainCharacter.location].room[mainCharacter.currentRoom - 1].roomEnemy.inventory.name);
											}
											break;
										default:
											break;
									}
									
									wrefresh(mainScreen);
									
									getch();

									break;
								default:
									Mix_PlayChannel(-1, buttonMoveSound, 0);
									break;
							}
							
						}
						
						
						
						break;
					case 1:
						break;
					default:
						break;
				}
				
				}
				
				chosenOption = -1;
			
				break;
				case 2:
				currentSelection = 0;
				while (chosenOption == -1) {
					for (int i = 0; i < 7; i++) {
						if (i == currentSelection) {
							attron(A_UNDERLINE);
						}
						
						switch (i) {
							case 0:
								mvwprintw(stdscr, i + 7, 3, "Upgrade Strength");
								break;
							case 1:
								mvwprintw(stdscr, i + 7, 3, "Upgrade Mystic");
								break;
							case 2:
								mvwprintw(stdscr, i + 7, 3, "Upgrade Dexterity");
								break;
							case 3:
								mvwprintw(stdscr, i + 7, 3, "Upgrade Resistance");
								break;
							case 4:
								mvwprintw(stdscr, i + 7, 3, "Upgrade Charisma");
								break;
							case 5:
								mvwprintw(stdscr, i + 7, 3, "Upgrade Insight");
								break;
							case 6:
								mvwprintw(stdscr, i + 7, 3, "Back");
								break;
							default:
								break;
						}
						attroff(A_UNDERLINE);
					}

					int ch = getch();

					switch (ch) {
						case KEY_UP:
							Mix_PlayChannel(-1, buttonMoveSound, 0);
							currentSelection = (currentSelection - 1) % 7;
							if (currentSelection < 0) {
								currentSelection = 6;
							}
							break;
						case KEY_DOWN:
							Mix_PlayChannel(-1, buttonMoveSound, 0);
							currentSelection = (currentSelection + 1) % 7;
							break;
						case '\n':
							Mix_PlayChannel(-1, buttonSelectSound, 0);
							chosenOption = currentSelection;
							if (chosenOption == 6) {
								chosenOption = -1;
								goto entryPoint;
							}
							break;
						default:
							Mix_PlayChannel(-1, buttonMoveSound, 0);
							break;
					}
				}		
				
						if (chosenOption != 6) {
							int toUpgrade = chosenOption;
							chosenOption = -1;
							
							setupUi(true);
					
							currentSelection = 0;
							mvwprintw(stdscr, 2, 3, "You look over to the alchemy bench and wonder...");
							mvwprintw(stdscr, 6, 3, "Spend what?");
							while (chosenOption == -1) {
								for (int i = 0; i < 5; i++) {
									if (i == currentSelection) {
										attron(A_UNDERLINE);
									}
									
									switch (i) {
										case 0:
											mvwprintw(stdscr, i + 7, 3, "Spend Coin");
											break;
										case 1:
											mvwprintw(stdscr, i + 7, 3, "Spend Small Gem");
											break;
										case 2:
											mvwprintw(stdscr, i + 7, 3, "Spend Medium Gem");
											break;
										case 3:
											mvwprintw(stdscr, i + 7, 3, "Spend Large Gem");
											break;
										case 4:
											mvwprintw(stdscr, i + 7, 3, "Back");	
											break;
										default:
											break;
									}
									attroff(A_UNDERLINE);
								}

								int ch = getch();

								switch (ch) {
									case KEY_UP:
										Mix_PlayChannel(-1, buttonMoveSound, 0);
										currentSelection = (currentSelection - 1) % 5;
										if (currentSelection < 0) {
											currentSelection = 4;
										}
										break;
									case KEY_DOWN:
										Mix_PlayChannel(-1, buttonMoveSound, 0);
										currentSelection = (currentSelection + 1) % 5;
										break;
									case '\n':
										Mix_PlayChannel(-1, buttonSelectSound, 0);
										chosenOption = currentSelection;
										if (mainCharacter.inventory[chosenOption].quantity > 0 && chosenOption != 4) {
											Mix_PlayChannel(-1, upgradeCharacterSound, 0);
											switch (chosenOption) {
												case 0:
												case 1:
												case 2:
												case 3:
													mainCharacter.inventory[chosenOption].quantity -= 1;
													
													switch (toUpgrade) {
														case 0:
															mainCharacter.skills.strength += 1;
															break;
														case 1:
															mainCharacter.skills.mystic += 1;
															break;
														case 2:
															mainCharacter.skills.dexterity += 1;
															break;
														case 3:
															mainCharacter.skills.resistance += 1;
															break;
														case 4:
															mainCharacter.skills.charisma += 1;
															break;
														case 5:
															mainCharacter.skills.insight += 1;
															break;
														default:
															break;
													}
													break;
												case 4:
													goto entryPoint;
													break;
												default:
													break;
											}
										} else if (chosenOption == 4) {
											// Do nothing
										} else {
											mvwprintw(mainScreen, 14, 3, "Not enough item");
										}
										break;
									default:
										Mix_PlayChannel(-1, buttonMoveSound, 0);
										break;
								}
							}
							
							
						
						}
				
				break;
			default:
				break;
		}
		
		
		
		chosenOption = -1;
		currentSelection = 0;
		setupUi(true);
		
		switch (mainCharacter.location) {
			case GREAT_HALL:
				strcpy(mainCharacter.locationChar, "Great Hall");
				break;
			case GUARD_ROOM:
				strcpy(mainCharacter.locationChar, "Guard Room");
				break;
			case KITCHEN:
				strcpy(mainCharacter.locationChar, "Kitchen");
				break;
			case PANTRY:
				strcpy(mainCharacter.locationChar, "Pantry");
				break;
			case ARMORY:
				strcpy(mainCharacter.locationChar, "Armory");
				break;
			case CELLS:
				strcpy(mainCharacter.locationChar, "Cells");
				break;
			case CHAPEL:
				strcpy(mainCharacter.locationChar, "Chapel");
				break;
			case THE_HOLE:
				strcpy(mainCharacter.locationChar, "'The Hole'");
				break;
			case TORTURE_CHAMBER:
				strcpy(mainCharacter.locationChar, "Torture Chamber");
				break;
			case LABORATORY:
				strcpy(mainCharacter.locationChar, "Laboratory");
				break;
			case CRYPT:
				strcpy(mainCharacter.locationChar, "Crypt");
				break;
			case THE_LAIR:
				strcpy(mainCharacter.locationChar, "The Lair");
				break;
			case THE_BURROW:
				strcpy(mainCharacter.locationChar, "The Burrow");
				break;
			default:
				break;
		}
	}
	
exitSequence:
  Mix_FreeMusic(backgroundMusic);
  Mix_FreeChunk(buttonMoveSound);
  Mix_FreeChunk(buttonSelectSound);
	delwin(information);
	delwin(mainScreen);
	delwin(inventoryScreen);
	endwin();
	
	return 0;
}