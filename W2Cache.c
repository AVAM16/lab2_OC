#include "W2Cache.h"
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache LCaches;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *dados, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(dados, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), dados, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}

/***********************  *************************/

void initCache() { LCaches.init1=0; LCaches.init2=0; }

void accessL2(uint32_t address, uint8_t *dados, uint32_t mode){
  unsigned int Tag, index, offset;
  //printf("entrei em L2\n");
  if (LCaches.init2==0){
    //printf("meter os atributos\n");
    for (int i =0; i<L2_SIZE/(BLOCK_SIZE*L2_WAYS); i++){
      for (int j=0; j<L2_WAYS; j++){
        LCaches.lines2[i][j].Dirty=0;
        LCaches.lines2[i][j].Tag=0;
        LCaches.lines2[i][j].Valid=0;
      }
    }
    LCaches.init2=1;
  }
  Tag=address/((L2_SIZE / (BLOCK_SIZE * L2_WAYS)) * BLOCK_SIZE);
  index= (address/BLOCK_SIZE) % (L2_SIZE/(BLOCK_SIZE*L2_WAYS));
  offset=address%BLOCK_SIZE;

  int found = 0;
  int i = 0;
  while (i < L2_WAYS && !found) {
    if (LCaches.lines2[index][i].Valid && LCaches.lines2[index][i].Tag == Tag) {
      found = 1;
    } else {
      i++;
    }
  }

  if (!found) {
    i = 0;
    while (i < L2_WAYS && LCaches.lines2[index][i].Valid) {
      i++;
    }
    if (LCaches.lines2[index][i].Dirty) {
      accessDRAM(LCaches.lines2[index][i].Tag * (L2_SIZE / (BLOCK_SIZE * L2_WAYS)) * BLOCK_SIZE + index * BLOCK_SIZE, LCaches.lines2[index][i].dados, MODE_WRITE);
      LCaches.lines2[index][i].dados[0] = 0;
      LCaches.lines2[index][i].dados[WORD_SIZE] = 0;
    }

    accessDRAM(address - offset, LCaches.lines2[index][i].dados, MODE_READ);

    LCaches.lines2[index][i].Valid = 1;
    LCaches.lines2[index][i].Dirty = 0;
    LCaches.lines2[index][i].Tag = Tag;

    if (mode == MODE_READ) {
      memcpy(dados, &(LCaches.lines2[index][i].dados), BLOCK_SIZE);
      time += L2_READ_TIME;
    }
  } else {
    if (mode == MODE_READ) {
      memcpy(dados, &(LCaches.lines2[index][i].dados), BLOCK_SIZE);
      time += L1_READ_TIME;
    }
    if (mode == MODE_WRITE) {
      memcpy(&(LCaches.lines2[index][i].dados), dados, BLOCK_SIZE);
      time += L1_WRITE_TIME;
      LCaches.lines2[index][i].Dirty = 1;
    }
  }
}



void accessL1(uint32_t address, uint8_t *dados, uint32_t mode) {

  uint32_t index, Tag, offset;
  //uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (LCaches.init1 == 0) {
    for (int i = 0; i < L1_SIZE/BLOCK_SIZE; i++) {
      LCaches.lines1[i].Valid = 0;
      LCaches.lines1[i].Dirty = 0;
      LCaches.lines1[i].Tag = 0; 
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
      if (mode == MODE_READ) {    // read dados from cache line
        memcpy(dados, &(Line->dados[offset]), WORD_SIZE);
        time += L1_READ_TIME;
      }
      if (mode == MODE_WRITE) { // write dados from cache line
        memcpy(&(Line->dados[offset]), dados, WORD_SIZE);
        time += L1_WRITE_TIME;
        Line->Dirty = 1;
      }
    }
    else{
      //aceder a L2
      //printf("não está em L1\n");
      if (Line->Dirty){
        accessL2((Line->Tag) *(L1_SIZE/BLOCK_SIZE)* BLOCK_SIZE + index*BLOCK_SIZE, Line->dados, MODE_WRITE);
        Line->dados[0] = 0;
        Line->dados[WORD_SIZE] = 0;
      }
      //printf("aceder a L2\n");
      accessL2(address-offset, Line->dados, MODE_READ);
      if (mode==MODE_READ){
        memcpy(dados, &(Line->dados[offset]), WORD_SIZE);
        time+=L1_READ_TIME;
        Line->Dirty=0;
        Line->Valid=1;
        Line->Tag=Tag;
      }
      if (mode==MODE_WRITE){
        memcpy(&(Line->dados[offset]), dados, WORD_SIZE);
        time+=L1_WRITE_TIME;
        Line->Dirty=1;
        Line->Valid=1;
        Line->Tag=Tag;
      }
    } 
 
}

void read(uint32_t address, uint8_t *dados) {
  accessL1(address, dados, MODE_READ);
}

void write(uint32_t address, uint8_t *dados) {
  accessL1(address, dados, MODE_WRITE);
}
