#include "L1Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache SimpleCache;

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

void initCache() { SimpleCache.init = 0; }

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (SimpleCache.init == 0) {
    for (index = 0; index < L1_SIZE; index = index + BLOCK_SIZE) {
      SimpleCache.lines[index].Valid = 0;
    }
    SimpleCache.init = 1;
  }

  for(index = 0; index < L1_SIZE; index = index + BLOCK_SIZE) {
    CacheLine *Line = &SimpleCache.lines[index];

    Tag = address >> 3; // Why do I do this?

    MemAddress = address >> 3; // again this....!
    MemAddress = MemAddress << 3; // address of the block in memory

    /* access Cache*/

    if (!Line->Valid || Line->Tag != Tag) {         // if block not present - miss
      accessDRAM(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

      if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
        accessDRAM(MemAddress, &(L1Cache[0]),
                  MODE_WRITE); // then write back old block
      }

      memcpy(&(L1Cache[0]), TempBlock,
            BLOCK_SIZE); // copy new block to cache line
      Line->Valid = 1;
      Line->Tag = Tag;
      Line->Dirty = 0;
    } // if miss, then replaced with the correct block

    if (mode == MODE_READ) {    // read data from cache line
      if (0 == (address % 8)) { // even word on block
        memcpy(data[index], &(L1Cache[0]), WORD_SIZE);
      } else { // odd word on block
        memcpy(data[index], &(L1Cache[WORD_SIZE]), WORD_SIZE);
      }
      time += L1_READ_TIME;
    }

    if (mode == MODE_WRITE) { // write data from cache line
      if (!(address % 8)) {   // even word on block
        memcpy(&(L1Cache[0]), data[index], WORD_SIZE);
      } else { // odd word on block
        memcpy(&(L1Cache[WORD_SIZE]), data[index], WORD_SIZE);
      }
      time += L1_WRITE_TIME;
      Line->Dirty = 1;
    }
    
    address = address + BLOCK_SIZE;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
