#ifndef GLOBAL_SETUP_H
#define GLOBAL_SETUP_H
#include "app_defs.h"
#include "general.h"

void chooseBPM_init();

u8 isChooseBPM(u8 index);

u16 chooseBPM(u8 index);

void global_setup_init();

void global_setup_typepad(u8 index, u8 value);

#endif
