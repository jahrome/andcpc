#ifndef PSPCTRL_H
#define PSPCTRL_H

#include "psptypes.h"

typedef struct SceCtrlData {

  int    Buttons;
  u32    TimeStamp;
  int    Lx;
  int    Ly;

} SceCtrlData;


# define PSP_CTRL_UP         0x00001
# define PSP_CTRL_RIGHT      0x00002
# define PSP_CTRL_DOWN       0x00004
# define PSP_CTRL_LEFT       0x00008
# define PSP_CTRL_TRIANGLE   0x00010
# define PSP_CTRL_CIRCLE     0x00020
# define PSP_CTRL_CROSS      0x00040
# define PSP_CTRL_SQUARE     0x00080
# define PSP_CTRL_SELECT     0x00100
# define PSP_CTRL_START      0x00200
# define PSP_CTRL_LTRIGGER   0x00400
# define PSP_CTRL_RTRIGGER   0x00800
# define PSP_CTRL_FIRE       0x01000
# define PSP_CTRL_HOME       0x02000
# define PSP_CTRL_HOLD       0x04000
# define PSP_CTRL_MASK       0x07fff

#define PSP_CTRL_MODE_ANALOG 0

//some keys of the keyboard to emulate sce

#define PSP_UPLEFT         79 //SDLK_KP7
#define PSP_UP             80 //SDLK_KP8
#define PSP_UPRIGHT        81 //SDLK_KP9

#define PSP_LEFT           83 //SDLK_KP4
#define PSP_RIGHT          85 //SDLK_KP6

#define PSP_DOWNLEFT       87 //SDLK_KP1
#define PSP_DOWN           88 //SDLK_KP2
#define PSP_DOWNRIGHT      89 //SDLK_KP3

#define PSP_A              38 //SDLK_a
#define PSP_B              56 //SDLK_b
#define PSP_X              53 //SDLK_x
#define PSP_Y              29 //SDLK_y
#define PSP_L              46 //SDLK_l
#define PSP_R              27 //SDLK_r
#define PSP_FIRE           65 //SDLK_SPACE
#define PSP_START          36 //SDLK_RETURN
#define PSP_SELECT         39  //SDLK_s
#define PSP_VOLUP         86     //SDLK_KP_PLUS
#define PSP_VOLDOWN       82    //SDLK_KP_MINUS

#define PSP_NOEVENT -1

#define DELAY_KEY_FIRST 250
#define DELAY_KEY_REPEAT 80
    
  extern int  sceCtrlReadBufferPositive(SceCtrlData* c, int v);
  extern int  sceCtrlPeekBufferPositive(SceCtrlData* c, int v);


#endif
