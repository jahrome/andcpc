#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_events.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "global.h"
#include "gp2x_psp.h"
#include "gp2x_cpu.h"

#ifdef ANDROID_DEBUG
#include <android/log.h>
#endif // ANDROID_DEBUG

/* For internal use only */
# define GP2X_CTRL_UPRIGHT    0x10000
# define GP2X_CTRL_UPLEFT     0x20000
# define GP2X_CTRL_DOWNRIGHT  0x40000
# define GP2X_CTRL_DOWNLEFT   0x80000


static int    loc_Volume = 50;

static int    loc_LastEventMask    = 0;
static int    loc_CurrEventMask    = 0;
static int    loc_CurrEventButtons = 0;
static u32    loc_LastTimeStamp    = 0;
static u32    loc_CurrTimeStamp    = 0;

static int    loc_VolumeButtons = 0;
static int    loc_VolumePress   = 0;
static u32    loc_LastVolumeTimeStamp = 0;

# define GP2X_MIN_TIME_VOLUME  300000
# define GP2X_MIN_TIME_REPEAT  100000

static inline int
gp2xConvertMaskToButtons(int Mask)
{
  int Buttons = Mask & GP2X_CTRL_MASK;
  if (Mask & GP2X_CTRL_UPLEFT ) Buttons |= GP2X_CTRL_UP|GP2X_CTRL_LEFT;
  if (Mask & GP2X_CTRL_UPRIGHT) Buttons |= GP2X_CTRL_UP|GP2X_CTRL_RIGHT;
  if (Mask & GP2X_CTRL_DOWNLEFT ) Buttons |= GP2X_CTRL_DOWN|GP2X_CTRL_LEFT;
  if (Mask & GP2X_CTRL_DOWNRIGHT) Buttons |= GP2X_CTRL_DOWN|GP2X_CTRL_RIGHT;
  return Buttons;
}

void
gp2xTreatVolume(gp2xCtrlData* c)
{
  if (c->Buttons & GP2X_CTRL_VOLDOWN) {
    /* Already down ? */
    if (! (loc_VolumeButtons & GP2X_CTRL_VOLDOWN)) {
      loc_LastVolumeTimeStamp = loc_CurrTimeStamp;
      loc_VolumeButtons |= GP2X_CTRL_VOLDOWN;
      loc_VolumePress = 1;
      gp2xDecreaseVolume();
    } else 
    if ((((loc_CurrTimeStamp - loc_LastVolumeTimeStamp) > GP2X_MIN_TIME_VOLUME) && (loc_VolumePress == 1)) ||
        (((loc_CurrTimeStamp - loc_LastVolumeTimeStamp) > GP2X_MIN_TIME_REPEAT) && (loc_VolumePress  > 1))) {
      loc_LastVolumeTimeStamp = loc_CurrTimeStamp;
      loc_VolumePress++;
      gp2xDecreaseVolume();
    }

  } else
  if (c->Buttons & GP2X_CTRL_VOLUP) {
    /* Already down ? */
    if (! (loc_VolumeButtons & GP2X_CTRL_VOLUP)) {
      loc_LastVolumeTimeStamp = loc_CurrTimeStamp;
      loc_VolumeButtons |= GP2X_CTRL_VOLUP;
      loc_VolumePress = 1;
      gp2xIncreaseVolume();
    } else 
    if ((((loc_CurrTimeStamp - loc_LastVolumeTimeStamp) > GP2X_MIN_TIME_VOLUME) && (loc_VolumePress == 1)) ||
        (((loc_CurrTimeStamp - loc_LastVolumeTimeStamp) > GP2X_MIN_TIME_REPEAT) && (loc_VolumePress  > 1))) {
      loc_LastVolumeTimeStamp = loc_CurrTimeStamp;
      loc_VolumePress++;
      gp2xIncreaseVolume();
    }

  } else {
    loc_VolumeButtons = 0;
  }
}

int  
gp2xCtrlPeekBufferPositive(gp2xCtrlData* c, int v)
{
  SDL_Event SDLEvent;    

  int Event         = 0;
  int ButtonPress   = 0;
  int ButtonRelease = 0;
  int Mask = 0;

  memset(c, 0x0, sizeof(gp2xCtrlData));
  loc_CurrTimeStamp = SDL_GetTicks() * 1000;

  if (SDL_PollEvent(&SDLEvent)) {
#if defined(GP2X_MODE) || defined(WIZ_MODE)
    Event = SDLEvent.jbutton.button;
    if (SDLEvent.type==SDL_JOYBUTTONDOWN) ButtonPress = 1;
    else
    if (SDLEvent.type==SDL_JOYBUTTONUP) ButtonRelease = 1;
#else
    Event=((SDL_KeyboardEvent*)(&SDLEvent))->keysym.scancode;
#ifdef ANDROID_DEBUG
    __android_log_print(ANDROID_LOG_ERROR,"Tag","Message %d", Event);
#endif // ANDROID_DEBUG
    if (SDLEvent.type==SDL_KEYDOWN) {
      ButtonPress = 1;
#ifdef ANDROID_DEBUG
    __android_log_print(ANDROID_LOG_ERROR,"Tag","down %d", Event);
#endif // ANDROID_DEBUG
    } else
    if (SDLEvent.type==SDL_KEYUP) {
      ButtonRelease = 1;
#ifdef ANDROID_DEBUG
    __android_log_print(ANDROID_LOG_ERROR,"Tag","up %d", Event);
#endif // ANDROID_DEBUG
    }
#endif
    switch (Event) {
      case GP2X_UP        : Mask = GP2X_CTRL_UP;
      break;
      case GP2X_DOWN      : Mask = GP2X_CTRL_DOWN;
      break;
      case GP2X_LEFT      : Mask = GP2X_CTRL_LEFT;
      break;
      case GP2X_RIGHT     : Mask = GP2X_CTRL_RIGHT;
      break;
      case GP2X_UPLEFT    : Mask = GP2X_CTRL_UPLEFT;
      break;
      case GP2X_UPRIGHT   : Mask = GP2X_CTRL_UPRIGHT;
      break;
      case GP2X_DOWNLEFT  : Mask = GP2X_CTRL_DOWNLEFT;
      break;
      case GP2X_DOWNRIGHT : Mask = GP2X_CTRL_DOWNRIGHT;
      break;
      case GP2X_A         : Mask = GP2X_CTRL_SQUARE;
      break;
      case GP2X_B         : Mask = GP2X_CTRL_CIRCLE;
      break;
      case GP2X_X         : Mask = GP2X_CTRL_CROSS;
      break;
      case GP2X_Y         : Mask = GP2X_CTRL_TRIANGLE;
      break;
      case GP2X_L         : Mask = GP2X_CTRL_LTRIGGER;
      break;
      case GP2X_R         : Mask = GP2X_CTRL_RTRIGGER;
      break;
      case GP2X_FIRE      : Mask = GP2X_CTRL_FIRE;
      break;
      case GP2X_START     : Mask = GP2X_CTRL_START;
      break;
      case GP2X_SELECT    : Mask = GP2X_CTRL_SELECT;
      break;
      case GP2X_VOLUP     : Mask = GP2X_CTRL_VOLUP;
      break;
      case GP2X_VOLDOWN   : Mask = GP2X_CTRL_VOLDOWN;
      break;
    }
    loc_LastEventMask = loc_CurrEventMask;
    if (ButtonPress) {
      loc_CurrEventMask |= Mask;
    } else 
    if (ButtonRelease) {
      loc_CurrEventMask &= ~Mask;
    }
    loc_CurrEventButtons = gp2xConvertMaskToButtons(loc_CurrEventMask);
    c->Buttons   = loc_CurrEventButtons;
    c->TimeStamp = loc_CurrTimeStamp;

    loc_LastTimeStamp = loc_CurrTimeStamp;

  } else {
    c->Buttons   = loc_CurrEventButtons;
    c->TimeStamp = loc_CurrTimeStamp;
  }

  gp2xTreatVolume(c);

  return (c->Buttons != 0);
}

int
gp2xCtrlReadBufferPositive(gp2xCtrlData* c, int v)
{
  while (! gp2xCtrlPeekBufferPositive(c, v));
  return 1;
}


void
gp2xPowerSetClockFrequency(int freq)
{
#if defined(GP2X_MODE) || defined(WIZ_MODE)
  if ((freq >= GP2X_MIN_CLOCK) && (freq <= GP2X_MAX_CLOCK)) {
    cpu_set_clock(freq);
  }
# endif
}

int
gp2xGetSoundVolume()
{
  return loc_Volume;
}

void
gp2xDecreaseVolume()
{
  loc_Volume -= 2;
  if (loc_Volume < 0) loc_Volume = 0;
}

void
gp2xIncreaseVolume()
{
  loc_Volume += 2;
  if (loc_Volume > 100) loc_Volume = 100;
}

# if defined(GP2X_MODE)
int 
gp2xInsmodMMUhack(void)
{
# ifdef GP2X_MMU_HACK
	int mmufd = open("/dev/mmuhack", O_RDWR);
	if(mmufd < 0) {
		system("/sbin/insmod ./drivers/mmuhack.o");
		mmufd = open("/dev/mmuhack", O_RDWR);
	}
	if(mmufd < 0) return 0;
 	close(mmufd); 
# endif
	return 1;
}

int 
gp2xRmmodMMUhack(void)
{
# ifdef GP2X_MMU_HACK
  system("/sbin/rmmod mmuhack");
# endif
  return 0;
}  

# endif
