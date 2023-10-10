#include "L1Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache L1;

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

/*********************** L1 cache *************************/

void initCache() { L1.init = 0; }

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, offset;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (L1.init == 0) {
    // L1.line = (CacheLine *)malloc((L1_SIZE/BLOCK_SIZE) * sizeof(CacheLine));
    for (int i = 0; i < L1_SIZE/BLOCK_SIZE; i++) {
      L1.lines[i].Valid = 0;
      L1.lines[i].Dirty = 0;
      L1.lines[i].Tag = 0;
      // for (int j =0; j< BLOCK_SIZE; j+=WORD_SIZE){
      //   L1.line[i].dados=0;
      // }
      
    }
    L1.init = 1;
  }

  Tag = address / ((L1_SIZE / BLOCK_SIZE) * BLOCK_SIZE);
  index = (address / BLOCK_SIZE) % (L1_SIZE / BLOCK_SIZE);
  offset = address % BLOCK_SIZE;

    /* access Cache*/
  CacheLine *Line = &L1.lines[index];
  if (!Line->Valid || Line->Tag != Tag) {         // if block not present - miss
      accessDRAM(address-offset, TempBlock, MODE_READ); // get new block from DRAM

      if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
        accessDRAM(address-offset, &(L1Cache[index*BLOCK_SIZE]),
                  MODE_WRITE); // then write back old block
      }

      memcpy(&(L1Cache[index*BLOCK_SIZE]), TempBlock,
            BLOCK_SIZE); // copy new block to cache line
      Line->Valid = 1;
      Line->Tag = Tag;
      Line->Dirty = 0;
    } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    if (0 == (address % 64)) { // even word on block
      memcpy(data, &(L1Cache[index*BLOCK_SIZE]), WORD_SIZE);
    } else { // odd word on block
      memcpy(data, &(L1Cache[index*BLOCK_SIZE+ offset]), WORD_SIZE);
    }
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    if (!(address % 64)) {   // even word on block
      memcpy(&(L1Cache[index*BLOCK_SIZE]), data, WORD_SIZE);
    } else { // odd word on block
      memcpy(&(L1Cache[index*BLOCK_SIZE+offset]), data, WORD_SIZE);
    }
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
