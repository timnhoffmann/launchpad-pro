#define GENERAL
#include "general.h"
#include "note.h"
#include "seq_ca.h"
// store the pressed state of the buttons:
u16 buttonState[10] = {0,0,0,0,0,0,0,0,0,0};

u8 getButtonState(u8 i, u8 j) {
  return (buttonState[j] & (1<<i) ) != 0;
}

u8 getButtonStateIndex(u8 index) {
  int i = index%10;
  int j = index/10;
  return (buttonState[j] & (1<<i) ) != 0;
}

void setMode(u8 m) {
  mode = m;
  switch (mode) {
	case MODE_SEQ_STEP:
	  {}
	  break;
	case MODE_NOTE:
	  {
	    note_mode_init();
	  }
	  break;
	case MODE_SEQ_CA:
	  {
	    seq_ca_mode_init();
	  }
	  break;
	case MODE_USER:
	  {}
	  break;
	case MODE_SEQ_STEP_SETUP:
	  {}
	  break;
	case MODE_NOTE_SETUP:
	  {
	    note_setup_init();
	  }
	  break;
	case MODE_SEQ_CA_SETUP:
	  {
	    seq_ca_setup_init();
	  }
	  break;
	case MODE_USER_SETUP:
	  {}
	  break;
	}
}

//______________________________________________________________________________
//
// helpers common for several modes
//______________________________________________________________________________
//

void chooseMIDI_init(u8 currentChannel) {
  // the Pad-no for the MIDI-channel
  u8 mc = currentChannel;
  mc = mc%8 +1 + (mc/8 +1)*10;
  // light the channel choice pads:
  for(int i = 11; i<19; i++) {
    u8 v = i==mc?MAXLED:5;
    hal_plot_led(TYPEPAD, i, 0, 0, v);
  }
  for(int i = 21; i<29; i++) {
    u8 v = i==mc?MAXLED:5;
    hal_plot_led(TYPEPAD, i, 0, 0, v);
  }
}

u8 isChooseMIDI(u8 index) {
  u8 i = index%10;
  u8 j = index/10;
  return (i>0) && (i<9) && (j>0) && (j<3);
    }

u8 chooseMIDI(u8 index) {
  u8 i = index%10;
  u8 j = index/10;
  return (i-1) + 8*(j-1);
}

//______________________________________________________________________________
//
// helper for bitwise left and right rotation of u8 values
//______________________________________________________________________________
//

u8 rotl8 (u8 value, unsigned int count) {
    const unsigned int mask = (CHAR_BIT*sizeof(value)-1);
    count &= mask;
    return (value<<count) | (value>>( (-count) & mask ));
}

u8 rotr8 (u8 value, unsigned int count) {
    const unsigned int mask = (CHAR_BIT*sizeof(value)-1);
    count &= mask;
    return (value>>count) | (value<<( (-count) & mask ));
}

//______________________________________________________________________________
//
// totally naive and almost unusable rand -
// still enough to pseudo-randomly fill the grid...
// with some input from here:
// http://www.electro-tech-online.com/threads/ultra-fast-pseudorandom-number-generator-for-8-bit.124249/
//______________________________________________________________________________
//

u8 seed1 = 19;
u8 seed2 = 93;
u8 seed3 = 27;
u8 seed4 = 1;
u8 simple_rand() {
  seed4++;
  seed1= (seed1^seed3^seed4);
  seed2 = (seed2+seed1);
  seed3 =  (seed3+ ((seed2>>1)^seed1));
  return seed3;
}
