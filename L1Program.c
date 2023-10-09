#include "L1Cache.h"

int main() {
  resetTime();
  initCache();
  uint8_t value1[256];
  uint8_t value2[256];
  for (int i=0; i<256; i++){
    value1[i] = i;
    value2[i] = 0;
  };

  write(1, (&value1));

  uint32_t clock = getTime();
  printf("Time: %d\n", clock);

  read(1, (&value2));
  clock = getTime();
  printf("Time: %d\n", clock);

  write(512, (&value1));
  clock = getTime();
  printf("Time: %d\n", clock);

  read(512, (&value2));
  clock = getTime();
  printf("Time: %d\n", clock);

  return 0;
}
