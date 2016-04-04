#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>



struct cache;
struct set;
struct line;

char replacementPolicy = 'r';
int frequencyRange = 100;
int hitTime = 0;
int missPenalty = 0;
int totalMisses = 0;
int totalInputs = 0;
int totalCycles = 0;

int roundUp(float x) {
  if (x - (int)x > 0)
    return x + 1;
  else
    return x;
}

typedef struct {
  int valid;
  int tag;
  int cyclesSinceUsed; // for lru
  int* accessed; // 1 if accessed, 0 if not (each index refers to the previous time the set was accessed) for LFU 
} line;

typedef struct {
  int numberOfLines;
  line** lines;
} set;

typedef struct {
  int numberOfSets;
  int numberOfSetBits;
  int numberOfTagBits;
  set** sets;
} cache;

int sumArray(int* numbers, int length) {
  int sum = 0;
  for (int i = 0; i < length; i++)
    sum = sum + numbers[i];
  return sum;
}

void shiftAndAddAccessed(line* l, int addition) {
  for (int i = 0; i < frequencyRange - 1; i++) {
    l->accessed[i] = l->accessed[i+1];
  }
  l->accessed[frequencyRange-1] = addition;
}

void loadAddress(cache* c, int setID, int tagID) {
  set* s = c->sets[setID];
  line** lines = s->lines;
  for (int i = 0; i < s->numberOfLines; i++) {
    if (lines[i]->valid != 1) {
      lines[i]->valid = 1;
      lines[i]->tag = tagID;
      if (replacementPolicy == 'R')
        lines[i]->cyclesSinceUsed = 0;
      else {
        for (int j = 0; j < frequencyRange - 1; j++) {
          lines[i]->accessed[j] = 0;
        }
        lines[i]->accessed[frequencyRange-1] = 1;
      }
      return;
    }
  }
  //Now evict something!
  if (replacementPolicy == 'R') { // Replace least recently used
    int indexOfLRU = 0;
    for (int i = 0; i < s->numberOfLines; i++) {
      if (lines[i]->cyclesSinceUsed > lines[indexOfLRU]->cyclesSinceUsed)
        indexOfLRU = i;
    }
    lines[indexOfLRU]->tag = tagID; // SHOULD work
    lines[indexOfLRU]->cyclesSinceUsed = 0;
  } else { // Replace least frequently used
    int indexOfLFU = 0;
    for (int i = 0; i < s->numberOfLines; i++) {
      int sumI = sumArray(lines[i]->accessed, frequencyRange);
      int sumIndex = sumArray(lines[indexOfLFU]->accessed, frequencyRange);
      if (sumI < sumIndex) {
        indexOfLFU = i;
      }
    }
    lines[indexOfLFU]->tag = tagID;
    for (int i = 0; i < frequencyRange - 1; i++) {
      lines[indexOfLFU]->accessed[i] = 0;
    }
    lines[indexOfLFU]->accessed[frequencyRange-1] = 1;
  }
}

line* newLine() {
  line* l = (line*)malloc(sizeof(line));
  l->accessed = malloc(sizeof(int)*frequencyRange);
  l->valid = 0;
  return l;
}

set* newSet(int e, int b, int m) {
  set* set1 = (set*)malloc(sizeof(set));
  set1->numberOfLines = e;
  set1->lines = malloc(sizeof(set)*e);
  for (int i = 0; i < e; i++) {
    set1->lines[i] = newLine();
  }
  return set1;
}

cache* newCache(int s, int e, int b, int m) {
  cache* c = (cache*)malloc(sizeof(cache));
  c->sets = malloc(sizeof(set)*s);
  int numSetBits = roundUp(log2(s));
  c->numberOfSetBits = numSetBits;
  c->numberOfTagBits = m - (numSetBits + (roundUp(log2(b))));
  c->numberOfSets = s;
  for (int i = 0; i < s; i++) {
    c->sets[i] = newSet(e, b, m);
  }
  return c;
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

int convertBase2ToBase10(char* input) {
  int length = strlen(input);
  int sum = 0;
  int multiplier = 1;
  for (int i = 0; i < length; i++) {
    sum = sum + multiplier*(input[length-1 - i] - '0');
    multiplier = multiplier*2;
  }
  return sum;
}
/* Deprecated since c automatically converts base 16 to base 10 */
/* int convertBase16ToBase10(char* input) { */
/*   int length = strlen(input); */
/*   int sum = 0; */
/*   int multiplier = 1; */
/*   for (int i = 0; i < length; i++) { */
/*     int value; */
/*     char c = input[length-1 - i]; */
/*     if (isdigit(c)) */
/*       value = c - '0'; */
/*     else */
/*       if (c == 'a' || c == 'A') */
/*         value = 10; */
/*       else if (c == 'b' || c == 'B') */
/*         value = 11; */
/*       else if (c == 'c' || c == 'C') */
/*         value = 12; */
/*       else if (c == 'd' || c == 'D') */
/*         value = 13; */
/*       else if (c == 'e' || c == 'E') */
/*         value = 14; */
/*       else if (c == 'f' || c == 'F') */
/*         value = 15; */
/*     sum = sum + multiplier*value; */
/*     multiplier = multiplier*16; */
/*   } */
/*   return sum; */
/* } */


/* Use the following code to turn into a string: */
/*      char* s;                                 */
/*      sprintf(s,"%d",convertToBase2(250));     */
int convertToBase2(int number) {
  int digits[100];
  int index = 0;
  int remainder;
  while(number != 0) {
    remainder = number % 2;
    number = number / 2;
    if (remainder == 0)
      digits[index++] = 0;
    else
      digits[index++] = 1;
  }
  int sum = 0;
  int multiplier = 1;
  for (int i = 0; i < index; i++) {
    sum = sum + multiplier*digits[i];
    multiplier = multiplier*10;
  }
  return sum;
}

int hexDigit(char c) {
  if (isdigit(c)) return 1;
  if (c == 'a' || c == 'A')
    return 10;
  if (c == 'b' || c == 'B')
    return 11;
  if (c == 'c' || c == 'C')
    return 12;
  if (c == 'd' || c == 'D')
    return 13;
  if (c == 'e' || c == 'E')
    return 14;
  if (c == 'f' || c == 'F')
    return 15;
  return 0;
}

int hexDigits(char* input, int length) {
  for (int i = 0; i < length; i++) {
    if (!hexDigit(input[i])) {
      return 0;
    }
  }
  return 1;
}

int parseHexToInt(char* hex, int length) {
  int multiplier = 16*(length - 1);
  int sum = 0;
  for (int i = 0; i < length; i++) {
    int value = hexDigit(hex[i]);
    if (value == 1)
      value = hex[i] - '0';
    sum = sum + value*multiplier;
    multiplier = multiplier/16;
  }
  return sum;
}

void convertBase2ToString(int number, char* s, int numberCharactersToHave) {
  char string[numberCharactersToHave];
  int length = 0;
  for (; number != 0; length++)
    number = number/10;
  int index = 0;
  while (index + length < numberCharactersToHave) {
    s[index++] = '0';
  }
  for (int i = 0; i < length; i++) {
    s[index + i] = string[i];
  }
  for (int i = 0; i < numberCharactersToHave; i++)
    printf("%c\n",s[i]);
}

int numDigits(int x) {
  return (x == 0 ? 1 : (int)(log10(x)+1));
}

int nthDigit(int x, int index) {
  
}

int read(cache* c, int address, int addressSize) {
  int value = 0;
  int length = numDigits(address);
  int digits[addressSize];
  for (int i = 0; i < addressSize - length; i++)
    digits[i] = 0;
  //for (int i = addressSize - length; i < addressSize; i++) {
  int submount = 0;
  for (int i = 0; i < length; i++) {
    int x = address;
    for (int j = 0; j < length-i-1; j++) {
      x = x/10;
    }
    x = x - submount;
    submount = (submount + x)*10;
    digits[addressSize-length+i] = x;
  }
  
  int tagBits = c->numberOfTagBits;
  int setBits = c->numberOfSetBits;
  int tagID = 0;
  int setID = 0;
  int multiplier = 1;
  for (int i = 0; i < tagBits; i++) {
    tagID = tagID + digits[i]*multiplier;
    multiplier = multiplier*2;
  }
  multiplier = 1;
  for (int i = 0; i < setBits; i++) {
    setID = setID + digits[tagBits + i]*multiplier;
    multiplier = multiplier*2;
  }
  set* s = c->sets[setID];
  for (int i = 0; i < s->numberOfLines; i++) {
    s->lines[i]->cyclesSinceUsed++;
    if (s->lines[i]->valid) {
      if (s->lines[i]->tag == tagID) {
        s->lines[i]->cyclesSinceUsed = 0;
        shiftAndAddAccessed(s->lines[i], 1);
        value = 1;
      } else {
        shiftAndAddAccessed(s->lines[i], 0);
      }
    }
  }
  if (value)
    return value;
  loadAddress(c, setID, tagID);
  return 0;
}

int main() {
  int sets;
  int lines;
  int blockSize;
  int addressSize;

  char input[20];
  int digits;
  printf("Enter the number of sets (At most 3 digits): ");
  fgets(input, 4, stdin);
  if (isdigit(input[0]))
    if (isdigit(input[1]))
      if (isdigit(input[2]))
        digits = 3;
      else
        digits = 2;
    else
      digits = 1;
  else
    digits = 0;
  sets = getInteger(input, digits);

  printf("Enter the number of lines per set (At most 3 digits): ");
  fgets(input, 4, stdin);
  if (isdigit(input[0]))
    if (isdigit(input[1]))
      if (isdigit(input[2]))
        digits = 3;
      else
      digits = 2;
    else
      digits = 1;
  else
    digits = 0;
  lines = getInteger(input, digits);
  
  printf("Enter the block size (At most 3 digits): ");
  fgets(input, 4, stdin);
  if (isdigit(input[0]))
    if (isdigit(input[1]))
      if (isdigit(input[2]))
        digits = 3;
      else
        digits = 2;
    else
      digits = 1;
  else
    digits = 0;
  blockSize = getInteger(input, digits);

  printf("Enter the memory address size (At most 3 digits): ");
  fgets(input, 4, stdin); 
  if (isdigit(input[0]))
    if (isdigit(input[1]))
      if (isdigit(input[2]))
        digits = 3;
      else
        digits = 2;
    else
      digits = 1;
  else
    digits = 0;
  addressSize = getInteger(input, digits);

  printf("Enter LRU or LFU for the replacement policy: ");
  fgets(input, 4, stdin);
  while (1) {
    if (input[1] == 'R' || input[1] == 'r') {
      replacementPolicy = 'R';
      break;
    } else if (input[1] == 'F' || input[1] == 'f') {
      replacementPolicy = 'F';
      break;
    }
    fgets(input, 4, stdin);
    printf("Enter LRU or LFU for the replacement policy: ");
    fgets(input, 4, stdin);
  }
  fgets(input, 4, stdin);

  printf("Enter the hit time (At most 3 digits): ");
  fgets(input, 4, stdin);
  if (isdigit(input[0]))
    if (isdigit(input[1]))
      if (isdigit(input[2]))
        digits = 3;
      else
        digits = 2;
    else
      digits = 1;
  else
    digits = 0;
  hitTime = getInteger(input, digits);

  printf("Enter the miss penalty (At most 3 digits): ");
  fgets(input, 4, stdin);
  if (isdigit(input[0]))
    if (isdigit(input[1]))
      if (isdigit(input[2]))
        digits = 3;
      else
        digits = 2;
    else
      digits = 1;
  else
    digits = 0;
  missPenalty = getInteger(input, digits);
  
  cache* c = newCache(sets, lines, blockSize, addressSize);
  /* printf("Sets: %d\n", c->numberOfSets); */
  /* printf("Set Bits: %d\n", c->numberOfSetBits); */
  /* printf("Lines: %d\n", c->sets[0]->numberOfLines); */
  /* printf("Tag Bits: %d\n", c->numberOfTagBits); */
  /* printf("Validity: %d\n", c->sets[0]->lines[0]->valid); */

  char address[addressSize];
  int expectedDigits = addressSize/4;
  printf("Begin entering memory addresses:\n");
  fgets(input, expectedDigits + 1, stdin);
  fgets(address, expectedDigits + 1, stdin);
  
  while (!(input[0] == '-' && input[1] == '1')) {
    if (hexDigits(input, expectedDigits)) {
      totalInputs++;
      int base10 = parseHexToInt(input, expectedDigits);
      int base2 = convertToBase2(base10);
      //      convertBase2ToString(base2, address, addressSize);
      if (read(c, base2, addressSize)) {
        totalCycles = totalCycles + hitTime;
        printf("H\n");
      } else {
        totalMisses++;
        totalCycles = totalCycles + hitTime + missPenalty;
        printf("M\n");
      }
    } else {
      printf("Invalid input -- enter a %d digit base 16 number\n", expectedDigits);
    }
    fgets(input, expectedDigits + 1, stdin);
    fgets(address, expectedDigits + 1, stdin);
  }
  printf("%d %d\n",totalMisses*100/totalInputs,totalCycles);
    
  
  
  return 0;
}
