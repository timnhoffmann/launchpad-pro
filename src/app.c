/******************************************************************************
 
 Copyright (c) 2015, Focusrite Audio Engineering Ltd.
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

#include "app.h"
#include <limits.h>  // for def of CHAR_BIT

//__________________________________________
//
// general consts and vars
//__________________________________________
//

// the mode - can be MODE_SEQ_CA, MODE_SEQ_STEP, MODE_NOTES, MODE_USER
u8 mode = MODE_SEQ_CA;


u8  playing = 0; // bits encode playing of notes...
u32 time = 0;
u8  running = 0;
u8  clocksteps = 0;
// tempo estimate in ms/beat
u32 tempo = 500; // 120 bpm

// store the pressed state of the buttons:
u16 buttonState[10] = {0,0,0,0,0,0,0,0,0,0};

/**
 * returns wether the button at (i,j) is currently pressed.
 * the state gets updated AFTER each press is evaluated (so for a given event it gives the state of that button before the event).
 * @param i -- the column
 * @param j -- the row
 */
u8 getButtonState(u8 i, u8 j) {
  return (buttonState[j] & (1<<i) ) != 0;
}

/**
 * returns wether the button at index is currently pressed.
 * the state gets updated AFTER each press is evaluated (so for a given event it gives the state of that button before the event).
 * @param index -- the button index
 */
u8 getButtonStateIndex(u8 index) {
  int i = index%10;
  int j = index/10;
  return (buttonState[j] & (1<<i) ) != 0;
}

//_________________________________________________
//
// SEQ_CA mode
//_________________________________________________
//

const int SIZEM = 8;
int seq_ca_noteNr[8] = {60,62,64,65,67,69,36,36};
u8 seq_ca_channel[8] = {9,9,9,9,9,9,0,0};
u8 slots[8] = {0,0,0,0,0,0,0,0};


u8 seq_ca_getBit(u8 i, u8 j) {
  return (u8) (slots[j] & (1<<i)) != 0?1:0;
}

// update a single LED indices are 0..7 x 8..7

void seq_ca_updateLED(u8 i, u8 j) {
u8 index = (j+1)*10 + i +1;
      ((slots[j]>>i)&1) == 1?
	hal_plot_led(TYPEPAD, index, MAXLED, MAXLED, MAXLED)
	:
	hal_plot_led(TYPEPAD, index, 0,0,0);
}

void seq_ca_updateLEDs() {
  for(int j = 0; j<SIZEM; j++)
    for(int i = 0; i<8;i++) {
      seq_ca_updateLED(i,j);
    }
}



/**
 * clear the SEQ_CA
 */
void seq_ca_clear() {
  for(int i = 0; i<SIZEM; i++)
    slots[i] = 0;
  seq_ca_updateLEDs();
}

/**
 * fill the SEQ_CA randomly
 */
void seq_ca_random() {
  for(int i = 0; i<SIZEM; i++)
    slots[i] = simple_rand();
  seq_ca_updateLEDs();
}

void seq_ca_mode_init() {
  seq_ca_updateLEDs();
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
u8 makeNote(int i) {
  
  return (u8) ( 0b01111111 &
  (36
                - seq_ca_getBit(i, 0)*12
                +seq_ca_getBit(i, 1)*12
                +seq_ca_getBit(i, 2)*7
                +seq_ca_getBit(i, 3)*5
                +seq_ca_getBit(i, 4)*3
                +seq_ca_getBit(i, 5)*2
                +seq_ca_getBit(i, 6)*1
  		)
  	       );
  
  /* return (u8) (0b01111111 & */
  /* 	       (36 */
  /* 		+ scale[((seq_ca_getBit(i, 0)*1 */
  /* 		+seq_ca_getBit(i, 1)*2 */
  /* 		+seq_ca_getBit(i, 2)*4 */
  /* 		+seq_ca_getBit(i, 3)*8 */
  /* 		+seq_ca_getBit(i, 4)*16 */
  /* 		+seq_ca_getBit(i, 5)*23 */
  /* 		+seq_ca_getBit(i, 6)*64 */
  /* 			  +seq_ca_getBit(i, 7)*128)/2)%14] */
  /* 			     ) */
  /* 	       ); */
    }

// Play the current state as midi notes:
void seq_ca_play() {
  for(int i = 0; i<8; i++) {
    int c = 0; 
    for(int j = 0; j<SIZEM;j++) {
      //int bit = (slots[j]>>>i) & 0b1;
      c += (slots[j]>>i) & 0b1;  // TODO: we had here >>> (same for the palying >> below...)
    }
    int on = (c%2 == 1);
    if(on  & ! ( (playing >> i)%2 ==1) ) {
      if(i>5) {
	seq_ca_noteNr[i] = makeNote(i);
      }
      hal_send_midi(USBMIDI, NOTEON | seq_ca_channel[i], seq_ca_noteNr[i], 16*c); // FIXEDV?96:16*c
      playing =(u8) (playing | (1<<i));
    } else if((!on) &  ( (playing >> i)%2 ==1) ) {
      hal_send_midi(USBMIDI, NOTEON | seq_ca_channel[i], seq_ca_noteNr[i], 0);
      playing = (u8) (playing & (~(1<<i)));
    }
  }      
}


/**
 * the TYPEPAD response for the SEQ_CA mode
 * @param index - index of the button
 * @param vsalue - velocity of the button
 */
void seq_ca_typepad(u8 index, u8 value) {
  //  if (value == 0) return;
  // grid positions of the buttons
  u8 i = index%10;
  u8 j = index/10;
  //  if((value!=0) & (i>0) & (i<9) & (j>0) & (j<9)) { // square button pressed:
  //    if(!getButtonState(i,j)) { // the button is freshly pressed
  if((i>0) & (i<9) & (j>0) & (j<9)) { // square button pressed:
    if(value != 0) { // the button is freshly pressed
      // 8x8 grid position
      --i;
      --j;
      // toggle the bit in the CA
      slots[j] ^= 1<<i;
    }
    // update the LED to reflect the bit state:
    seq_ca_updateLED(i,j);
  } else { //not
    switch (index) {
    case BUTTON_DELETE: // clear the grid
      {
	hal_plot_led(TYPEPAD, index, value, value, value);
	if(!getButtonState(i,j))
	  seq_ca_clear();
      }
      break;
    case BUTTON_QUANTIZE: // fill randomly
      {
	hal_plot_led(TYPEPAD, index, value, value, value);
	if(!getButtonState(i,j))
	  seq_ca_random();
      }
      break;
    case BUTTON_UNDO: // reverse time
      {
	hal_plot_led(TYPEPAD, index, value, value, value);
	if(!getButtonState(i,j))
	  time++;
      }
      break;
    case BUTTON_UP: // shifts pattern up
      {
	hal_plot_led(TYPEPAD, index, value, value, value);
	if(!getButtonState(i,j)) {
	  u8 tmp = slots[SIZEM-1];
	  for(int i = SIZEM-1; i>0;i--)
	    slots[i] = slots[i-1];
	  slots[0] = tmp;
	  seq_ca_updateLEDs();
	  time++;
	}
      }
      break;
    case BUTTON_DOWN: // shifts pattern down
      {
	hal_plot_led(TYPEPAD, index, value, value, value);
	if(!getButtonState(i,j)) {
	  u8 tmp = slots[0];
	  for(int i = 0; i<SIZEM-1;i++)
	    slots[i] = slots[i+1];
	  slots[SIZEM-1] = tmp;
	  seq_ca_updateLEDs();
	  time++;
	}
      }
      break;
    case BUTTON_LEFT: // shifts pattern left
      {
	hal_plot_led(TYPEPAD, index, value, value, value);
	if(!getButtonState(i,j)) {
	  for(int i = 0; i<SIZEM;i++)
	    slots[i] = rotr8(slots[i],1);
	  seq_ca_updateLEDs();
	  time++;
	}
      }
      break;
    case BUTTON_RIGHT: // shifts pattern right
      {
	hal_plot_led(TYPEPAD, index, value, value, value);
	if(!getButtonState(i,j)) {
	  for(int i = 0; i<SIZEM;i++)
	    slots[i] = rotl8(slots[i],1);
	  seq_ca_updateLEDs();
	  time++;
	}
      }
      break;

    }
  }
}

//_________________________________________________
//
// NOTE mode
//_________________________________________________
//

u8 note_root = 36;
u8 note_channel = 0;

void note_mode_init() {
  for(int j = 0; j<8;j++)
    for(int i = 0; i<8; i++) {
      u8 note = note_root + i + j*5;
      u8 r = 0;
      u8 g = 0;
      u8 b = 0;
      if(note%12 == note_root%12) { r = 32;b=32;}
      if(note%12 == 0) {r=32; g = 32;}
      hal_plot_led(TYPEPAD, (j+1)*10+i+1, r, g, b);
    }
}

/**
 * the TYPEPAD response for the NOTE mode
 * @param index - index of the button
 * @param vsalue - velocity of the button
 */

void note_typepad(u8 index, u8 value) {
  u8 i = index%10;
  u8 j = index/10;
  if((i>0) & (i<9) & (j>0) & (j<9)) {
    u8 note = note_root + (i-1) + (j-1)*5;
    if(value != 0) {
      if(!getButtonState(i,j)) { // the button is freshly pressed
	//play note
	hal_send_midi(USBMIDI, NOTEON | note_channel, note, value);
      }
    } else {
      hal_send_midi(USBMIDI, NOTEON | note_channel, note, value);
      u8 r = 0;
      u8 g = 0;
      u8 b = 0;
      if(note%12 == note_root%12) { r = 32;b=32;}
      if(note%12 == 0) {r=32; g = 32;}
      hal_plot_led(TYPEPAD, index, r, g, b);
    }
    // update the LED:
    //    seq_ca_updateLED(i,j);
  } else if(value) {
    switch (index) {
    case BUTTON_UP:
      {
	note_root+=5;
	note_mode_init();
      }
      break;
    case BUTTON_DOWN:
      {
	note_root-=5;
	note_mode_init();
      }
      break;
    case BUTTON_LEFT:
      {
	note_root-=1;
	note_mode_init();
      }
      break;
    case BUTTON_RIGHT:
      {
	note_root+=1;
	note_mode_init();
      }
      break;
    }
  }
}

//_________________________________________________
//
// general stuff
//_________________________________________________
//


void midiTick() {
  clocksteps = (clocksteps+1)%6;// 16th for now
  if(clocksteps == 0) { 
    seq_ca_play(); // send out notes
    if(mode == MODE_SEQ_CA) 
      seq_ca_updateLEDs(); // make new state visible
    seq_ca_update(++time); // calculate next state
    seq_ca_update(++time); //
  }
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
	}
}
  
//______________________________________________________________________________
//
// This is where the fun is!  Add your code to the callbacks below to define how
// your app behaves.
//______________________________________________________________________________

void app_surface_event(u8 type, u8 index, u8 value) {
  switch (type)
    {
    case  TYPEPAD:
      {
	// example - light / extinguish pad LEDs, send MIDI
	//hal_plot_led(TYPEPAD, index, value, value, value);
	//hal_send_midi(DINMIDI, NOTEON | 0, index, value);
	int i = index%10;
	int j = index/10;
	switch (mode) {
	case MODE_SEQ_STEP:
	  {}
	  break;
	case MODE_NOTE:
	  {
	    note_typepad(index,value);
	  }
	  break;
	case MODE_SEQ_CA:
	  {
	    seq_ca_typepad(index,value);
	  }
	  break;
	case MODE_USER:
	  {}
	  break;
	}
	if((index >= BUTTON_SESSION) & !(buttonState[j]&(1<<i))) {
	  if(mode != index) {
	    hal_plot_led(TYPEPAD, mode, 0, 0, 0);
	    hal_plot_led(TYPEPAD, index, MAXLED, MAXLED, MAXLED);
	    setMode(index);
	  }
	}
	// update the buttonState
	buttonState[j] ^= (buttonState[j] ^ ((value?1:0)<<i) ) & (1<<i); 
      }
      break;
      
    case TYPESETUP: // switch to setup mode
      {
	// just light the LED  for now...
	hal_plot_led(TYPESETUP, 0, value, value, value);
      }
      break;
    }
}

//______________________________________________________________________________

void app_midi_event(u8 port, u8 status, u8 d1, u8 d2)
{
	// example - MIDI interface functionality for USB "MIDI" port -> DIN port
	if (port == USBMIDI)
	{
		hal_send_midi(DINMIDI, status, d1, d2);
	}

	// // example -MIDI interface functionality for DIN -> USB "MIDI" port port
	if (port == DINMIDI)
	{
		hal_send_midi(USBMIDI, status, d1, d2);
	}

	switch(status) {
	case MIDISTART:
	case MIDICONTINUE:
	  {
	  running = 1;
	  }
	  break;
	case MIDISTOP:
	  {
	  running = 0;
	  }
	  break;
	case MIDITIMINGCLOCK:
	  {
	  midiTick();
	  }
	  break;
	default:
	  break;
	}
}

//______________________________________________________________________________

void app_sysex_event(u8 port, u8 * data, u16 count)
{
	// example - respond to UDI messages?
}

//______________________________________________________________________________

void app_aftertouch_event(u8 index, u8 value)
{
  if(mode == MODE_NOTE) {
    // example - send poly aftertouch to MIDI ports
    hal_send_midi(USBMIDI, POLYAFTERTOUCH | note_channel, index, value); 
    // example - set LED to white, brightness in proportion to pressure
    hal_plot_led(TYPEPAD, index, value/2, value/2, value/2);
  }
}
	
//______________________________________________________________________________

void app_cable_event(u8 type, u8 value)
{
    // example - light the Setup LED to indicate cable connections
	if (type == MIDI_IN_CABLE)
	{
		hal_plot_led(TYPESETUP, 0, 0, value, 0); // green
	}
	else if (type == MIDI_OUT_CABLE)
	{
		hal_plot_led(TYPESETUP, 0, value, 0, 0); // red
	}
}

//______________________________________________________________________________


void app_timer_event()
{
	// example - send MIDI clock at 125bpm
#define TICK_MS 20
	
	static u8 ms = TICK_MS;
	
	if (++ms >= TICK_MS)
	{
		ms = 0;
		
		// send a clock pulse up the USB
		hal_send_midi(USBSTANDALONE, MIDITIMINGCLOCK, 0, 0);
	}
}

//______________________________________________________________________________

void app_init()
{
  hal_plot_led(TYPEPAD, BUTTON_DEVICE, MAXLED, MAXLED, MAXLED);
  
  u8 m = 0b10011101;
  for (int j=1; j < 9; ++j)
    {
      u8 v = rotr8(m,j-1);
      for (int i=1; i < 9; ++i)
	{
	  u8 r = 0;
	  u8 g = 0;
	  u8 b = 0;
	  if((v>>(i-1)) & 1) {
	    r = MAXLED;
	    g = MAXLED;
	    b = MAXLED;
	  }
	  hal_plot_led(TYPEPAD, j * 10 + i, r, b, g);
	}
    }
}

//________________________________________________________
//
// helper for bitwise left and right rotation of u8 values
//________________________________________________________

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


//________________________________________________________
//
// totally naive and almost unusable rand -
// still enough to pseudo-randomly fill the grid...
// with some input from here:
// http://www.electro-tech-online.com/threads/ultra-fast-pseudorandom-number-generator-for-8-bit.124249/
//________________________________________________________

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
