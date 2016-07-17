#ifndef COLORS_H
#define COLORS_H

#include "app_defs.h"

#ifdef COLORS
   #define CEXT
#else
   #define CEXT extern
#endif
//
// MAXLED is 63 so r, g, and b values are 6 bits each...
//

#define COLOR(X) (X)[0], (X)[1], (X)[2]
typedef u8 color[3];

#define C_MIX(X,Y,L) ((64-(L))*(X)[0] + (L)*(Y)[0])/64, ((64-(L))*(X)[1] + (L)*(Y)[1])/64, ((64-(L))*(X)[2] + (L)*(Y)[2])/64, 

#define RED      63, 0, 0
#define GREEN    0, 63, 0
#define BLUE     0, 0, 63

#define WHITE    63, 63, 63
#define BLACK    0, 0, 0

#define YELLOW   63, 63, 0
#define CYAN     0, 63, 63
#define MAGENTA   63, 0, 63

#define CA_PAD   63, 63, 63


#define BUTTON_ON 63, 63, 63
#define BUTTON_OFF 0, 0, 0

#define INST_ACTIVE 16, 16, 16
#define INST_INACTIVE 0, 0, 0
#define INST_PLAYING 63, 63, 63
#define INST_CURRENT 63, 63, 63
#define INST_SELECT 0, 0, 16

#define NOTE_C 48, 16, 0
#define NOTE_MIDDLE_C 0, 32, 63
#define NOTE_PLAYING 16, 63, 0

// note bits for seq_ca_setup upper row:
#define NOTEBIT_OFF 0, 0, 5
#define NOTEBIT_ON 0, 0, 63

// seq step step display
#define STEP_PLAYING 48, 16, 0
#define STEP_SELECTED 16, 63, 0
#define STEP_ACTIVE 0, 32, 63
#define STEP_MUTED 0, 0, 16

#define SEQ_PLAYING 48, 16, 0
#define SEQ_SELECTED 16, 63, 0
#define SEQ_START 8, 8, 63
#define SEQ_MUTED 16, 0, 0




#ifndef COLORS
extern color red;
extern color green;
extern color blue;

extern color white;
extern color black;

extern color yellow;
extern color cyan;
extern color magenta;

extern color note_c;
extern color note_middle_c;
extern color note_playing;

#endif

#endif
