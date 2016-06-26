
#include "note.h"

//_________________________________________________
//
// NOTE mode
//_________________________________________________
//

u8 note_root = 36;
u8 note_channel = 0;

void note_mode_init() {
  for(int j = 0; j<8;j++) {
    hal_plot_led(TYPEPAD, (j+1)*10+9, 0, 0, 0);
    for(int i = 0; i<8; i++) {
      u8 note = note_root + i + j*5;
      u8 r = 0;
      u8 g = 0;
      u8 b = 0;
      //if(note%12 == note_root%12) { r = 32;b=32;}
      if(note%12 == 0) {r=32; g = 32;}
      if(note==MIDDLE_C) {r=MAXLED,g=b=0;}//for reference and orientation
      hal_plot_led(TYPEPAD, (j+1)*10+i+1, r, g, b);
    }
  }
}

void note_aftertouch(u8 index, u8 value) {
  /* u8 i = index%10; */
  /* u8 j = index/10; */
  /* if((i>0) && (i<9) && (j>0) && (j<9)) { */
  /*   u8 note = note_root + (i-1) + (j-1)*5; */
  /*   hal_send_midi(midiport, POLYAFTERTOUCH | note_channel, note, value); */
  /* } */
}


void note_typepad(u8 index, u8 value) {

  u8 i = index%10;
  u8 j = index/10;
  if((i>0) && (i<9) && (j>0) && (j<9)) {
    u8 note = note_root + (i-1) + (j-1)*5;
    if(value != 0) {
      //      if(!getButtonState(i,j)) { // the button is freshly pressed
	//play note
	hal_send_midi(midiport, NOTEON | note_channel, note, value);
	//	hal_plot_led(TYPESETUP, 0, 0, MAXLED, 0);
	//      }
    } else {
      //      hal_plot_led(TYPESETUP, 0, MAXLED, 0, 0);
      hal_send_midi(midiport, NOTEON | note_channel, note, 0);
      u8 r = 0;
      u8 g = 0;
      u8 b = 0;
      //if(note%12 == note_root%12) { r = 32;b=32;}
      if(note%12 == 0) {r=32; g = 32;}
      if(note==MIDDLE_C) {r=MAXLED,g=b=0;}// for reference and orientation
      hal_plot_led(TYPEPAD, index, r, g, b);
    }
    // update the LED:
    //    seq_ca_updateLED(i,j);
    return;
  } else if(value) {
    switch (index) {
    case BUTTON_UP:
      {
	if(note_root>=5){
	  note_root-=5;
	  note_mode_init();
	}
      }
      break;
    case BUTTON_DOWN:
      {
	if(note_root <=150) {
	  note_root+=5;
	  note_mode_init();
	}
      }
      break;
    case BUTTON_LEFT:
      {
	if(note_root <155) {
	  note_root+=1;
	  note_mode_init();
	}
      }
      break;
    case BUTTON_RIGHT:
      {
	if(note_root>0) {
	  note_root-=1;
	  note_mode_init();
	}
      }
      break;
    }
  }
    all_modes_typepad(index,value);
}

void note_setup_init() {
  //blank the borders
  for(int i = 0; i<8; i++) {
    hal_plot_led(TYPEPAD, i+1, 0,0,0);
    hal_plot_led(TYPEPAD, 10*(i+1), 0,0,0);
    hal_plot_led(TYPEPAD, 10*(i+1)+9, 0,0,0);
  }
  hal_plot_led(TYPEPAD, BUTTON_UP, 0,0,0);
  hal_plot_led(TYPEPAD, BUTTON_DOWN, 0,0,0);
  hal_plot_led(TYPEPAD, BUTTON_LEFT, 0,0,0);
  hal_plot_led(TYPEPAD, BUTTON_RIGHT, 0,0,0);

  // blank the upper pads
  for(int j = 0; j<5;j++)
    for(int i = 0; i<8; i++) {
      hal_plot_led(TYPEPAD, (j+3)*10+i+1, 0,0,0);
    }
  // setup Channel choice:
  chooseMIDI_init(note_channel);
}

void note_setup_typepad(u8 index, u8 value) {
  if((value!=0 ) && isChooseMIDI(index)) {
    note_channel = chooseMIDI(index);
    chooseMIDI_init(note_channel);
  } else
    all_modes_typepad(index,value);
}
