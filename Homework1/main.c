#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
struct room;
struct creature;

typedef struct {
  int id;
  int type; //  PC = 0, Animal = 1, or NPC = 2
  struct room* location;
} creature;

typedef struct {
  int state; // 0 for clean, 1 for half-dirty, 2 for dirty
  int id;
  struct room* north;
  struct room* south;
  struct room* east;
  struct room* west;
  creature* creatures[10];
} room;
 

void react(creature* c, int v);
int leaveRoom(creature* c);
void performAction(creature* culprit, char* action);

creature* pc = NULL;
int respect = 40;

room* newRoom() {
  room* r = (room*)malloc(sizeof(room));
  r->north = NULL;
  r->south = NULL;
  r->east = NULL;
  r->west = NULL;
  for (int i = 0; i < 10; i++)
    r->creatures[i] = NULL;
  return r;
}

creature* newCreature() {
  creature* c = (creature*)malloc(sizeof(creature));
  c->location = NULL;
  return c;
}

char* getType(creature* c) {
  if (c->type == 0)
    return "PC";
  if (c->type == 1)
    return "Animal";
  if (c->type == 2)
    return "NPC";
}

room* getNeighbor(room* r, char direction) {
  switch(direction) {
  case 'n':
    return (room*)r->north;
  case 'e':
    return (room*)r->east;
  case 's':
    return (room*)r->south;
  case 'w':
    return (room*)r->west;
  default:
    return NULL;
  }
}

int isValidType(int x) {
  return x == 0 || x == 1 || (x == 2 && pc == 0);
}

int getInteger(char* s, int numCharacters) {
  if (numCharacters == 0) return -2; // The lowest number we expect is -1, so -2 means invalid string
  char num[numCharacters];
  int i = 0;
  if (s[i] == '-') // Check for negative numbers
    num[i] = s[i++];
  for (; i < numCharacters; i++) {
    if (!isdigit(s[i]))
      return -2;
    else
      num[i] = s[i];
  }
  return atoi(num);
}

int isValidRoomString(char* s, int numRooms) {
  int length = strlen(s);
  char str[length];
  for (int i = 0; i < length; i++)
    str[i] = s[i];
  char* token = strtok(str," ");
  int x = getInteger(token, strlen(token));
  if (!(x == 0 || x == 1 || x == 2))
    return 0;
  for (int i = 0; i < 3; i++) {
    token = strtok(NULL," "); // get next token
    x = getInteger(token, strlen(token));
    if (x < -1 || x > numRooms - 1)
      return 0;
  }
  token = strtok(NULL," \n\0\t"); // The last token may have funky stuff (\n) after it
  x = getInteger(token, strlen(token));
  if (x < -1 || x > numRooms - 1)
    return 0;
  return 1;
}

int isValidCreatureString(char* s, int numRooms) {
  int length = strlen(s);
  char str[length];
  for (int i = 0; i < length; i++)
    str[i] = s[i];
  char* token = strtok(str," ");
  int x = getInteger(token, strlen(token));
  if (!(x == 0 || x == 1 || x == 2))
    return 0;
  token = strtok(NULL," \n\0\t");
  x = getInteger(token, strlen(token));
  if (x < -1 || x > numRooms - 1)
    return 0;
  return 1;
}

int roomNotFull(room* r) {
  for (int i = 0; i < 10; i++) {
    if (!r->creatures[i])
      return 1;
  }
  return 0;
}

int addCreatureToRoom(room* r, creature* c) {
  for (int i = 0; i < 10; i++) {
    if (!r->creatures[i]) {
      r->creatures[i] = (creature*)c;
      c->location = (struct room*)r;
      return 1;
    }
  }
  return 0;
}

int removeCreatureFromRoom(room* r, creature* c) {
  for (int i = 0; i < 10; i++) {
    creature* creat = (creature*)r->creatures[i];
    if (creat == c) {
      r->creatures[i] = NULL;
      return 1;
    }
  }
  return 0;
}

char* getState(room* r) {
  int state = r->state;
  switch(state) {
  case 0:
    return "clean";
  case 1:
    return "half-dirty";
  case 2:
    return "dirty";
  default:
    return NULL;
  }
}

void printContentsOfRoom(room* r) {
  printf("\nRoom %d:\n", r->id);
  printf("\tState: %s\n", getState(r));
  printf("\n\tNeighbors:\n");
  if (r->north)
    printf("\t    North: Room %d\n",((room*)r->north)->id);
  if (r->east)
    printf("\t    East: Room %d\n",((room*)r->east)->id);
  if (r->south)
    printf("\t    South: Room %d\n",((room*)r->south)->id);
  if (r->west)
    printf("\t    West: Room %d\n",((room*)r->west)->id);
  printf("\n\tCreatures:\n");
  for (int i = 0; i < 10; i++) {
    creature* c = (creature*)r->creatures[i];
    if (c) {
      if (c == pc)
	printf("\t    %s\n",getType(c));
      else 
	printf("\t    %s %d\n",getType(c),c->id);
    }
  }
}

void performAction(creature* culprit, char* action) {
  room* r = (room*)culprit->location;
  if (strcmp(action,"dirty") == 0)
    r->state++;
  else
    r->state--;
  if (culprit == pc)
    printf("You perform %s on Room %d, making the room %s\n\n", action, r->id, getState(r));
  else
    printf("\n%s %d performs %s on Room %d, making the room %s\n\n", getType(culprit), culprit->id, action, r->id, getState(r));
  for (int i = 0; i < 10; i++) {
    creature* c = (creature*)r->creatures[i];
    if (c) { // not null
      if (c != pc) { // only non pc creatures react
	if (c == culprit) { // the culprit reacts more strongly
	  if (strcmp(getType(c),"Animal") == 0) {
	    if (strcmp(action,"dirty") == 0) {
	      respect -= 3;
	      printf("Animal %d growls at you harshly.\n", c->id);
	      if (r->state == 2)
		leaveRoom(c);
	    } else { // must be clean
	      respect += 3;
	      printf("Animal %d licks your face vigorously.\n", c->id);
	    }
	  } else { // must be NPC
	    if (strcmp(action,"dirty") == 0) {
	      respect += 3;
	      printf("NPC %d smiles at you intensely.\n", c->id);
	    } else { // must be clean
	      respect -= 3;
	      printf("NPC %d grumbles with disgust.\n", c->id);
	      if (r->state == 0)
		leaveRoom(c);
	    }
	  }
	} else { // creatures which did not perform the action react regularly
	  if (strcmp(getType(c),"Animal") == 0) {
	    if (strcmp(action,"dirty") == 0) {
	      react(c,-1);
	      if (r->state == 2)
		leaveRoom(c);
	    } else { // must be clean
	      react(c,1);
	    }
	  } else { // must be NPC
	    if (strcmp(action,"dirty") == 0) {
	      react(c,1);
	    } else { // must be clean
	      react(c,-1);
	      if (r->state == 0)
		leaveRoom(c);
	    }
	  }
	}
      }
    }
  }
}

void react(creature* c, int v) {
  if (strcmp(getType(c),"Animal") == 0) {
    if (v < 0) {
      respect--;
      printf("Animal %d growls at you.\n", c->id);
    } else {
      respect++;
      printf("Animal %d licks your face.\n", c->id);
    }
  } else { //must be npc
    if (v < 0) {
      respect--;
      printf("NPC %d grumbles.\n", c->id);
    } else {
      respect++;
      printf("NPC %d smiles at you.\n", c->id);
    }
  }
}

// returns true if creature burrowed out of the game
int leaveRoom(creature* c) {
  // Check what rooms are not full and add them to an array somehow then generate randomly over that
  room* current = (room*)c->location;
  removeCreatureFromRoom(current, c);
  room* north = (room*)current->north;
  room* east = (room*)current->east;
  room* south = (room*)current->south;
  room* west = (room*)current->west;
  int numberAvailable = 0;
  room* available[4];
  if (north && roomNotFull(north))
    available[numberAvailable++] = north;
  if (east && roomNotFull(east))
    available[numberAvailable++] = east;
  if (south && roomNotFull(south))
    available[numberAvailable++] = south;
  if (west && roomNotFull(west))
    available[numberAvailable++] = west;
  if (numberAvailable > 0) {
    time_t t;
    int direction = rand() % numberAvailable;
    room* relocation = available[direction];
    addCreatureToRoom(available[direction], c);
    if (strcmp(getType(c),"Animal") == 0) {
      if (relocation->state == 2)
	relocation->state = 1;
    } else {
      if (relocation->state == 0)
	relocation->state = 1;
    }
    if (available[direction] == north) {
      printf("%s %d flees to the north\n\n", getType(c), c->id);
    } else if (available[direction] == east) {
      printf("%s %d flees to the east\n\n", getType(c), c->id);
    } else if (available[direction] == south) {
      printf("%s %d flees to the south\n\n", getType(c), c->id);
    } else if (available[direction] == west) {
      printf("%s %d flees to the west\n\n", getType(c), c->id);
    }
  } else { // Nowhere to go, burrow out!
    printf("\n%s %d has nowhere to go, and thus burrows out through the roof!\n", getType(c), c->id);
    for (int i = 0; i < 10; i++) {
      creature* c = (creature*)current->creatures[i];
      if (c && c != pc) {
	printf("  ");
	react(c,-1);
      }
    }
    printf("\n");
  }
}

int stringContainsChar(char* str, char c) {
  int length = strlen(str);
  for (int i = 0; i < length; i++) {
    if (str[i] == c)
      return 1;
  }
  return 0;
}

creature* getCreature(room* r, char* idstr) {
  int id = getInteger(idstr, strlen(idstr));
  if (id == -2)
    return NULL;
  for (int i = 0; i < 10; i++) {
    creature* c = (creature*)r->creatures[i];
    if (c && c->id == id)
      return c;
  }
  return NULL;
}

int main() {
  srand(time(NULL)); // Initialize random number generator
  // Get the number of rooms
  char input[20];

  int digits = 0;
  int numberOfRooms = 0;
  do {
    for (int i = 0; i < 20; i++) input[i] = 'a';
    printf("Enter the number of rooms: (less than 100)\n");
    fgets(input, 3, stdin);
    if (isdigit(input[0]))
      if (isdigit(input[1]))
        digits = 2;
      else
        digits = 1;
    else
      digits = 0;
    numberOfRooms = getInteger(input, digits);
  } while (numberOfRooms < 1 || numberOfRooms > 99);
  room* rooms[numberOfRooms];

  // Enter information for the rooms
  int neighbors[numberOfRooms][4]; // fill in this array, and once all rooms are created iterate through to set neighbor pointers
  printf("\nPlease enter 5 numbers describing the room.\nState: 0, 1 or 2 for clean, half-drty or dirty respectively\nNorth South East and West: the number corresponding to the neighbor in that direction, or -1 for no neighbor\nThe format of each input line should be\n State North South East West\n");
  for (int i = 0; i < numberOfRooms; i++) {
    rooms[i] = newRoom();
    do {
      printf("\nEnter \"State North South East West\" for room %d:\n", i);
      for (int j = 0; j < 20; j++) input[j] = 'a';
      fgets(input, 20, stdin);
    } while (!isValidRoomString(input, numberOfRooms));
    char* token = strtok(input," ");
    int x = getInteger(token, strlen(token));
    rooms[i]->state = x;
    rooms[i]->id = i;
    for (int j = 0; j < 3; j++) {
      token = strtok(NULL," "); // get next token
      x = getInteger(token, strlen(token));
      neighbors[i][j] = x;
    }
    token = strtok(NULL," ");
    x = getInteger(token, strlen(token)-1); // The end of string token must not be included
    neighbors[i][3] = x;
  }
  // Information taken, now set neighbors!
  for (int i = 0; i < numberOfRooms; i++) {
    if (neighbors[i][0] > -1)
      rooms[i]->north = (struct room*)rooms[neighbors[i][0]];
    if (neighbors[i][1] > -1)
      rooms[i]->south = (struct room*)rooms[neighbors[i][1]];
    if (neighbors[i][2] > -1)
      rooms[i]->east = (struct room*)rooms[neighbors[i][2]];
    if (neighbors[i][3] > -1)
      rooms[i]->west = (struct room*)rooms[neighbors[i][3]];
  }

  digits = 0;
  int maxCreatures = 99;
  if (numberOfRooms < 10)
    maxCreatures = numberOfRooms*10;
  int numberOfCreatures = 0;
  do {
    for (int i = 0; i < 20; i++) input[i] = 'a';
    printf("\nEnter the number of creatures: (maximum of %d)\n", maxCreatures);
    fgets(input, 3, stdin);
    if (isdigit(input[0]))
      if (isdigit(input[1]))
        digits = 2;
      else
        digits = 1;
    else
      digits = 0;
    numberOfCreatures = getInteger(input, digits);
  } while (numberOfCreatures < 1 || numberOfCreatures > 99);
  creature* creatures[numberOfCreatures];

  printf("\nPlease enter 2 numbers describing the creature.\ncreatureType: 0 for PC, 1 for animal and 2 for NPC.\nLocation: The room number the creature is in.\nThe format of each input line should be\n Type Location\n");
  for (int i = 0; i < numberOfCreatures; i++) {
    creatures[i] = newCreature();
    int creatureInstalled = 0;
    while (!creatureInstalled) {
      do {
	printf("\nEnter \"Type Location\" for creature %d:\n", i);
	for (int j = 0; j < 20; j++) input[j] = 'a';
	fgets(input, 20, stdin);
      } while (!isValidCreatureString(input, numberOfRooms));
      // Once this checks out, check if room is full. If not, insert creature and set creatureInstalled to true
      char* token = strtok(input," ");
      int typeNum = getInteger(token, strlen(token));
      token = strtok(NULL," ");
      int roomNum = getInteger(token, strlen(token)-1);
      if (typeNum != 0) { // If type is not pc, or pc has not been defined
	if (roomNotFull(rooms[roomNum])) {
	  creatures[i]->type = typeNum;
	  creatures[i]->id = i;
	  addCreatureToRoom(rooms[roomNum], creatures[i]);
	  creatureInstalled = 1;
	} else {
	  printf("\n-- Room %d is full --\n", roomNum);
	}
      } else if (!pc) {
	if (roomNotFull(rooms[roomNum])) {
	  creatures[i]->type = 0;
	  addCreatureToRoom(rooms[roomNum], creatures[i]);
	  pc = creatures[i];
	  pc->id = i;
	  creatureInstalled = 1;
	} else {
	  printf("\n-- Room %d is full --\n", roomNum);
	}
      } else {
	printf("\n-- PC already defined --\n");
      }
    }
  }
  if (!pc) {
    printf("\nNo PC created -- Logically, the creatures of the world have no respect for a PC,\n thus if we bend the meaning of nothing to mean 0,\n then technically you have lost the game.\n -- Better Luck Next Time --\n");
    return 0;
  }
  char* command = NULL;
  while ( command == NULL) {
    printf("\nEnter a valid command:\n");
    for (int j = 0; j < 20; j++) input[j] = 'a';
    fgets(input, 20, stdin);
    command = strtok(input," \n\0");
  }
  while (strcmp(command, "exit") != 0) {
    int validCommand = 1;
    if (strcmp(command, "look") == 0) {
      printContentsOfRoom((room*)pc->location);
    } else if (strcmp(command, "dirty") == 0) {
      if (((room*)pc->location)->state == 2)
	printf("\nThe room is already dirty.\n");
      else
	performAction(pc,"dirty");
    } else if (strcmp(command, "clean") == 0) {
      if (((room*)pc->location)->state == 0)
	printf("\nThe room is already clean.\n");
      else
	performAction(pc, "clean");
    } else if (strcmp(command, "north") == 0) {
      room* location = (room*)pc->location;
      if (location->north) {
	if (roomNotFull((room*)location->north)) {
	  removeCreatureFromRoom(location, pc);
	  addCreatureToRoom((room*)location->north, pc);
	  printf("\nYou move north to Room %d.\n",((room*)location->north)->id);
	} else {
	  printf("\nThe room to the north is already full!\n");
	}
      } else {
	printf("\nThere is no room to the north.\n");
      }
    } else if (strcmp(command, "east") == 0) {
      room* location = (room*)pc->location;
      if (location->east) {
	if (roomNotFull((room*)location->east)) {
	  removeCreatureFromRoom(location, pc);
	  addCreatureToRoom((room*)location->east, pc);
	  printf("\nYou move east to Room %d.\n",((room*)location->east)->id);
	} else {
	  printf("\nThe room to the east is already full!\n");
	}
      } else {
	printf("\nThere is no room to the east.\n");
      }
    } else if (strcmp(command, "south") == 0) {
      room* location = (room*)pc->location;
      if (location->south) {
	if (roomNotFull((room*)location->south)) {
	  removeCreatureFromRoom(location, pc);
	  addCreatureToRoom((room*)location->south, pc);
	  printf("\nYou move south to Room %d.\n",((room*)location->south)->id);
	} else {
	  printf("\nThe room to the south is already full!\n");
	}
      } else {
	printf("\nThere is no room to the south.\n");
      }
    } else if (strcmp(command, "west") == 0) {
      room* location = (room*)pc->location;
      if (location->west) {
	if (roomNotFull((room*)location->west)) {
	  removeCreatureFromRoom(location, pc);
	  addCreatureToRoom((room*)location->west, pc);
	  printf("\nYou move west to Room %d.\n",((room*)location->west)->id);
	} else {
	  printf("\nThe room to the west is already full!\n");
	}
      } else {
	printf("\nThere is no room to the west.\n");
      }
    } else if (stringContainsChar(command, ':')) {
      char* leftCommand = strtok(command,": \n\0");
      char* rightCommand = strtok(NULL, " \n\0");
      creature* creatCommand = getCreature((room*)pc->location, leftCommand);
      if (creatCommand) {
	if (strcmp(rightCommand, "dirty") == 0) {
	 if (((room*)pc->location)->state == 2)
	    printf("\nThe room is already dirty.\n");
	  else
	    performAction(creatCommand,"dirty"); 
	} else if (strcmp(rightCommand, "clean") == 0) {
	  if (((room*)pc->location)->state == 0)
	    printf("\nThe room is already clean.\n");
	  else
	    performAction(creatCommand,"clean");
	} else if (strcmp(rightCommand, "north") == 0) {
	  room* location = (room*)pc->location;
	  if (location->north) {
	    if (roomNotFull((room*)location->north)) {
	      removeCreatureFromRoom(location, creatCommand);
	      addCreatureToRoom((room*)location->north, creatCommand);
	      printf("\n%s %d move north to Room %d.\n", getType(creatCommand), creatCommand->id, ((room*)location->north)->id);
	    } else {
	      printf("\nThe room to the north is already full!\n");
	    }
	  } else {
	    printf("\nThere is no room to the north.\n");
	  }
	} else if (strcmp(rightCommand, "east") == 0) {
	  room* location = (room*)pc->location;
	  if (location->east) {
	    if (roomNotFull((room*)location->east)) {
	      removeCreatureFromRoom(location, creatCommand);
	      addCreatureToRoom((room*)location->east, creatCommand);
	      printf("\n%s %d move east to Room %d.\n", getType(creatCommand), creatCommand->id, ((room*)location->east)->id);
	    } else {
	      printf("\nThe room to the east is already full!\n");
	    }
	  } else {
	    printf("\nThere is no room to the east.\n");
	  }
	} else if (strcmp(rightCommand, "south") == 0) {
	  room* location = (room*)pc->location;
	  if (location->south) {
	    if (roomNotFull((room*)location->south)) {
	      removeCreatureFromRoom(location, creatCommand);
	      addCreatureToRoom((room*)location->south, creatCommand);
	      printf("\n%s %d move south to Room %d.\n", getType(creatCommand), creatCommand->id, ((room*)location->south)->id);
	    } else {
	      printf("\nThe room to the south is already full!\n");
	    }
	  } else {
	    printf("\nThere is no room to the south.\n");
	  }
	} else if (strcmp(rightCommand, "west") == 0) {
	  room* location = (room*)pc->location;
	  if (location->west) {
	    if (roomNotFull((room*)location->west)) {
	      removeCreatureFromRoom(location, creatCommand);
	      addCreatureToRoom((room*)location->west, creatCommand);
	      printf("\n%s %d move west to Room %d.\n", getType(creatCommand), creatCommand->id, ((room*)location->west)->id);
	    } else {
	      printf("\nThe room to the west is already full!\n");
	    }
	  } else {
	    printf("\nThere is no room to the west.\n");
	  }
	}
      } else {
	printf("\n-- Not a valid creature --\n");
      }
    } else {
      printf("\n -- Invalid Command --\n");
      validCommand = 0;
    }
    if (validCommand) {
      printf("\nRespect is now %d\n",respect);
    }
    if (respect > 79 || respect < 1)
      break;
    command = NULL;
    while (command == NULL) {
      printf("\nEnter a valid command:\n");
      for (int j = 0; j < 20; j++) input[j] = 'a';
      fgets(input, 20, stdin);
      command = strtok(input," \n\0");  
    }
  }
  
  if (respect > 79)
    printf("\nCongratulations! You have won!\n-- Thank you for playing --\n");
  else if (respect < 1)
    printf("\nSorry, it appears you've lost.\n-- Thank you for plaing --\n");
  else
    printf("\nThanks for playing!\n-- Come back and try again soon! --\n");

  for (int i = 0; i < numberOfRooms; i++)
    free(rooms[i]);
  for (int i = 0; i < numberOfCreatures; i++)
    free(creatures[i]);
  return 0;
}
  
