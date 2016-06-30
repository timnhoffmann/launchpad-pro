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
#include "general.h"
#include "note.h"
#include "seq_ca.h"
#include "seq_step.h"
#include "global_setup.h"
#include "timing.h"

//______________________________________________________________________________
//
// This is where the button and pad presses are handled:
//______________________________________________________________________________

void app_surface_event(u8 type, u8 index, u8 value) {
  switch (type)
    {
    case  TYPEPAD:
      {
	if(index < BUTTON_SESSION) {
	  switch (mode) {
	  case MODE_SEQ_STEP:
	    {
	      seq_step_typepad(index,value);
	    }
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
	    case MODE_SEQ_STEP_SETUP:
	    {
	      seq_step_setup_typepad(index,value);
	    }
	    break;
	  case MODE_NOTE_SETUP:
	    {
	      note_setup_typepad(index,value);
	    }
	    break;
	  case MODE_SEQ_CA_SETUP:
	    {
	      seq_ca_setup_typepad(index,value);
	    }
	    break;
	  case MODE_GLOBAL_SETUP:
	    {
	      global_setup_typepad(index, value);
	    }
	    break;
	  }
	} else {
	  if(value>0) { // index >= BUTTON_SESSION:
	    if((mode & 127)!= index) {
	      hal_plot_led(TYPEPAD, (mode & 127), 0, 0, 0);
	      hal_plot_led(TYPEPAD, index, MAXLED, MAXLED, MAXLED);
	      setMode(index|(mode&128));
	    }
	  }
	}
	// update the buttonState
	int i = index%10;
	int j = index/10;
	buttonState[j] ^= (buttonState[j] ^ ((value?1:0)<<i) ) & (1<<i); 
      }
      break;
      
    case TYPESETUP: // switch to setup mode
      {
	if(value) {
	  if(getButtonStateIndex(BUTTON_SHIFT)) {
	    setMode(MODE_GLOBAL_SETUP);
	  } else {
	    mode ^=128;
	    value = (mode&128)>>1;
	    hal_plot_led(TYPESETUP, 0, value, value, value);
	    if(mode == 0) mode = MODE_SEQ_CA;
	    setMode(mode);
	  }
	}
      }
      break;
    }
}

//______________________________________________________________________________
//
// we keep the MIDI-USB conversion functionality for now and handle MIDI clock
//______________________________________________________________________________
//

void app_midi_event(u8 port, u8 status, u8 d1, u8 d2)
{
	// example - MIDI interface functionality for USB "MIDI" port -> DIN port
	if (port == USBMIDI)
	{
		hal_send_midi(DINMIDI, status, d1, d2);
	}

	// example -MIDI interface functionality for DIN -> USB "MIDI" port port
	if (port == DINMIDI)
	{
		hal_send_midi(USBMIDI, status, d1, d2);
	}

	switch(status) {
	case MIDISTART:
	  {
	    midiStart();
	  }
	case MIDICONTINUE:
	  {
	    midiContinue();
	  }
	  break;
	case MIDISTOP:
	  {
	    midiStop();
	  }
	  break;
	case MIDITIMINGCLOCK: // handle MIDI clock ticks
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
	// no use for sysex sofar.
}

//______________________________________________________________________________
//
// in note mode send aftertouch messages
//______________________________________________________________________________

void app_aftertouch_event(u8 index, u8 value)
{
  if(mode == MODE_NOTE) {
    note_aftertouch(index, value);
    // example - send poly aftertouch to MIDI ports
    //hal_send_midi(midiport, POLYAFTERTOUCH | note_channel, index, value); 
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
//
// this will have to handle the internal clock - once it is implemented :)
//______________________________________________________________________________
//
void app_timer_event()
{
  msTick();
  
	// example - send MIDI clock at 125bpm
#define TICK_MS 20
	
	static u8 ms = TICK_MS;
	static u8 mt = 0;
	if (++ms >= TICK_MS)
	{

	  //hal_plot_led(TYPESETUP, 0, mt/2, 0, 0);
	  ms = 0;
	  mt++;
	  if(mt >= 96)
	    mt = 0;	  
		// send a clock pulse up the USB
		//hal_send_midi(USBSTANDALONE, MIDITIMINGCLOCK, 0, 0);
	}
}

//______________________________________________________________________________
//
// nothing to initialize really at the moment...
//______________________________________________________________________________
//
void app_init()
{
  // initialize the global variables

  
  // the midiport to use:
  midiport = USBMIDI;
  // the mode - can be MODE_SEQ_CA, MODE_SEQ_STEP, MODE_NOTES, MODE_USER
  mode = MODE_SEQ_CA;
  seq_ca_running = 0;
  seq_step_running = 0;
  
  time = 0;

  // tempo estimate in ms/beat
  tempo = 500; // 120 bpm

  setInternalSync(1);
  setBPMtt(1250); // 125 bpm


  
  //hal_plot_led(TYPEPAD, BUTTON_DEVICE, MAXLED, MAXLED, MAXLED);
  
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

  setMode(MODE_SEQ_CA);
}

