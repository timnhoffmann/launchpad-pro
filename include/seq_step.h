#ifndef SEQ_STEP_H
#define SEQ_STEP_H
#include "app_defs.h"

/**
 * init display for the step select rows
 */
void seq_step_step_display_init();

/**
 * step for index
 * @param - index the index to calculate the step from
 */
u8 chooseStep(u8 index);

/**
 * init the display of the not selection
 */
void seq_step_note_display_init();

/**
 * init the seq_step buttons for instrument selection
 */
void seq_step_inst_select_init();

/**
 * inits the seq_step mode after a mode change
 */
void seq_step_mode_init();

/**
 * the TYPEPAD response for the seq_step mode
 * @param index - index of the button
 * @param vsalue - velocity of the button
 */
void seq_step_typepad(u8 index, u8 value);

/**
 * the setup layout for the SEQ_CA
 */
void seq_step_setup_init();

/**
 * the TYPEPAD response for the seq_ca setup mode
 * @param index - index of the button
 * @param value - velocity of the button
 */
void seq_step_setup_typepad(u8 index, u8 value);

/**
 * toggle the running state
 */
void seq_step_toggle_running();

/**
 * turns off any playing note for all instruments
 */
void seq_step_noteOff_all();

/**
 * turns off any playing note for the given instrument
 * @param i - the instruments number
 */
void seq_step_noteOff(u8 i);

/**
 * the seq_step sequencer play routine. called for every step.
 */
void seq_step_play();


#endif
