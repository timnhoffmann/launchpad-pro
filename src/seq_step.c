#include "seq_step.h"
#include "app.h"
#include "general.h"
#include "timing.h"
// midi channel per instrument
u8 seq_step_channel[8] = {2,2,2,2,2,2,2,2};

// active / mute state for instruments:
u8 seq_step_active = 0b11111111;

// holds the currently playing note
s8 seq_step_playing[8];
s8 seq_step_playing_dur[8];

// pitches for 8 instruments on 16 steps
u8 seq_step_notes[8][16];

// velocities for 8 instruments on 16 steps
u8 seq_step_velocities[8][16];

// durations for 8 instruments on 16 steps
u8 seq_step_dur[8][16];

u16 seq_step_mute_state[8] = {0,0,0,0,0,0,0,0};

// maybe later note lengths...
// u8 seq_step_lengths[8][16];


u8 seq_step_note_root = 36;

u8 currentStep = 0;
u8 currentInstrument = 0;
u8 seq_step_play_pointer = 0;

// recording
u8 recording = 0;

s8 seq_step_rec_pos = 0;
s8 seq_step_rec_dur =-1;

void seq_step_step_display_init() {
  // the Pad-no for the current step
  u8 mc = currentStep;
  mc = mc%8 +1 + (mc/8 +1)*10;
  // light the step pads:
  for(int i = 11; i<19; i++) {
    u8 v = i==mc?MAXLED:0;
    u8 w = (seq_step_mute_state[currentInstrument]&1<<(i-11))?MAXLED:5;
    u8 u = (i-11)==seq_step_play_pointer?MAXLED:0;
    hal_plot_led(TYPEPAD, i, v, u, w);
  }
  for(int i = 21; i<29; i++) {
    u8 v = i==mc?MAXLED:0;
    u8 w = (seq_step_mute_state[currentInstrument]&1<<(i-13))?MAXLED:5;
    u8 u = (i-13)==seq_step_play_pointer?MAXLED:0;
    hal_plot_led(TYPEPAD, i, v, u, w);
  }
}

u8 chooseStep(u8 index) {
  u8 i = index%10;
  u8 j = index/10;
  return (i-1) + 8*(j-1);
}

void seq_step_note_display_init() {
  // light the note choice pads:
  for(int j = 0; j<6;j++)
    for(int i = 0; i<8; i++) {
      u8 note = seq_step_note_root + i + j*5;
      u8 r = 0;
      u8 g = 0;
      u8 b = 0;
      if(note%12 == 0) {r=32; g = 32;}
      if(note==MIDDLE_C) {r=MAXLED,g=b=0;}// for reference and orientation
      if(note==seq_step_notes[currentInstrument][currentStep]) {r=g=b=MAXLED;}// the current note
      hal_plot_led(TYPEPAD, (j+3)*10+i+1, r, g, b);
    }
}

void seq_step_length_display_init() {
  // light the pads to indicate step length
  for(int i = 0; i<8; i++)
    for(int j = 0; j<8; j++) {
      u8 g = (4- (i%4))*12 - 8;
      u8 b = (seq_step_dur[currentInstrument][currentStep] == (i + 1 + 8*j)) ?MAXLED : 0;
      hal_plot_led(TYPEPAD, (j+1)*10+i+1, 0, g, b);
    }
}

void seq_step_length_set(u8 index) {
  u8 i = index%10;
  u8 j = index/10 - 1;
  seq_step_dur[currentInstrument][currentStep] = i + 8*j;
  seq_step_length_display_init();
}

void seq_step_inst_select_init() {
  for(int i = 0; i<8; i++) {
    u8 b =(seq_step_active & (1<<i))?MAXLED:0;
    u8 g = (i== currentInstrument)?MAXLED:0;
    hal_plot_led(TYPEPAD, (i+1)*10+9, 0, g, b);
  }
}

void seq_step_mode_init() {
  seq_step_step_display_init();
  seq_step_note_display_init();
  seq_step_inst_select_init();
  u8 v = seq_step_running? MAXLED:0;
  hal_plot_led(TYPEPAD, BUTTON_CIRCLE, v,v,v);
  all_modes_init();
}

void muteOrSelect(u8 index) {
    u8 j = index/10 -1;
  if(getButtonStateIndex(BUTTON_TRACK_SEL) ){
    currentInstrument = j;
    seq_step_mode_init();
  } else {
    seq_step_active ^= 1<<j;
    if(!(seq_step_active & (1<<j)))
      seq_step_noteOff(j);
    seq_step_inst_select_init();
  }
}

void seq_step_delete_one(u8 inst) {
  for(int i = 0; i<16; i++) {
    seq_step_notes[inst][i] = MIDDLE_C;
    seq_step_velocities[inst][i] = 96;
    seq_step_dur[inst][i] = 2;
    seq_step_mute_state[inst] = 0;
  }
  if(mode == MODE_SEQ_STEP) {
    seq_step_step_display_init();
    seq_step_note_display_init();
  }
}

void seq_step_delete_all() {
  for(int i = 0; i<8; i++) {
    seq_step_delete_one(i);
  }
}

void seq_step_typepad(u8 index, u8 value) {
  u8 i = index%10;
  u8 j = index/10;
  if((i>0) && (i<9) && (j>0) && (j<3)) {//step
    if(value) {
      if(getButtonStateIndex(BUTTON_SOLO)) {
	seq_step_length_set(index);
      } else {
	currentStep = chooseStep(index);
	if(getButtonStateIndex(BUTTON_MUTE)) {
	  seq_step_mute_state[currentInstrument] ^= (1<<currentStep);
	} 
	seq_step_step_display_init();
	seq_step_note_display_init();
      }
    }
  } else if((i>0) && (i<9) && (j>2) && (j<9)) { // note
    if(recording) {
      if(value) {
	u8 pos = (timingRoundOff()/2 + seq_step_play_pointer +15)%16;
	// stop any playing note
	seq_step_noteOff(currentInstrument);
	seq_step_rec_pos = pos;
	seq_step_rec_dur = 0;
	u8 note = seq_step_note_root + (i-1) + (j-3)*5;
	seq_step_notes[currentInstrument][pos] = note;
	seq_step_velocities[currentInstrument][pos] = value;
	seq_step_mute_state[currentInstrument] |= (1<<pos);
	seq_step_playing[currentInstrument] = note;
	hal_send_midi(midiport, NOTEON | seq_step_channel[currentInstrument], seq_step_playing[currentInstrument], value);
	seq_step_playing_dur[currentInstrument] = 64;
	seq_step_note_display_init();		
      } else {
	seq_step_dur[currentInstrument][seq_step_rec_pos] = seq_step_rec_dur;
	hal_send_midi(midiport, NOTEON | seq_step_channel[currentInstrument], seq_step_playing[currentInstrument], 0);

      }
    } else if(value) {
      if(getButtonStateIndex(BUTTON_SOLO)) {
	seq_step_length_set(index);
      } else {
      	u8 note = seq_step_note_root + (i-1) + (j-3)*5;
	seq_step_notes[currentInstrument][currentStep] = note;
	seq_step_velocities[currentInstrument][currentStep] = value;
	seq_step_mute_state[currentInstrument] |= (1<<currentStep);
	seq_step_dur[currentInstrument][currentStep] = 3; // TODO: default length?
	seq_step_note_display_init();
      }
    }
  } else if(index%10 == 9) { //seq_step buttons for inst select
    if(value) {
      muteOrSelect(index);
    }
  } else {
    switch (index) {
      case BUTTON_RECORD_ARM: // reset to 1st step
      {
	if(value) {
	  seq_step_toggle_recording();
	  u8 v = recording? MAXLED:0;
	  hal_plot_led(TYPEPAD, index, v,0,0);
	} 
      }
      break;
      case BUTTON_MUTE:
      case BUTTON_TRACK_SEL:
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, MAXLED, MAXLED, MAXLED);
	} else
	  hal_plot_led(TYPEPAD, index, 0, 0, 0);
      }
      break;
    case BUTTON_CIRCLE: // play / pause
      {
	if(value)
	  seq_step_toggle_running();
      }
      break;
    case BUTTON_DOUBLE: // reset to 1st step
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, MAXLED,MAXLED,MAXLED);
	  seq_step_reset();
	  seq_step_step_display_init();
	} else
	  hal_plot_led(TYPEPAD, index, 0, 0, 0);
      }
      break;
    case BUTTON_DELETE: // delete current instrument's notes
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, MAXLED,MAXLED,MAXLED);
	  seq_step_delete_one(currentInstrument);
	} else
	  hal_plot_led(TYPEPAD, index, 0, 0, 0);
      }
      break;
    case BUTTON_SOLO: // step length
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, MAXLED,MAXLED,MAXLED);
	  seq_step_length_display_init();
	} else {
	  hal_plot_led(TYPEPAD, index, 0,0,0);
	  seq_step_step_display_init();
	  seq_step_note_display_init();
	}
      }
      break;
    case BUTTON_UP:
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, MAXLED,MAXLED,MAXLED);
	  if(seq_step_note_root>=5){
	    seq_step_note_root-=5;
	    seq_step_note_display_init();
	  }
	} else
	  hal_plot_led(TYPEPAD, index, 0,0,0);
	
      }
      break;
    case BUTTON_DOWN:
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, MAXLED,MAXLED,MAXLED);
	  if(seq_step_note_root <=150) {
	    seq_step_note_root+=5;
	    seq_step_note_display_init();
	  }
	} else
	  hal_plot_led(TYPEPAD, index, 0,0,0);
      }
      break;
    case BUTTON_LEFT:
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, MAXLED,MAXLED,MAXLED);
	  if(seq_step_note_root <155) {
	    seq_step_note_root+=1;
	    seq_step_note_display_init();
	  }
	} else
	  hal_plot_led(TYPEPAD, index, 0,0,0);
      }
      break;
    case BUTTON_RIGHT:
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, MAXLED,MAXLED,MAXLED);
	  if(seq_step_note_root>0) {
	    seq_step_note_root-=1;
	    seq_step_note_display_init();
	  }
	} else
	  hal_plot_led(TYPEPAD, index, 0,0,0);
      }
      break;
    }
  }
  all_modes_typepad(index,value);
}

//
// setup mode
//

void seq_step_setup_init() {
  chooseMIDI_init(seq_step_channel[currentInstrument]);
  seq_step_inst_select_init();
  u8 v = seq_step_running? MAXLED:0;
  hal_plot_led(TYPEPAD, BUTTON_CIRCLE, v,v,v);
}

void seq_step_setup_typepad(u8 index, u8 value) {
  //  u8 i = index%10;
  //  u8 j = index/10;
if(isChooseMIDI(index)) {
  //seq_step_noteOff(i);
  seq_step_channel[currentInstrument] = chooseMIDI(index);
  chooseMIDI_init(seq_step_channel[currentInstrument]);
  } else if(index%10 == 9) { //seq_step buttons for inst select
    if(value) {
      muteOrSelect(index);
    }
  } else if(index == BUTTON_CIRCLE) { // play / pause
  if(value)
     seq_step_toggle_running();
  } 
 all_modes_typepad(index,value);
}

void seq_step_toggle_running() {
  seq_step_running = !seq_step_running;
  u8 v = MAXLED;
  if(!seq_step_running) {
    v = 0;
    seq_step_noteOff_all();
  }
  hal_plot_led(TYPEPAD, BUTTON_CIRCLE, v,v,v);
}

void seq_step_noteOff_all() {
  for(int i = 0; i<8; i++)
    seq_step_noteOff(i);
}

void seq_step_noteOff(u8 i) {
  
  if(seq_step_playing[i] >=0) {
    hal_send_midi(midiport, NOTEON | seq_step_channel[i], seq_step_playing[i], 0);
    seq_step_playing[i] = -1;
    seq_step_playing_dur[i] = 0;
    //        if(mode == MODE_SEQ_STEP)
    //          hal_plot_led(TYPEPAD, (i+1)*10+9, SEQ_CA_SCENE_BR,SEQ_CA_SCENE_BR,SEQ_CA_SCENE_BR);
  }
  
}

void seq_step_play(u8 frac) {
  if( seq_step_running) {
    for(int j = 0; j<8;j++) { // each instrument
      if(seq_step_active &(1<<j)) { // inst active
	if(seq_step_playing_dur[j] > 0) { // old note playing?
	  if(seq_step_playing_dur[j] ==1) // switch off old notes?
	    hal_send_midi(midiport, NOTEON | seq_step_channel[j], seq_step_playing[j], 0);
	  seq_step_playing_dur[j]--;
	}
      }
    }
    if(frac == 0) {
      for(int j = 0; j<8;j++) { // each instrument
	if(seq_step_active &(1<<j)) { // inst active
	  if(seq_step_mute_state[j] &  1<<seq_step_play_pointer) { // play note if not muted
	    // switch off old note if ther is one still going
	    // TODO move this behind the new one to make legato...
	    if(seq_step_playing_dur[j] >0) 
	      hal_send_midi(midiport, NOTEON | seq_step_channel[j], seq_step_playing[j], 0);
	    // set new note and play it:
	    seq_step_playing[j] = seq_step_notes[j][seq_step_play_pointer];
	    seq_step_playing_dur[j] = seq_step_dur[j][seq_step_play_pointer];
	    hal_send_midi(midiport, NOTEON | seq_step_channel[j], seq_step_playing[j], seq_step_velocities[j][seq_step_play_pointer]);
	  }
	}
      }
      if(mode == MODE_SEQ_STEP && ! getButtonStateIndex(BUTTON_SOLO)) // show playing step:
	seq_step_step_display_init();
      
      seq_step_play_pointer = (seq_step_play_pointer +1)%16;
    }
    if(recording) {
      seq_step_rec_dur++;
    }
  }
}

void seq_step_reset() {
  seq_step_play_pointer = 0;
}

void seq_step_toggle_recording() {
  recording = ! recording;
}
