#include "timing.h"
#include "seq_ca.h"
#include "seq_step.h"


// we should change the divider by a factor of 12 --
// then bpmt*48 would give 16th, *3 would give whole notes, *6 half notes, *9 half triplets
// *12 quater notes, *18 quarter triplets, *24 8th, +36 8th triplets,
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
    if(internalSync) {
      seq_ca_updateTime();
      seq_step_play();
    }
  }
}
