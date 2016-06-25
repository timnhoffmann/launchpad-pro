#include "timing.h"
#include "seq_ca.h"


u32 msTc = 0;
u32 frac = 0;
void msTick() {
  msTc++;
  
  // bpmt = 10*bpm -> 10*sixteenth / min:
  u32 spmt = bpmt*4;
  // milliseconds per 16th:
  u32 msps = (600*1000)/spmt;
  frac += (600*1000)%spmt;
  if(frac>=(600*1000)) {
    frac -= (600*1000);
    msTc--;
  }
  if(msTc>=msps) {
    msTc -= msps;
    if(internalSync)
      seq_ca_updateTime();
  }
}
