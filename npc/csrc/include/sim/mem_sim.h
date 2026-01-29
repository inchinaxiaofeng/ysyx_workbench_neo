#ifndef __MEM_SIM__
#define __MEM_SIM__

#include <cstdint>
#include <stdbool.h>

typedef struct {
  bool iRen;
  bool iWen;
  FMT_WORD iReadAddr;
  FMT_WORD iWriteAddr;
  uint32_t iByteMask; // 用多少拿多少
  FMT_WORD iWriteData;
} SimMem;
bool getSimMem(SimMem *mem);
void setSimMem(FMT_WORD oReadData);

#endif // !__MEM_SIM__
