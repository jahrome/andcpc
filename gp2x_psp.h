# ifndef __SDL_JOY_H__
# define __SDL_JOY_H__

# ifdef __cplusplus
extern "C" {
# endif

#include "global.h"

typedef struct gp2xCtrlData {

  int    Buttons;
  u32    TimeStamp;

} gp2xCtrlData;

typedef struct gp2xCtrlData SceCtrlData;

# define GP2X_CTRL_UP         0x00001
# define GP2X_CTRL_RIGHT      0x00002
# define GP2X_CTRL_DOWN       0x00004
# define GP2X_CTRL_LEFT       0x00008
# define GP2X_CTRL_TRIANGLE   0x00010
# define GP2X_CTRL_CIRCLE     0x00020
# define GP2X_CTRL_CROSS      0x00040
# define GP2X_CTRL_SQUARE     0x00080
# define GP2X_CTRL_SELECT     0x00100
# define GP2X_CTRL_START      0x00200
# define GP2X_CTRL_LTRIGGER   0x00400
# define GP2X_CTRL_RTRIGGER   0x00800
# define GP2X_CTRL_FIRE       0x01000
# define GP2X_CTRL_VOLUP      0x02000
# define GP2X_CTRL_VOLDOWN    0x04000
# define GP2X_CTRL_MASK       0x07fff

# define GP2X_CTRL_A          GP2X_CTRL_SQUARE
# define GP2X_CTRL_B          GP2X_CTRL_CIRCLE
# define GP2X_CTRL_Y          GP2X_CTRL_TRIANGLE
# define GP2X_CTRL_X          GP2X_CTRL_CROSS

# define PSP_CTRL_UP         GP2X_CTRL_UP        
# define PSP_CTRL_RIGHT      GP2X_CTRL_RIGHT     
# define PSP_CTRL_DOWN       GP2X_CTRL_DOWN      
# define PSP_CTRL_LEFT       GP2X_CTRL_LEFT      
# define PSP_CTRL_TRIANGLE   GP2X_CTRL_TRIANGLE  
# define PSP_CTRL_CIRCLE     GP2X_CTRL_CIRCLE    
# define PSP_CTRL_CROSS      GP2X_CTRL_CROSS     
# define PSP_CTRL_SQUARE     GP2X_CTRL_SQUARE    
# define PSP_CTRL_SELECT     GP2X_CTRL_SELECT    
# define PSP_CTRL_START      GP2X_CTRL_START     
# define PSP_CTRL_LTRIGGER   GP2X_CTRL_LTRIGGER  
# define PSP_CTRL_RTRIGGER   GP2X_CTRL_RTRIGGER  
# define PSP_CTRL_FIRE       GP2X_CTRL_FIRE      
# define PSP_CTRL_VOLUP      GP2X_CTRL_VOLUP     
# define PSP_CTRL_VOLDOWN    GP2X_CTRL_VOLDOWN   
# define PSP_CTRL_MASK       GP2X_CTRL_MASK      

#if defined(WIZ_MODE) || defined(GP2X_MODE)

//gp2x buttons codes

#define GP2X_UP              (0)
#define GP2X_DOWN            (4)
#define GP2X_LEFT            (2)
#define GP2X_RIGHT           (6)
#define GP2X_UPLEFT          (1)
#define GP2X_UPRIGHT         (7)
#define GP2X_DOWNLEFT        (3)
#define GP2X_DOWNRIGHT       (5)

#define GP2X_A               (12)
#define GP2X_B               (13)
#define GP2X_X               (14)
#define GP2X_Y               (15)
#define GP2X_L               (10)
#define GP2X_R               (11)
#define GP2X_FIRE            (18)
#define GP2X_START           (9)        
#define GP2X_SELECT          (8)
#define GP2X_VOLUP           (16)
#define GP2X_VOLDOWN         (17)

#else

//some keys of the keyboard to emulate gp2x
#define GP2X_UPLEFT         83 //
#define GP2X_UP             82 // UP
#define GP2X_UPRIGHT        84 //

#define GP2X_LEFT           80 // LEFT
#define GP2X_RIGHT          79 // RIGHT

#define GP2X_DOWNLEFT       85 //
#define GP2X_DOWN           81 // DOWN
#define GP2X_DOWNRIGHT      86 //

# if 0
#define GP2X_A              38 //
#define GP2X_B              56 //
#define GP2X_X              53 //
#define GP2X_Y              29 //
# else
#define GP2X_A              14 // K
#define GP2X_B              15 // L
#define GP2X_X              19 // P
#define GP2X_Y              18 // O
# endif
#define GP2X_L              13 // J
#define GP2X_R              12 // I
#define GP2X_FIRE           54 //
#define GP2X_START          224 // MENU
#define GP2X_SELECT         77  // ACTION
#define GP2X_VOLUP         75     //SDLK_KP_PLUS
#define GP2X_VOLDOWN       78    //SDLK_KP_MINUS

#endif

#define GP2X_NOEVENT -1

#define DELAY_KEY_FIRST 250
#define DELAY_KEY_REPEAT 80
    
  extern int  gp2xCtrlReadBufferPositive(gp2xCtrlData* c, int v);
  extern int  gp2xCtrlPeekBufferPositive(gp2xCtrlData* c, int v);
  extern void gp2xPowerSetClockFrequency(int freq);

# define scePowerSetClockFrequency(A,B,C) gp2xPowerSetClockFrequency((A))

  extern int  gp2xGetSoundVolume();
  extern void gp2xDecreaseVolume();
  extern void gp2xIncreaseVolume();

# if defined(GP2X_MODE)
  extern int  gp2xInsmodMMUhack();
  extern int  gp2xRmmodMMUhack();
# endif

# ifdef __cplusplus
}
# endif

# endif
