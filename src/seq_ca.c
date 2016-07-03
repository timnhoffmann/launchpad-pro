/******************************************************************************
 
 Copyright (c) 2016, timh
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 * Neither the name of Focusrite Audio Engineering Ltd., nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 *****************************************************************************/

//______________________________________________________________________________
//
// Headers
//______________________________________________________________________________

#include "seq_ca.h"
#include "colors.h"

//#include "app.h"
//#include "general.h"

//_________________________________________________
//
// SEQ_CA mode
//_________________________________________________
//

#define SEQ_CA_SCENE_BR  16

//bits for playing instr
u8 playing = 0; 
// the ca bits
u8 slots[8] = {0,0,0,0,0,0,0,0};

const int SIZEM = 8;

// base note per instrument
int seq_ca_rootNote[8] = {60,62,64,65,67,69,36,36};

// current note per instrument
int seq_ca_noteNr[8] = {60,62,64,65,67,69,36,36};

// midi channel per instrument
u8 seq_ca_channel[8] = {9,9,9,9,9,9,0,0};

// active / mute state for instruments:
u8 seq_ca_active = 0b11111111;

// the bits that map to pitch
u8 seq_ca_noteBits[8] = {0,0,0,0,0,0,255,255};

// flag for double or single timestep
u8 doubleTime = 1;

u8 seq_ca_getBit(u8 i, u8 j) {
  return (u8) (slots[j] & (1<<i)) != 0?1:0;
}

// update a single LED indices are 0..7 x 8..7

void seq_ca_updateLED(u8 i, u8 j) {
  u8 index = (j+1)*10 + i +1;
  ((slots[j]>>i)&1) == 1?
    hal_plot_led(TYPEPAD, index, CA_PAD)
    :
    hal_plot_led(TYPEPAD, index, BLACK);
}

void seq_ca_updateLEDs() {
  for(int j = 0; j<SIZEM; j++)
    for(int i = 0; i<8;i++) {
      seq_ca_updateLED(i,j);
    }
}


void seq_ca_clear() {
  for(int i = 0; i<SIZEM; i++)
    slots[i] = 0;
  seq_ca_updateLEDs();
}

void seq_ca_random() {
  for(int i = 0; i<SIZEM; i++)
    slots[i] = simple_rand();
  seq_ca_updateLEDs();
}

void seq_ca_mode_init() {
  seq_ca_updateLEDs();
  // clear the lower Button row and set the scene buttons:
  for(int i = 0; i<8; i++) {
    hal_plot_led(TYPEPAD, i+1, BUTTON_OFF);
    if((seq_ca_active & (1<<(i))) != 0)
      hal_plot_led(TYPEPAD, (i+1)*10+9, INST_ACTIVE);
    else
      hal_plot_led(TYPEPAD, (i+1)*10+9, INST_INACTIVE);
    // u8 v = ((seq_ca_active & (1<<(i))) != 0)? SEQ_CA_SCENE_BR:0;
    // hal_plot_led(TYPEPAD, (i+1)*10+9, v, v, v);
  }
  if(doubleTime)
    hal_plot_led(TYPEPAD, BUTTON_DOUBLE, BUTTON_ON);
  else
    hal_plot_led(TYPEPAD, BUTTON_DOUBLE, BUTTON_OFF);
  // u8 v = doubleTime?MAXLED:0;
  // hal_plot_led(TYPEPAD, BUTTON_DOUBLE, v, v, v);
  if(seq_ca_running)
    hal_plot_led(TYPEPAD, BUTTON_CIRCLE, BUTTON_ON);
  else
        hal_plot_led(TYPEPAD, BUTTON_CIRCLE, BUTTON_OFF);
  // v = seq_ca_running?MAXLED:0;
  // hal_plot_led(TYPEPAD, BUTTON_CIRCLE, v, v, v);
  all_modes_init();
}

void seq_ca_update(int t) {
  
  for(int j = 0; j<SIZEM; j++) {
    u8 mask = (u8) ((t+j)%2 == 0? 0b10101010:0b01010101);
    u8 omask = (u8) ((t+j+1)%2 == 0? 0b10101010:0b01010101);
    u8 u = slots[(j+SIZEM-1)%SIZEM];
    u8 d = slots[(j+1)%SIZEM] ;
    u8 r = rotl8(slots[j],1);
    u8 l = rotr8(slots[j],1);
    u8 sum = (u8)(
		  (~(((u^d) ^ (l^r)) | (u & d & l & r ))) & (u | d | l | r)
		  );
    slots[j] = (u8) (((sum ^ slots[j]) & mask) | (slots[j] & omask)); 
  }
}

u8 scale[14] = {0,0,2,2,4,4,5,5,7,7,9,9,11,11};

u8 seq_ca_makeNote(int i) {
  if(seq_ca_noteBits[i]) {
    u8 v = slots[i]&seq_ca_noteBits[i];
    return (u8) ( 0b01111111 &
		  (seq_ca_rootNote[i]
		   - ((v>>0)&1)*12
		   + ((v>>1)&1)*12
		   + ((v>>2)&1)*7
		   + ((v>>3)&1)*5
		   + ((v>>4)&1)*3
		   + ((v>>5)&1)*2
		   + ((v>>6)&1)*2
		   + ((v>>7)&1)*1
		   )
		  );
  } else return seq_ca_rootNote[i];
  /*
  return (u8) ( 0b01111111 &
  (36
                - seq_ca_getBit(0, i)*12
                +seq_ca_getBit(1, i)*12
                +seq_ca_getBit(2, i)*7
                +seq_ca_getBit(3, i)*5
                +seq_ca_getBit(4, i)*3
                +seq_ca_getBit(5, i)*2
                +seq_ca_getBit(6, i)*1
  		)
  	       );  
  */
    }

void seq_ca_toggle_running() {
  seq_ca_running = !seq_ca_running;
  if(!seq_ca_running) {
    seq_ca_noteOff_all();
    hal_plot_led(TYPEPAD, BUTTON_CIRCLE, BUTTON_OFF);
  } else
    hal_plot_led(TYPEPAD, BUTTON_CIRCLE, BUTTON_ON);
}

void seq_ca_noteOff_all() {
  for(int i = 0; i<8; i++)
    seq_ca_noteOff(i);
}

void seq_ca_noteOff(u8 i) {
  if( (playing >> i)%2 ==1) {
    hal_send_midi(midiport, NOTEON | seq_ca_channel[i], seq_ca_noteNr[i], 0);
    playing = (u8) (playing & (~(1<<i)));
    if(mode == MODE_SEQ_CA) {
      (seq_ca_active & 1<<i) ? hal_plot_led(TYPEPAD, (i+1)*10+9, INST_ACTIVE):hal_plot_led(TYPEPAD, (i+1)*10+9, INST_INACTIVE);
    }
  }
}

void seq_ca_updateTime() {
  for(int i = 0; i<8; i++) {
    if(seq_ca_active & (1<<i)) {
      int c = 0; 
      for(int j = 0; j<SIZEM;j++) {
	//int bit = (slots[j]>>>i) & 0b1;
	c += (slots[i]>>j) & 0b1;  
      }
      int on = (c%2 == 1);
      if(on  & ! ( (playing >> i)%2 ==1) ) {
	seq_ca_noteNr[i] = seq_ca_makeNote(i);
	hal_send_midi(midiport, NOTEON | seq_ca_channel[i], seq_ca_noteNr[i], 16*c); // FIXEDV?96:16*c
	if(mode == MODE_SEQ_CA)
	  hal_plot_led(TYPEPAD, (i+1)*10+9, INST_PLAYING);
	playing =(u8) (playing | (1<<i));
      } else if((!on) /*&  ( (playing >> i)%2 ==1) */) {
	//	hal_send_midi(midiport, NOTEON | seq_ca_channel[i], seq_ca_noteNr[i], 0);
	//	playing = (u8) (playing & (~(1<<i)));
	seq_ca_noteOff(i);
      }
    }
  }
  if(mode == MODE_SEQ_CA) 
    seq_ca_updateLEDs(); // make new state visible

}


void seq_ca_play(u8 frac) {
  
  if((frac == 0) && seq_ca_running) {
    seq_ca_updateTime(); // send out notes
    seq_ca_update(++time); // calculate next state...
    if(doubleTime)
      seq_ca_update(++time); //...twice...
  }
}
 
void seq_ca_typepad(u8 index, u8 value) {
  //  if (value == 0) return;
  // grid positions of the buttons
  u8 i = index%10;
  u8 j = index/10;

  if((i>0) && (i<9) && (j>0) && (j<9)) { // pad pressed:
    if(value != 0) { // the button is freshly pressed
      // 8x8 grid position
      --i;
      --j;
      // toggle the bit in the CA
      slots[j] ^= 1<<i;
      //hal_plot_led(TYPESETUP, 0, MAXLED, 0, 0);
    } //else hal_plot_led(TYPESETUP, 0, 0, 0, 0);
    // update the LED to reflect the bit state:
    seq_ca_updateLED(i,j);
  } else if((index%10) == 9) { // a scene button:
    if(value) {
      int i = (index/10) - 1;
      //toggle the active state and adjust the LED:
      seq_ca_active ^= 1<< i;
      (seq_ca_active & 1<<i) ? hal_plot_led(TYPEPAD, index, INST_ACTIVE): hal_plot_led(TYPEPAD, index, INST_INACTIVE);
      seq_ca_noteOff(i);
    }
  } else if (index == BUTTON_DOUBLE) { // button for time / double-time -- should not default to switching off in case of value==0
    if(value) {
      doubleTime = !doubleTime;
      doubleTime ? hal_plot_led(TYPEPAD, BUTTON_DOUBLE, BUTTON_ON) : hal_plot_led(TYPEPAD, BUTTON_DOUBLE, BUTTON_ON);
    }
  } else if(index == BUTTON_CIRCLE) {
    if(value)
     seq_ca_toggle_running();
  } else if(value !=0) {// not a scene button
      switch (index) {
      /* case BUTTON_DOUBLE: // double time */
      /* 	{ */
      /* 	  doubleTime = !doubleTime; */
      /* 	  u8 v = (doubleTime?MAXLED:0); */
      /* 	  hal_plot_led(TYPEPAD, BUTTON_DOUBLE, v, v, v); */
      /* 	  return; */
      /* 	} */
      /* 	break; */
      case BUTTON_DELETE: // clear the grid
	{
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  //	if(!getButtonState(i,j))
	  seq_ca_clear();
	}
	break;
      case BUTTON_QUANTIZE: // fill randomly
	{
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  //	if(!getButtonState(i,j))
	  seq_ca_random();
	}
	break;
      case BUTTON_UNDO: // reverse time
	{
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  //	if(!getButtonState(i,j))
	  time++;
	}
	break;
      case BUTTON_UP: // shifts pattern up
	{
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  //	if(!getButtonState(i,j)) {
	  u8 tmp = slots[SIZEM-1];
	  for(int i = SIZEM-1; i>0;--i)
	    slots[i] = slots[i-1];
	  slots[0] = tmp;
	  seq_ca_updateLEDs();
	  time++;
	  //	}
	}
	break;
      case BUTTON_DOWN: // shifts pattern down
	{
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  //	if(!getButtonState(i,j)) {
	  u8 tmp = slots[0];
	  for(int i = 0; i<SIZEM-1;++i)
	    slots[i] = slots[i+1];
	  slots[SIZEM-1] = tmp;
	  seq_ca_updateLEDs();
	  time++;
	  //	  }
	}
	break;
      case BUTTON_LEFT: // shifts pattern left
	{
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  //	if(!getButtonState(i,j)) {
	  for(int i = 0; i<SIZEM;i++)
	    slots[i] = rotr8(slots[i],1);
	  seq_ca_updateLEDs();
	  time++;
	  //	}
	}
	break;
      case BUTTON_RIGHT: // shifts pattern right
	{
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  //	if(!getButtonState(i,j)) {
	  for(int i = 0; i<SIZEM;i++)
	    slots[i] = rotl8(slots[i],1);
	  seq_ca_updateLEDs();
	  time++;
	  //	}
	}
	break;
      }
  } else // value == 0
    hal_plot_led(TYPEPAD, index, BUTTON_OFF);
  //TODO should happen insdead of switching off for the relevant Buttons...
  all_modes_typepad(index,value);
}

//_________________________________________________
//
// SEQ_CA setup mode
//_________________________________________________
//

u8 seq_ca_setup_inst = 0;
u8 seq_ca_setup_note_root = 48;

void seq_ca_setup_init() {
  // clear the bottom buttons and set the scene buttons to represent the current instrument
  for(int i = 0; i<8; i++) {
    hal_plot_led(TYPEPAD, i+1, BUTTON_OFF);
    (i==seq_ca_setup_inst) ? hal_plot_led(TYPEPAD, (i+1)*10+9, INST_CURRENT) : hal_plot_led(TYPEPAD, (i+1)*10+9, INST_SELECT);
  }
  
  // the Pad-no for the MIDI-channel
  chooseMIDI_init(seq_ca_channel[seq_ca_setup_inst]);
  /* u8 mc = seq_ca_channel[seq_ca_setup_inst]; */
  /* mc = mc%8 +1 + (mc/8 +1)*10; */
  /* // light the channel choice pads: */
  /* for(int i = 11; i<19; i++) { */
  /*   u8 v = i==mc?MAXLED:5; */
  /*   hal_plot_led(TYPEPAD, i, 0, 0, v); */
  /* } */
  /* for(int i = 21; i<29; i++) { */
  /*   u8 v = i==mc?MAXLED:5; */
  /*   hal_plot_led(TYPEPAD, i, 0, 0, v); */
  /* } */

  // light the note choice pads:
  for(int j = 0; j<5;j++)
    for(int i = 0; i<8; i++) {
      u8 note = seq_ca_setup_note_root + i + j*5;
      /*
      u8 r = 0;
      u8 g = 0;
      u8 b = 0;
      //if(note%12 == seq_ca_setup_note_root%12) { r = 32;b=32;}
      if(note%12 == 0) {r=32; g = 32;}
      if(note==MIDDLE_C) {r=MAXLED,g=b=0;}// for reference and orientation
      if(note==seq_ca_rootNote[seq_ca_setup_inst]) {r=g=b=MAXLED;}// the current base note
      hal_plot_led(TYPEPAD, (j+3)*10+i+1, r, g, b);
      */
      u8* c = black;

      if(note%12 == 0)
	c = note_c;
      if(note==MIDDLE_C) // for reference and orientation
	c = note_middle_c;
      if(note==seq_ca_rootNote[seq_ca_setup_inst]) // the current base note
	c = note_playing;
      hal_plot_led(TYPEPAD, (j+3)*10+i+1, COLOR(c));
    }

  // the upper pad row for the noteBits:
  u8 noteBits = seq_ca_noteBits[seq_ca_setup_inst];
  for(int i = 0; i<8; i++) {
    ((noteBits & (1<<i)) != 0) ? hal_plot_led(TYPEPAD, 81+i, NOTEBIT_ON) : hal_plot_led(TYPEPAD, 81+i, NOTEBIT_OFF);
  }
  seq_ca_running ? hal_plot_led(TYPEPAD, BUTTON_CIRCLE, BUTTON_ON) : hal_plot_led(TYPEPAD, BUTTON_CIRCLE, BUTTON_OFF);
 
  all_modes_init();
}

void seq_ca_setup_typepad(u8 index, u8 value) {
  u8 i = index%10;
  u8 j = index/10;
  if((i>0) && (i<9) && (j>2) && (j<9)) { // select root note
    if(value != 0) {
      //set note as root or set the noteBit:
      if(j==8) { // noteBit
	seq_ca_noteBits[seq_ca_setup_inst] ^= 1<<(i-1);
      } else { // note
	u8 note = seq_ca_setup_note_root + (i-1) + (j-3)*5;
	seq_ca_rootNote[seq_ca_setup_inst] = note;
      }
    } else {
      seq_ca_setup_init();
    }
  } else if(isChooseMIDI(index)) { // select midi channel
    seq_ca_noteOff(i);
    seq_ca_channel[seq_ca_setup_inst] = chooseMIDI(index);
    chooseMIDI_init(seq_ca_channel[seq_ca_setup_inst]);
  } else if(index == BUTTON_CIRCLE) { // play/pause
    if(value)
     seq_ca_toggle_running();
  } else if(value) {
    if(index%10 == 9) { // scene buttons select the instrument 1-8 to setup
      seq_ca_setup_inst = (index/10)-1;
      seq_ca_setup_init();
    } else
    switch (index) {
    case BUTTON_UP:
      {
	if(seq_ca_setup_note_root>=5){
	  seq_ca_setup_note_root-=5;
	  seq_ca_setup_init();
	}
      }
      break;
    case BUTTON_DOWN:
      {
	if(seq_ca_setup_note_root <=150) {
	  seq_ca_setup_note_root+=5;
	  seq_ca_setup_init();
	}
      }
      break;
    case BUTTON_LEFT:
      {
	if(seq_ca_setup_note_root <155) {
	  seq_ca_setup_note_root+=1;
	  seq_ca_setup_init();
	}
      }
      break;
    case BUTTON_RIGHT:
      {
	if(seq_ca_setup_note_root>0) {
	  seq_ca_setup_note_root-=1;
	  seq_ca_setup_init();
	}
      }
      break;
    }
    all_modes_typepad(index,value);
  } 
}
