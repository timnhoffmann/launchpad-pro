#ifndef TIMING_H
#define TIMING_H

#include "app_defs.h"
#include "general.h"


/**
 * called every timer tick.
 * This function keeps track of milliSec ticks vs beats per minute and all that...
 */
void msTick();

/**
 * gets called every MIDI clock tick.
 * Currently plays and updates the ca and step sequencers.
 */
void midiTick();

/**
 * sets internal or MIDI sync
 * @param intSync - !=0: internal sync, 0: MIDI sync
 */
void setInternalSync(u8 intSync);

/**
 * set the number of ms per sequencerTick
 */
void setMSperDIV();

/**
 * set 10*BPM as integer for internal tempo
 * @param bpmTt - 10 times the beats per minute so set.
 */
void setBPMtt(u32 bpmTt);

/**
 * midi start callback
 */
void midiStart();
/**
 * midi continue callback
 */
void midiContinue();

/**
 * midi stop callback
 */
void midiStop();

#endif
