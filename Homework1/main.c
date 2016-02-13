#include <stdio.h>



struct room;
struct creature;

typedef struct {
  struct room* north;
  struct room* south;
  struct room* east;
  struct room* west;
  struct creature* creatures[10];
} room;
  
typedef struct {
  int id;
  int type; // Animal = 0, NPC = 1 or PC = 2
  struct room* location;
} creature;

char* getType(creature* c) {
  if (c->type == 0)
    return "Animal";
  if (c->type == 1)
    return "NPC";
  if (c->type == 2)
    return "PC";
}

creature* pc = NULL;

room* getNeighbor(room* r, char direction) {
  switch(direction) {
  case 'n':
    return r->north;
  case 'e':
    return r->east;
  case 's':
    return r->south;
  case 'w':
    return r->west;
  default:
    return NULL;
  }
}

int isValidType(int x) {
  return x == 0 || x == 1 || (x == 2 && pc == 0);
}

int getInteger(char* s, int numCharacters) {
  if (numCharacters == 0) return 0;
  char num[numCharacters];
  for (int i = 0; i < numCharacters; i++) {
    if (!isdigit(s[i]))
      return 0;
    else
      num[i] = s[i];
  }
  return atoi(num);
}

int isValidRoomString(char* s) {
  return 1; // Continue working from here
}

int main() {
  // Get the number of rooms
  char input[20];
  int digits = 0;
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
  } while (!getInteger(input, digits));
  int numberOfRooms = getInteger(input, digits);
  room* rooms[numberOfRooms];

  // Enter information for the rooms
  int neighbors[numberOfRooms][4]; // fill in this array, and once all rooms are created iterate through to set neighbor pointers
  printf("\nPlease enter 5 numbers describing the room.\nState: 0, 1 or 2 for clean, half-drty or dirty respectively\nNorth South East and West: the number corresponding to the neighbor in that direction, or -1 for no neighbor\nThe format of each input line should be\n State North South East West\n");
  for (int i = 0; i < numberOfRooms; i++) {
    rooms[i] = (room*)malloc(sizeof(room));
    do {
      printf("Enter information for room %d:\n", i);
      for (int i = 0; i < 20; i++) input[i] = 'a';
      fgets(input, 20, stdin);
    } while (!isValidRoomString(input));
  } 
  
  creature* alex = (creature*)malloc(sizeof(creature));
  do {
    printf("Enter the type of creature:\n");
    fgets(input, 2, stdin);
  } while (!isValidType(input[0] - '0'));
  alex->type = input[0] - '0';
  printf(getType(alex));
  //  fgets(a, 13, stdin);
  //printf("The string you entered is: ");
  //printf(a);

  for (int i = 0; i < numberOfRooms; i++) {
    free(rooms[i]);
  }
  return 0;
}
