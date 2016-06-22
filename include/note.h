#ifndef NOTE_H
#define NOTE_H

#include "app.h"
#include "general.h"

void note_mode_init();

/**
 * the TYPEPAD response for the NOTE mode
 * @param index - index of the button
 * @param vsalue - velocity of the button
 */
void note_typepad(u8 index, u8 value);

void note_setup_init();

/**
 * the TYPEPAD response for the NOTE setup mode
 * @param index - index of the button
 * @param vsalue - velocity of the button
 */
void note_setup_typepad(u8 index, u8 value);

/**
* aftertouch response in note_mode
* @param index - pad pressed
* @param value - the pressure value
*/
void note_aftertouch(u8 index, u8 value);

#endif
