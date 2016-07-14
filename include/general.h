
#ifndef GENERAL_H
#define GENERAL_H

#include "app_defs.h"
#include <limits.h>  // for def of CHAR_BIT

#ifdef GENERAL
   #define EXT
#else
   #define EXT extern
#endif

//__________________________________________
//
// general consts and vars
//__________________________________________
//

// the midiport to use:
EXT u8 midiport;
// the mode - can be MODE_SEQ_CA, MODE_SEQ_STEP, MODE_NOTES, MODE_USER
EXT u8 mode;


EXT u8  internalSync;

EXT u8  seq_step_running;

EXT u8  seq_ca_running;

EXT u32 time;
//EXT u8  running;
// tempo estimate in ms/beat
EXT u32 tempo;
// tempo in 10*BPM
EXT u32 bpmtt;
// store the pressed state of the buttons:
#ifndef GENERAL
extern u16 buttonState[];
#endif

// ____________________________________________________________________________
//
// buttons
// ____________________________________________________________________________

#define	BUTTON_RECORD_ARM	01
#define	BUTTON_TRACK_SEL	02
#define	BUTTON_MUTE		03
#define	BUTTON_SOLO		04
#define	BUTTON_VOLUME		05
#define	BUTTON_PAN		06
#define	BUTTON_SENDS		07
#define	BUTTON_STOP_CLIP	08
//
#define	BUTTON_CIRCLE		10
#define	BUTTON_DOUBLE		20
#define	BUTTON_DUPLICATE       	30
#define	BUTTON_QUANTIZE		40
#define	BUTTON_DELETE		50
#define	BUTTON_UNDO		60
#define	BUTTON_CLICK		70
#define	BUTTON_SHIFT		80
//
#define	BUTTON_SCENE_1		19
#define	BUTTON_SCENE_2		29
#define	BUTTON_SCENE_3		39
#define	BUTTON_SCENE_4		49
#define	BUTTON_SCENE_5		59
#define	BUTTON_SCENE_6		69
#define	BUTTON_SCENE_7		79
#define	BUTTON_SCENE_8		89
//
#define	BUTTON_UP		91
#define	BUTTON_DOWN		92
#define	BUTTON_LEFT		93
#define	BUTTON_RIGHT		94
#define	BUTTON_SESSION		95
#define	BUTTON_NOTE		96
#define	BUTTON_DEVICE		97
#define	BUTTON_USER		98
//
// not corresponding to an LED (or a buttonevent for that)
#define	BUTTON_SETUP		90

// ____________________________________________________________________________
//
// some constants
// ____________________________________________________________________________

#define MODE_SEQ_CA                   BUTTON_DEVICE
#define MODE_NOTE                     BUTTON_NOTE
#define MODE_SEQ_STEP                 BUTTON_SESSION
#define MODE_USER                     BUTTON_USER
#define MODE_SEQ_CA_SETUP             (BUTTON_DEVICE|128)
#define MODE_NOTE_SETUP               (BUTTON_NOTE|128)
#define MODE_SEQ_STEP_SETUP           (BUTTON_SESSION|128)
#define MODE_USER_SETUP               (BUTTON_USER|128)
#define MODE_GLOBAL_SETUP             128

// your choice...
#define MIDDLE_C                60
//#define MIDDLE_C                72


//______________________________________________________________________________
//
// utility methods
//______________________________________________________________________________

/**
 * returns whether the button at (i,j) is currently pressed.
 * the state gets updated AFTER each press is evaluated (so for a given event it gives the state of that button before the event).
 * @param i -- the column
 * @param j -- the row
 */
u8 getButtonState(u8 i, u8 j);

/**
 * returns wether the button at index is currently pressed.
 * the state gets updated AFTER each press is evaluated (so for a given event it gives the state of that button before the event).
 * @param index -- the button index
 */
u8 getButtonStateIndex(u8 index);

/** 
 * switches to the given mode
 * @param m - the mode to switch to.
 */
void setMode(u8 m);

/**
 * initializes the two bottom rows of pads to serve as MIDI channel selector
 * @param currentChannel - the current MIDI channel (gets highlighted)
 */
void chooseMIDI_init(u8 currentChannel);

/**
 * returns whether the given index is in the bottom two rows of pads
 * @param index - the index to check
 */
u8 isChooseMIDI(u8 index);

/**
 * returns the picked midichannel for the given index
 * @param the index to check
 */
u8 chooseMIDI(u8 index);

void all_modes_init();

void all_modes_typepad(u8 index, u8 value);

/**
 * bitwise rotate a u8 value to the left
 * @param value - the value to be rotated
 * @param count - the number of bits to rotate by
 */
u8 rotl8 (u8 value, unsigned int count);

/**
 * bitwise rotate a u8 value to the right
 * @param value - the value to be rotated
 * @param count - the number of bits to rotate by
 */
u8 rotr8 (u8 value, unsigned int count);

/** 
 * totally naive and almost unusable rand -
 * still enough to pseudo-randomly fill the grid...
 * with some input from here:
 * http://www.electro-tech-online.com/threads/ultra-fast-pseudorandom-number-generator-for-8-bit.124249/
 */
u8 simple_rand();


#endif
