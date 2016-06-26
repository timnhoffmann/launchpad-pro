#ifndef SEQ_CA_H
#define SEQ_CA_H

#include "app.h"
#include "general.h"

u8 seq_ca_getBit(u8 i, u8 j);
void seq_ca_updateLED(u8 i, u8 j);
void seq_ca_updateLEDs();

/**
 * clear the SEQ_CA
 */
void seq_ca_clear();

/**
 * fill the SEQ_CA randomly
 */
void seq_ca_random();

/**
 * inits the seq_ca mode after a mode change
 */
void seq_ca_mode_init();

/**
 * update step for the cellular automaton. This is the heart of the ca.
 * Depending on the parity of the time parameter the even or odd bits/leds/cells are updated.
 * @param t - the time. only t%2 matters though.
 */
void seq_ca_update(int t);

/**
 * Progress time for the ca sequencer.
 * This sends out the notes and updates the ca state.
 * @param t - the time. only t%2 matters though.
 */
void seq_ca_updateTime();

/** 
 * togglerunning state for seq_ca
 */
void seq_ca_toggle_running();

/**
 * note off for all seq_ca instruments
 */
void seq_ca_noteOff_all();

/**
 * note of for the given seq_ca instrument
 * @param i - the instrument to silence.
 */
void seq_ca_noteOff(u8 i);

/**
 * generates the pitch for the i-th instrument
 * @param i - the number of the instrument. 0<= i<8.
 */
u8 makeNote(int i);

/**
 * Play the current state as midi notes.
 */
void seq_ca_play();

/**
 * the TYPEPAD response for the SEQ_CA mode
 * @param index - index of the button
 * @param vsalue - velocity of the button
 */
void seq_ca_typepad(u8 index, u8 value);

/**
 * the setup layout for the SEQ_CA
 */
void seq_ca_setup_init();

/**
 * the TYPEPAD response for the seq_ca setup mode
 * @param index - index of the button
 * @param value - velocity of the button
 */
void seq_ca_setup_typepad(u8 index, u8 value);

#endif
