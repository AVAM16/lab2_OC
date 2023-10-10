#include "L2Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache L2;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}

/*********************** L1 L2 *************************/

void initL2() { L2.init = 0; }

void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, offset;

  if (L2.init == 0) {
    for (int i = 0; i < (L2_SIZE / BLOCK_SIZE); i++) {
      L2.lines2[i].Valid = 0;
      L2.lines2[i].Dirty = 0;
      L2.lines2[i].Tag = 0;
    }
    L2.init = 1;
  }

  Tag = address / ((L2_SIZE / BLOCK_SIZE) * BLOCK_SIZE);
  index = (address / BLOCK_SIZE) % (L2_SIZE / BLOCK_SIZE);
  offset = address % BLOCK_SIZE;

  int found = 0;
  int i = 0;
  if (L2.lines2[index][i].Valid && L2.lines2[index][i].Tag == Tag) {
    found = 1;


  if (!found) {
    i = 0;
    while (i < ASSOCIATIVITY_L2 && L2.lines2[index][i].Valid) {
      i++;
    }
    if (i == ASSOCIATIVITY_L2) {
      i = 0;
      unsigned int min = L2.lines2[index][0].Time;
      for (int j = 1; j < ASSOCIATIVITY_L2; j++) {
        if (L2.lines2[index][j].Time < min) {
          min = L2.lines2[index][j].Time;
          i = j;
        }
      }
    }
    if (L2.lines2[index][i].Dirty) {
      accessDRAM(L2.lines2[index][i].Tag * (L2_SIZE / (BLOCK_SIZE * ASSOCIATIVITY_L2)) * BLOCK_SIZE + index * BLOCK_SIZE, L2.lines2[index][i].Data, MODE_WRITE);
      L2.lines2[index][i].Data[0] = 0;
      L2.lines2[index][i].Data[WORD_SIZE] = 0;
    }

    accessDRAM(address - offset, L2.lines2[index][i].Data, MODE_READ);

    L2.lines2[index][i].Valid = 1;
    L2.lines2[index][i].Dirty = 0;
    L2.lines2[index][i].Tag = Tag;
    L2.lines2[index][i].Time = time;

    if (mode == MODE_READ) {
      memcpy(data, &(L2.lines2[index][i].Data), BLOCK_SIZE);
      time += L2_READ_TIME;
    }
  } else {
    if (mode == MODE_READ) {
      memcpy(data, &(L2.lines2[index][i].Data), BLOCK_SIZE);
      time += L1_READ_TIME;
      L2.lines2[index][i].Time = time;
    }
    if (mode == MODE_WRITE) {
      memcpy(&(L2.lines2[index][i].Data), data, BLOCK_SIZE);
      time += L1_WRITE_TIME;
      L2.lines2[index][i].Dirty = 1;
      L2.lines2[index][i].Time = time;
    }
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL2(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL2(address, data, MODE_WRITE);
}
