#ifndef LAUNCHPAD_APP_H
#define LAUNCHPAD_APP_H

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

// ____________________________________________________________________________
//
// Don't modify this file!  This declares the binary interface to the library,
// so modifying it will probably break things.
// ____________________________________________________________________________
//
#include "app_defs.h"

/******************************************************************************
 Button indexing is as follows - numbers in brackets do not correspond to real
buttons, but can be harmessly sent in hal_set_led.
 
 (90)91 92 93 94 95 96 97 98 (99)
 .......
 20  21 22 23 24 25 26 27 28  29
 10  11 12 13 14 15 16 17 18  19
 (0)  1  2  3  4  5  6  7  8  (9)
 
 *****************************************************************************/

// ____________________________________________________________________________
//
// Interface to the hardware (implemented in launchpad_pro.a library)
// ____________________________________________________________________________

/**
 * Set an LED's RGB value.  This function is safe to call from any
 * of the app functions below, at any time.
 *
 * @param type - TYPEPAD to address any pad or button, TYPESETUP to address the Setup button
 * @param index - The index of the button.  The buttons are indexed from the bottom left as detailed above.
 * @param red - red colour value, in [0, MAXLED]
 * @param green - green colour value, in [0, MAXLED]
 * @param blue - blue colour value, in [0, MAXLED]
 */
void hal_plot_led(u8 type, u8 index, u8 red, u8 green, u8 blue);

/**
 * Send a MIDI message to either USB port or to the DIN output.
 * 
 * @param port - which port to send the message to - can be USBSTANDALONE, USBMIDI or DINMIDI.
 * @param status - MIDI status byte
 * @param data1 - first MIDI data byte
 * @param data2 - second MIDI data byte
 *
 * There is little error checking in this code - if you send invalid MIDI, the results are undefined!
 */
void hal_send_midi(u8 port, u8 status, u8 data1, u8 data2);

/**
 * Send system exclusive to USB or DIN.  Messages must be correctly formatted
 * (F0 ... F7) and must not exceed 320 bytes.
 *
 * @param port - which port to send the message to - can be USBSTANDALONE, USBMIDI or DINMIDI.
 * @param data - pointer to array containing sysex data. Can be on the stack.
 * @param length - must not exceed 320 bytes, behaviour undefined if it does.
 */
void hal_send_sysex(u8 port, const u8* data, u16 length);

// ____________________________________________________________________________
//
// Callbacks from the hardware (implemented in your app.c)
// ____________________________________________________________________________

/**
 * Called on startup, this is a good place to do any initialisation.
 */
void app_init();

/**
 *  1kHz (1ms) timer.  You can set LEDs and send MIDI out from this function,
 *  but you will get LED tearing as there is (currently) no double buffering.
 *
 *  You will get some jitter.
 */
void app_timer_event();

/**
 * Called when a MIDI message is received from USB or DIN.
 *
 * @param port - the port the message was received from - USBSTANDALONE, USBMIDI or DINMIDI.
 * @param status - MIDI status byte
 * @param data1 - first MIDI data byte
 * @param data2 - second MIDI data byte
 */
void app_midi_event(u8 port, u8 status, u8 d1, u8 d2);

/**
 * As above, but for system exclusive messages.  Low level hardware buffering sets
 * a maximum message size of 320 bytes, messages larger than this will not work.
 *
 * @param port - the port the message was received from - USBSTANDALONE, USBMIDI or DINMIDI.
 * @param data - pointer to array containing sysex data.  Only valid in the scope of this callback.
 * @param length - the amount of data received.
 */
void app_sysex_event(u8 port, u8 * data, u16 count);

/**
 * Called when a MIDI DIN breakout cable is connected or disconnected.  Note that
 * you can still write MIDI events to the DIN ports even if no cable is connected.
 *
 * @param type - which cable was connected/disconnected - MIDI_IN_CABLE or MIDI_OUT_CABLE.
 * @param value - 0 = disconnected, nonzero = connected.
 */
void app_cable_event(u8 type, u8 value);

/**
 * Called when the user presses or releases any button or pad on the control surface.
 *
 * @param type - TYPEPAD for normal pads or buttons, TYPESETUP for the Setup button
 * @param index - The index of the button, as detailed at the start of this file.
 * @param value - 0 for release, nonzero for press.
 */
void app_surface_event(u8 type, u8 index, u8 value);

/**
 * Called when the low level pad scanning reports an aftertouch (pad pressure) event.
 * A pad press event will always come first.  Note that the factory firmware sets a
 * threshold to prevent excessive aftertouch after the initial hit, at the expense of
 * dynamic range.  This firmware does not - the full range of the pad is transmitted,
 * with the side effect that aftertouch onset is more rapid.
 *
 * @param index - The index of the pad, as detailed at the start of this file.
 * @param value - the aftertouch value in [0, 127]
 */
void app_aftertouch_event(u8 index, u8 value);

// ____________________________________________________________________________
//
// utility methods
// ____________________________________________________________________________

/**
 * cyclic left rotation
 * @param value - the value to rotate
 * @param count - the number of bits to rotatet by
 */
u8 rotl8 (u8 value, unsigned int count);

/**
 * cyclic right rotation
 * @param value - the value to rotate
 * @param count - the number of bits to rotatet by
 */
u8 rotr8 (u8 value, unsigned int count);

/**
* a totally naive and almost unusable rand function
*/
u8 simple_rand();
#endif
