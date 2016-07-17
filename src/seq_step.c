#include "seq_step.h"
#include "app.h"
#include "general.h"
#include "timing.h"
#include "colors.h"
// midi channel per instrument
u8 seq_step_channel[8] = {3,3,3,3,3,3,0,0};

// active / mute state for instruments:
u8 seq_step_active = 0b11111111;

// holds the currently playing note
s8 seq_step_playing[8];
s16 seq_step_playing_dur[8];

// pitches for 8 instruments on 8 seqs on 16 steps
u8 seq_step_notes[8][8][16];
u8 seq_step_root_notes[8] = {36,37,38,39,40,42,60,60};

// velocities for 8 instruments on 8 seqs on 16 steps
u8 seq_step_velocities[8][8][16];

// durations for 8 instruments on 8 seqs on 16 steps
u16 seq_step_dur[8][8][16];

u16 seq_step_mute_state[8][8] = {{0,0,0,0,0,0,0,0},
				 {0,0,0,0,0,0,0,0},
				 {0,0,0,0,0,0,0,0},
				 {0,0,0,0,0,0,0,0},
				 {0,0,0,0,0,0,0,0},
				 {0,0,0,0,0,0,0,0},
				 {0,0,0,0,0,0,0,0},
				 {0,0,0,0,0,0,0,0}
};


// sequence lengths...
u8 seq_step_seq_length[8] = {16,16,16,16,16,16,16,16};

u8 seq_step_note_root = 36;

u8 currentStep[8] = {0,0,0,0,0,0,0,0};

/** 
 * the selected sequence all editing happens in
 */
u8 currentSeq = 0;

// mute mode...
u8 mute_mode = 0;

// track select mode...
u8 track_sel_mode = 0;

/**
 * the sequence to start playing from per instrument
 */
u8 seq_step_start_seq[8] = {0,0,0,0,0,0,0,0};

u8 currentInstrument = 0;
u8 seq_step_play_pointer[8] = {0,0,0,0,0,0,0,0};
u8 seq_step_play_seq[8] = {0,0,0,0,0,0,0,0};

// recording
u8 recording = 0;

s8 seq_step_rec_pos = 0;
s8 seq_step_rec_seq = 0;
s8 seq_step_rec_dur =-1;
s8 seq_step_rec_note =-1;
s8 seq_step_rec_index =-1;

void seq_step_step_display_init() {
  // the Pad-no for the current step
  u8 mc = currentStep[currentInstrument];
  mc = mc%8 +1 + (mc/8 +1)*10;

  u8 sq = (seq_step_play_pointer[currentInstrument]/16 + seq_step_start_seq[currentInstrument])%8;
  u8 st = seq_step_play_pointer[currentInstrument]%16;
  // light the step pads:
  for(int i = 11; i<19; i++) {
    /*
    u8 v = i==mc?MAXLED:0;
    u8 w = (seq_step_mute_state[currentInstrument]&1<<(i-11))?MAXLED:5;
    u8 u = (i-11)==seq_step_play_pointer?MAXLED:0;
    hal_plot_led(TYPEPAD, i, v, u, w);
    */
    if((i-11)==st && sq == currentSeq) hal_plot_led(TYPEPAD, i, STEP_PLAYING);
    else if(i== mc) hal_plot_led(TYPEPAD, i, STEP_SELECTED);
    else if(seq_step_mute_state[currentInstrument][currentSeq]&1<<(i-11)) hal_plot_led(TYPEPAD, i, STEP_ACTIVE);
    else hal_plot_led(TYPEPAD, i, STEP_MUTED);
  }
  for(int i = 21; i<29; i++) {
    /*
    u8 v = i==mc?MAXLED:0;
    u8 w = (seq_step_mute_state[currentInstrument]&1<<(i-13))?MAXLED:5;
    u8 u = (i-13)==seq_step_play_pointer?MAXLED:0;
    hal_plot_led(TYPEPAD, i, v, u, w);
    */
    if((i-13)==st && sq == currentSeq) hal_plot_led(TYPEPAD, i, STEP_PLAYING);
    else if(i== mc) hal_plot_led(TYPEPAD, i, STEP_SELECTED);
    else if(seq_step_mute_state[currentInstrument][currentSeq]&1<<(i-13)) hal_plot_led(TYPEPAD, i, STEP_ACTIVE);
    else hal_plot_led(TYPEPAD, i, STEP_MUTED);
  }
  for(int i = 31; i<39; i++) {
    if(i-31 == sq)
      hal_plot_led(TYPEPAD, i, SEQ_PLAYING);
    else if(i-31 == seq_step_start_seq[currentInstrument])
      hal_plot_led(TYPEPAD, i, SEQ_START);
    else if(i-31 == currentSeq)
      hal_plot_led(TYPEPAD, i, SEQ_SELECTED);
    else
      hal_plot_led(TYPEPAD, i, SEQ_MUTED);
  }
}

u8 chooseStep(u8 index) {
  u8 i = index%10;
  u8 j = index/10;
  return (i-1) + 8*(j-1);
}

u8 chooseSeq(u8 index) {
  u8 i = index%10;
  //u8 j = index/10;
  return (i-1);
}

void seq_step_note_display_init() {
  // light the note choice pads:
  for(int j = 0; j<5;j++)
    for(int i = 0; i<8; i++) {
      u8 note = seq_step_note_root + i + j*5;
      /*
      u8 r = 0;
      u8 g = 0;
      u8 b = 0;
      if(note%12 == 0) {r=32; g = 32;}
      if(note==MIDDLE_C) {r=MAXLED,g=b=0;}// for reference and orientation
      if(note==seq_step_notes[currentInstrument][currentStep]) {r=g=b=MAXLED;}// the current note
      hal_plot_led(TYPEPAD, (j+3)*10+i+1, r, g, b);
      */
      u8* c = black;

      if(note%12 == 0)
	c = note_c;
      if(note==MIDDLE_C) // for reference and orientation
	c = note_middle_c;
      if(mode == MODE_SEQ_STEP) {
	if(note==seq_step_notes[currentInstrument][currentSeq][currentStep[currentInstrument]]) // the current note
	  c = note_playing;
      } else if(mode == MODE_SEQ_STEP_SETUP) {
	if(note==seq_step_root_notes[currentInstrument]) // the current root note
	  c = note_playing;
      }
      hal_plot_led(TYPEPAD, (j+4)*10+i+1, COLOR(c));
    }
}

void seq_step_length_display_init() {
  // light the pads to indicate step length
  for(int i = 0; i<8; i++)
    for(int j = 0; j<8; j++) {
      u8 g = (4- (i%4))*12 - 8;
      u8 b = (seq_step_dur[currentInstrument][currentSeq][currentStep[currentInstrument]] == (i + 1 + 8*j)) ?MAXLED : 0;
      hal_plot_led(TYPEPAD, (j+1)*10+i+1, 0, g, b);
    }
}

void seq_step_length_set(u8 index) {
  u8 i = index%10;
  u8 j = index/10 - 1;
  seq_step_dur[currentInstrument][currentSeq][currentStep[currentInstrument]] = i + 8*j;
  seq_step_length_display_init();
}

void seq_step_velocity_display_init() {
  // light the pads to indicate step velocity
  for(int i = 0; i<8; i++)
    for(int j = 0; j<8; j++) {
      if((seq_step_velocities[currentInstrument][currentSeq][currentStep[currentInstrument]]/2 + 1) == (i + 1 + 8*j))
	hal_plot_led(TYPEPAD, (j+1)*10+i+1, WHITE);
      else {
	u8 v = (i + 8*j);
	hal_plot_led(TYPEPAD, (j+1)*10+i+1, v, v/2, 0);
      }
    }
}

void seq_step_velocity_set(u8 index) {
  u8 i = index%10;
  u8 j = index/10 - 1;
  seq_step_velocities[currentInstrument][currentSeq][currentStep[currentInstrument]] = (i + 8*j)*2-1;
  seq_step_velocity_display_init();
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
  if(mute_mode)
    hal_plot_led(TYPEPAD, BUTTON_MUTE, BUTTON_ON);
  else
    hal_plot_led(TYPEPAD, BUTTON_MUTE, BUTTON_OFF);
  if(track_sel_mode)
    hal_plot_led(TYPEPAD, BUTTON_TRACK_SEL, BUTTON_ON);
  else
    hal_plot_led(TYPEPAD, BUTTON_TRACK_SEL, BUTTON_OFF);
  all_modes_init();
}

void muteOrSelect(u8 index) {
  u8 j = index/10 -1;
  //  if(getButtonStateIndex(BUTTON_TRACK_SEL) ){
  if(track_sel_mode){
    currentInstrument = j;
    if(mode == MODE_SEQ_STEP)
      seq_step_mode_init();
    else seq_step_setup_init();
  } else {
    seq_step_active ^= 1<<j;
    if(!(seq_step_active & (1<<j)))
      seq_step_noteOff(j);
    seq_step_inst_select_init();
  }
}

void seq_step_duplicate_seq(u8 i) {
  u8 target = (i+1)%8;
  for(int k = 0; k<16;k++) {
    seq_step_notes[currentInstrument][target][k] = seq_step_notes[currentInstrument][i][k];
    seq_step_velocities[currentInstrument][target][k] = seq_step_velocities[currentInstrument][i][k];
    seq_step_dur[currentInstrument][target][k] = seq_step_dur[currentInstrument][i][k];
  }
  seq_step_mute_state[currentInstrument][target] = seq_step_mute_state[currentInstrument][i];
  currentSeq = target;
  if(mode == MODE_SEQ_STEP) {
    seq_step_step_display_init();
    seq_step_note_display_init();
  }
}

void seq_step_delete_one(u8 inst, u8 seq) {
  for(int i = 0; i<16; i++) {
    seq_step_notes[inst][seq][i] = seq_step_root_notes[inst]; // MIDDLE_C;
    seq_step_velocities[inst][seq][i] = 96;
    seq_step_dur[inst][seq][i] = 2;
    seq_step_mute_state[inst][seq] = 0;
  }
  if(mode == MODE_SEQ_STEP) {
    seq_step_step_display_init();
    seq_step_note_display_init();
  }
}

void seq_step_delete_all(u8 inst) {
  for(int k = 0; k<8; k++) {
    seq_step_delete_one(inst, k);
  }
}

void seq_step_quantize() {
  for(int i = 0; i< 16; i++) {
    seq_step_velocities[currentInstrument][currentSeq][i] = seq_step_velocities[currentInstrument][currentSeq][currentStep[currentInstrument]];
    seq_step_dur[currentInstrument][currentSeq][i] = seq_step_dur[currentInstrument][currentSeq][currentStep[currentInstrument]];
  }
}

void seq_step_typepad(u8 index, u8 value) {
  u8 i = index%10;
  u8 j = index/10;
  if((i>0) && (i<9) && (j>0) && (j<3)) {// step
    if(value) {
      if(getButtonStateIndex(BUTTON_SOLO)) {
	seq_step_length_set(index);
      } else if(getButtonStateIndex(BUTTON_VOLUME)) {
	seq_step_velocity_set(index);
      } else if(getButtonStateIndex(BUTTON_STOP_CLIP)) {
	u8 sq = currentSeq -seq_step_start_seq[currentInstrument];
	sq = sq>=0?sq:0;
	seq_step_seq_length[currentInstrument] = 16*(sq) + 8*(j-1) + (i);
      } else if(mute_mode) {
	seq_step_mute_state[currentInstrument][currentSeq] ^= (1<<(8*(j-1) + (i-1)));
	seq_step_step_display_init();
      } else {
	currentStep[currentInstrument] = chooseStep(index);
	if(getButtonStateIndex(BUTTON_MUTE)) {
	  seq_step_mute_state[currentInstrument][currentSeq] ^= (1<<currentStep[currentInstrument]);
	} 
	seq_step_step_display_init();
	seq_step_note_display_init();
      }
    }
  } else if((i>0) && (i<9) && (j==3) ) {// seq
    if(value) {
      u8 s = chooseSeq(index);
      if(getButtonStateIndex(BUTTON_SHIFT)) {
	for(int k = 0; k<8; k++)
	  seq_step_start_seq[k] = s;
      } else if(getButtonStateIndex(BUTTON_SENDS)) {
	seq_step_start_seq[currentInstrument] = s;
      } else 
	currentSeq = s;
      seq_step_step_display_init();
      seq_step_note_display_init();
    }
  } else if((i>0) && (i<9) && (j>3) && (j<9)) { // note
    if(recording) {
      if( (value == 0 && seq_step_rec_index == index) || (seq_step_rec_note >= 0) ) {
	seq_step_dur[currentInstrument][seq_step_rec_seq][seq_step_rec_pos] = seq_step_rec_dur;
	seq_step_mute_state[currentInstrument][seq_step_rec_seq] |= (1<<seq_step_rec_pos);
	hal_send_midi(midiport, NOTEON | seq_step_channel[currentInstrument], seq_step_playing[currentInstrument], 0);
	seq_step_playing_dur[currentInstrument] = 0;
	seq_step_rec_note = -1;
      }
      if(value) {
	u8 point = (0*(timingRoundOff()>0?1:0) + (u16)seq_step_play_pointer[currentInstrument] + seq_step_seq_length[currentInstrument] )%seq_step_seq_length[currentInstrument];
	u8 st = point%16;
	u8 sq = (point/16 + seq_step_start_seq[j])%8;
	// stop any playing note
	seq_step_noteOff(currentInstrument);
	seq_step_rec_pos = st;
	seq_step_rec_seq = sq;
	seq_step_rec_dur = 1;
	seq_step_rec_note = seq_step_note_root + (i-1) + (j-4)*5;
	seq_step_rec_index = index;
	
	seq_step_notes[currentInstrument][sq][st] = seq_step_rec_note;
	seq_step_velocities[currentInstrument][sq][st] = value;
	seq_step_mute_state[currentInstrument][sq] &= ~(1<<st);
	seq_step_playing[currentInstrument] = seq_step_rec_note;
	hal_send_midi(midiport, NOTEON | seq_step_channel[currentInstrument], seq_step_playing[currentInstrument], value);
	seq_step_playing_dur[currentInstrument] = 64;
	seq_step_note_display_init();		
      }
      /* else { */
      /* 	seq_step_dur[currentInstrument][seq_step_rec_seq][seq_step_rec_pos] = seq_step_rec_dur; */
      /* 	seq_step_mute_state[currentInstrument][seq_step_rec_seq] |= (1<<seq_step_rec_pos); */
      /* 	hal_send_midi(midiport, NOTEON | seq_step_channel[currentInstrument], seq_step_playing[currentInstrument], 0); */

      /* } */
    } else if(value) {
      if(getButtonStateIndex(BUTTON_SOLO)) {
	seq_step_length_set(index);
      } else if(getButtonStateIndex(BUTTON_VOLUME)) {
	seq_step_velocity_set(index);
      } else {
      	u8 note = seq_step_note_root + (i-1) + (j-4)*5;
	seq_step_notes[currentInstrument][currentSeq][currentStep[currentInstrument]] = note;
	seq_step_velocities[currentInstrument][currentSeq][currentStep[currentInstrument]] = value;
	seq_step_mute_state[currentInstrument][currentSeq] |= (1<<currentStep[currentInstrument]);
	seq_step_dur[currentInstrument][currentSeq][currentStep[currentInstrument]] = 3; // TODO: default length?
	seq_step_note_display_init();
      }
    }
  } else if(index%10 == 9) { //seq_step buttons for inst select
    if(value) {
      muteOrSelect(index);
    }
  } else {
    switch (index) {
    case BUTTON_RECORD_ARM: // arm for recording
      {
	if(value) {
	  seq_step_toggle_recording();
	  u8 v = recording? MAXLED:0;
	  hal_plot_led(TYPEPAD, index, v,0,0);
	} 
      }
      break;
    case BUTTON_MUTE:
      {
	if(value) {
	  seq_step_toggle_mute();
	  if(mute_mode)
	    hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  else
	    hal_plot_led(TYPEPAD, index, BUTTON_OFF);
	}
      }
      break;
    case BUTTON_TRACK_SEL:
	{
	  if(value) {
	  seq_step_toggle_track_select();
	  if(track_sel_mode)
	    hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  else
	    hal_plot_led(TYPEPAD, index, BUTTON_OFF);
	}
	  //	if(value) {
	  //	  hal_plot_led(TYPEPAD, index, MAXLED, MAXLED, MAXLED);
	  //	} else
	  //	  hal_plot_led(TYPEPAD, index, 0, 0, 0);
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
      case BUTTON_DUPLICATE: // reset to 1st step
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  seq_step_duplicate_seq(currentSeq);
	} else
	  hal_plot_led(TYPEPAD, index, BUTTON_OFF);
      }
      break;
    case BUTTON_QUANTIZE: // reset to 1st step
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, MAXLED,MAXLED,MAXLED);
	  seq_step_quantize();
	  seq_step_step_display_init();
	} else
	  hal_plot_led(TYPEPAD, index, 0, 0, 0);
      }
      break;

    case BUTTON_DELETE: // delete current instrument's notes
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  if(getButtonStateIndex(BUTTON_SHIFT)) 
	    seq_step_delete_all(currentInstrument);
	  else
	    seq_step_delete_one(currentInstrument, currentSeq);
	} else
	  hal_plot_led(TYPEPAD, index, BUTTON_OFF);
      }
      break;
    case BUTTON_SOLO: // step length
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  seq_step_length_display_init();
	} else {
	  hal_plot_led(TYPEPAD, index, BUTTON_OFF);
	  seq_step_step_display_init();
	  seq_step_note_display_init();
	}
      }
      break;
      case BUTTON_VOLUME: // step velocity
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  seq_step_velocity_display_init();
	} else {
	  hal_plot_led(TYPEPAD, index, BUTTON_OFF);
	  seq_step_step_display_init();
	  seq_step_note_display_init();
	}
      }
      break;
    case BUTTON_UP:
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  if(seq_step_note_root>=5){
	    seq_step_note_root-=5;
	    seq_step_note_display_init();
	  }
	} else
	  hal_plot_led(TYPEPAD, index, BUTTON_OFF);
	
      }
      break;
    case BUTTON_DOWN:
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  if(seq_step_note_root <=150) {
	    seq_step_note_root+=5;
	    seq_step_note_display_init();
	  }
	} else
	  hal_plot_led(TYPEPAD, index, BUTTON_OFF);
      }
      break;
    case BUTTON_LEFT:
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  if(seq_step_note_root <155) {
	    seq_step_note_root+=1;
	    seq_step_note_display_init();
	  }
	} else
	  hal_plot_led(TYPEPAD, index, BUTTON_OFF);
      }
      break;
    case BUTTON_RIGHT:
      {
	if(value) {
	  hal_plot_led(TYPEPAD, index, BUTTON_ON);
	  if(seq_step_note_root>0) {
	    seq_step_note_root-=1;
	    seq_step_note_display_init();
	  }
	} else
	  hal_plot_led(TYPEPAD, index, BUTTON_OFF);
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
  seq_step_note_display_init();
  u8 v = seq_step_running? MAXLED:0;
  hal_plot_led(TYPEPAD, BUTTON_CIRCLE, v,v,v);
}

void seq_step_setup_typepad(u8 index, u8 value) {
      u8 i = index%10;
      u8 j = index/10;

  if(isChooseMIDI(index)) {
    //seq_step_noteOff(i);
    seq_step_channel[currentInstrument] = chooseMIDI(index);
    chooseMIDI_init(seq_step_channel[currentInstrument]);
  } else if((i>0) && (i<9) && (j>2) && (j<9)) { // note
    if(value) {
      seq_step_root_notes[currentInstrument] = seq_step_note_root + (i-1) + (j-3)*5;
      seq_step_note_display_init();
    }
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
	  u8 sq = (seq_step_play_pointer[j]/16 + seq_step_start_seq[j])%8;
	  u8 st = seq_step_play_pointer[j]%16;
	  if(seq_step_mute_state[j][sq] &  1<<st) { // play note if not muted
	    // switch off old note if ther is one still going
	    // TODO move this behind the new one to make legato...
	    if(seq_step_playing_dur[j] >0) 
	      hal_send_midi(midiport, NOTEON | seq_step_channel[j], seq_step_playing[j], 0);
	    // set new note and play it:
	    seq_step_playing[j] = seq_step_notes[j][sq][st];
	    seq_step_playing_dur[j] = seq_step_dur[j][sq][st];
	    hal_send_midi(midiport, NOTEON | seq_step_channel[j], seq_step_playing[j], seq_step_velocities[j][sq][st]);
	  }
	}
      }
      if(mode == MODE_SEQ_STEP && ! (getButtonStateIndex(BUTTON_SOLO)|| getButtonStateIndex(BUTTON_VOLUME))) // show playing step:
	seq_step_step_display_init();
      
      for(int i = 0; i<8; i++)
	seq_step_play_pointer[i] = (seq_step_play_pointer[i] +1)%seq_step_seq_length[i];
    }
    if(recording) {
      seq_step_rec_dur++;
    }
  }
}

void seq_step_reset() {
  for(int i = 0; i<8; i++)
    seq_step_play_pointer[i] = 0;
}

void seq_step_toggle_recording() {
  recording = ! recording;
}

void seq_step_toggle_mute() {
  mute_mode = ! mute_mode;
}

void seq_step_toggle_track_select() {
  track_sel_mode = ! track_sel_mode;
}
