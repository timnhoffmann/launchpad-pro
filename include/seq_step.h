#ifndef SEQ_STEP_H
#define SEQ_STEP_H
#include "app_defs.h"

/**
 * init display for the step select rows
 */
void seq_step_step_display_init();

/**
 * displays the length of the current step
 */
void seq_step_length_display_init();

/**
 * set the length of current step form button
 * @param index - index of the pressed pad
 */
void seq_step_length_set(u8 index);

/**
 * displays the velocity of the current step
 */
void seq_step_velocity_display_init();

/**
 * set the velocity of current step form button
 * @param index - index of the pressed pad
 */
void seq_step_velocity_set(u8 index);


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
 * @param frac - the sub-step count - should be 0<= frac<4
 */
void seq_step_play(u8 frac);

/**
 * sets playing from start of pattern
 */
void seq_step_reset();

/**
 * delete all notes for the given instrument
 * @param inst - the instrument
 */
void seq_step_delete_one(u8 inst);

/**
 * delete all notes for all instruments
 */
void seq_step_delete_all();

/**
 * toggle recording state
 */
void seq_step_toggle_recording();

/**
 * set velocity and steplength to the current step's value 
 * for the whole sequence.
 */
void seq_step_quantize();

#endif
