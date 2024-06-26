#include "L2Cache.h"
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache LCaches;

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

/***********************  *************************/

void initCache() { LCaches.init1=0; LCaches.init2=0; }

void accessL2(uint32_t address, uint8_t *data, uint32_t mode){
  unsigned int Tag, index, offset;

  if (LCaches.init2==0){
    for (int i =0; i<L2_SIZE/BLOCK_SIZE; i++){
      CacheLine *Line= &LCaches.lines2[i];
      Line->Dirty=0;
      Line->Tag=0;
      Line->Valid=0;
    }
    LCaches.init2=1;
  }
  Tag=address/L2_SIZE;
  index= (address/BLOCK_SIZE) % (L2_SIZE/BLOCK_SIZE);
  offset=address%BLOCK_SIZE;

  CacheLine *Line= &LCaches.lines2[index];
  if (!Line->Valid || Line->Tag!=Tag){
    //printf("nao existe em L2\n");
    if (Line->Valid && Line->Dirty){
      accessDRAM(address-offset, Line->dados, MODE_WRITE);
      Line->dados[0]=0;
      Line->dados[WORD_SIZE]=0;
    }
    accessDRAM(address - offset, Line->dados, MODE_READ);

    Line->Valid=1;
    Line->Tag=Tag;
    Line->Dirty=0;

    if (mode == MODE_READ) {    // read data from cache line
      memcpy(data, &(Line->dados), BLOCK_SIZE);
      time += L2_READ_TIME;
    }
  } else{
    //printf("existe em L2\n");
    if (mode == MODE_READ) {    // read data from cache line
      memcpy(data, &(Line->dados), BLOCK_SIZE);
      time += L1_READ_TIME;
    }
    if (mode == MODE_WRITE) { // write data from cache line
      memcpy(&(Line->dados), data, BLOCK_SIZE);
      time += L1_WRITE_TIME;
      Line->Dirty = 1;
    }
  }

}






void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, offset;
  //uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (LCaches.init1 == 0) {
    // L1.lines = (CacheLine *)malloc((L1_SIZE/BLOCK_SIZE) * sizeof(CacheLine));
    for (int i = 0; i < L1_SIZE/BLOCK_SIZE; i++) {
      CacheLine *Line = &LCaches.lines1[i];
      Line->Valid = 0;
      Line->Dirty = 0;
      Line->Tag = 0;
    }
    LCaches.init1 = 1;
  }

  Tag = address / ((L1_SIZE / BLOCK_SIZE) * BLOCK_SIZE);
  index = (address / BLOCK_SIZE) % (L1_SIZE / BLOCK_SIZE);
  offset = address % BLOCK_SIZE;

    /* access Cache*/
  CacheLine *Line = &LCaches.lines1[index];
    if (Line->Valid && Line->Tag==Tag){
      //printf("está em L1\n");
      if (mode == MODE_READ) {    // read data from cache line
        memcpy(data, &(Line->dados[offset]), WORD_SIZE);
        time += L1_READ_TIME;
      }
      if (mode == MODE_WRITE) { // write data from cache line
        memcpy(&(Line->dados[offset]), data, WORD_SIZE);
        time += L1_WRITE_TIME;
        Line->Dirty = 1;
      }
    }
    else{
      //aceder a L2
      if (Line->Dirty){
        accessL2((Line->Tag) *(L1_SIZE/BLOCK_SIZE)* BLOCK_SIZE + index*BLOCK_SIZE, LCaches.lines1[index].dados, MODE_WRITE);
        Line->dados[0] = 0;
        Line->dados[WORD_SIZE] = 0;
      }

      accessL2(address-offset, Line->dados, MODE_READ);
      if (mode==MODE_READ){
        memcpy(data, &(Line->dados[offset]), WORD_SIZE);
        time+=L1_READ_TIME;
        Line->Dirty=0;
        Line->Valid=1;
        Line->Tag=Tag;
      }
      if (mode==MODE_WRITE){
        memcpy(&(Line->dados[offset]), data, WORD_SIZE);
        time+=L1_WRITE_TIME;
        Line->Dirty=1;
        Line->Valid=1;
        Line->Tag=Tag;
      }
    } 
 
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
