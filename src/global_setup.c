#include "app.h"
#include "general.h"

int ipow(int v, u8 exp) {
  int r = 1;
  for(int i = 0; i<exp; i++) r*=v;
  return r;
}

void chooseBPM_init() {
  // set the lower four ros of pads to choose the bpm:
  u16 no = bpmt;
  for (int j=0; j<4;j++) {
    u8 rest = no%10;
    for(int i = 1; i<=9; i++) {
      u8 v = i==rest?MAXLED:5;
      hal_plot_led(TYPEPAD, (i)+10*(j+1), 0, 0, v);
    }
    no = no/10;
  }

}

u8 isChooseBPM(u8 index) {
  u8 i = index%10;
  u8 j = index/10;
  return (i>0) && (j>0) && (j<5);
    }

u16 chooseBPM(u8 index) {
  u8 i = index%10 ;
  u8 j = index/10 -1;
  u16 ttj = ipow(10,j);
  i = ((bpmt/ttj)%10) == i?0:i;
  return (10*ttj)*(bpmt/(10*ttj)) + bpmt%(ttj) + (ttj * i);
}

void global_setup_init() {
  chooseBPM_init();
  all_modes_init();
}

void global_setup_typepad(u8 index, u8 value) {
  if(isChooseBPM(index) && value) {
    bpmt = chooseBPM(index);
    chooseBPM_init(); // to reflect the changed state
  } else all_modes_typepad(index, value);
}
