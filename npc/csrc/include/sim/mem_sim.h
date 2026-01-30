#ifndef __MEM_SIM__
#define __MEM_SIM__

#include <common.h>
#include <stdbool.h>

typedef struct {
  bool iRen;
  bool iWen;
  word_t iReadAddr;
  word_t iWriteAddr;
  uint32_t iByteMask; // 用多少拿多少
  word_t iWriteData;
} SimMem;
bool getSimMem(SimMem *mem);
void setSimMem(word_t oReadData);

#endif // !__MEM_SIM__
