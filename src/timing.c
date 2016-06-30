#include "timing.h"
#include "seq_ca.h"
#include "seq_step.h"
#include "global_setup.h"

// division of a beat into seqTicks
#define DIVIDER 48
// number of seqTicks per 16th
#define SIXTEENTH 12
// number of seqTicks per MIDI clock tick
#define CLOCKTEENTH 2

// is midi running?
u8 midiRunning = 0;

// counts the ms ticks
s32 msTc = 0;

// the number of ms per seqTick
u32 msPerSeqTick = 0;
// fractional part of that.
s32 msPerSeqTickFrac = 0;

// stores the current /running fractional part for better timing in internal sync mode
s32 frac = 0;

// seqTicks per minute times ten
u32 stpmtt = 0;

// the numer of seqTicks 
s8 seqTc =0;

// stores the drift against midi clock in external sync mode
s32 drift = 0;


// incremets per msTick to estimate bpm from MIDI clock:
u32  msPerMidiTc = 0;

void msTick() {
  if((seq_step_running || seq_ca_running) && (internalSync || midiRunning)) {
    
    if(msTc >= 0) {
      msTc -= msPerSeqTick;

      if(internalSync) {
	frac += msPerSeqTickFrac;
	if(frac>stpmtt/2) {
	  frac -= stpmtt;
	  msTc--;
	}
       }
      
      if(seqTc >= 0) {
	seq_ca_play();
	seq_step_play();
	seqTc = -(SIXTEENTH);
      }
      seqTc++;
    }

    // increment all the counters
    msTc++;
    if(midiRunning)
      msPerMidiTc++;
  }
}

u32 measurements[24];
u8  mc = 0;

void midiTick() {
    
  if(!internalSync) {
    if(mode == MODE_GLOBAL_SETUP)
      global_setup_init();
    
    drift += CLOCKTEENTH*msPerSeqTick - msPerMidiTc;
    msTc += drift;
    drift -= drift;
    
    measurements[mc] = msPerMidiTc; 
    msPerMidiTc = 0;
    mc = (mc+1)%24;
    
    u8 c = 0;
    u32 tmp = 0;
    for(int i = 0; i<24; i++) {
      if(measurements[i] != 0) {
	tmp+=measurements[i];
	c++;
      }
    }

    // set the new estimate of ms per seqTicks:
    msPerSeqTick = (tmp)/(CLOCKTEENTH*c);
    
    // estimate (and set) the bpm we are recieving
    bpmtt=(600*1000*CLOCKTEENTH*c)/ (DIVIDER*tmp);
    
  }
}

void setInternalSync(u8 intSync) {
  if(internalSync != intSync) {
    internalSync = intSync;
    msTc = 0;
    frac = 0;

    //setMSperDIV();
  }
}

void setMSperDIV() {
    stpmtt = bpmtt*DIVIDER;
    // milliseconds per sequencer tick:
    msPerSeqTick = ((600*1000)/stpmtt);
    msPerSeqTickFrac = ((600*1000)%stpmtt);
    frac = 0;
}


void setBPMtt(u32 bpmTt) {
  bpmtt = bpmTt;
  
  if(internalSync)
    setMSperDIV();
}

void midiStart() {
  if(!internalSync) {
    seq_step_reset();
    msPerMidiTc = 0;
    msTc = 0;
    drift = 0;
    seqTc = 0;
  }
  midiRunning = 1;
}

void midiContinue() {
  midiRunning = 1;
}

void midiStop() {
  midiRunning = 0;
  //  if(!internalSync) {
  //for(int i = 0; i<24;i++) measurements[i] = 0;
  //  }
}
