/*
 *  Copyright (C) 2009 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <SDL.h>

#include "cap32.h"
#include "z80.h"
#include "psp_kbd.h"
#include "kbd.h"
#include "psp_menu.h"
#include "psp_sdl.h"
#include "psp_danzeff.h"
# ifdef USE_PSP_IRKEYB
#include "psp_irkeyb.h"
# endif
# ifdef USE_IRDA_JOY
#include "psp_irda.h"
# endif
#include "psp_battery.h"

# define KBD_MIN_ANALOG_TIME      150000
# define KBD_MIN_START_TIME      1500000
# define KBD_MAX_EVENT_TIME       500000
# define KBD_MIN_PENDING_TIME     300000
# define KBD_MIN_IR_PENDING_TIME  100000
//# define KBD_MIN_DANZEFF_TIME   150000
# define KBD_MIN_DANZEFF_TIME      10000
# define KBD_MIN_COMMAND_TIME     100000
# define KBD_MIN_BATTCHECK_TIME 90000000 
# define KBD_MIN_AUTOFIRE_TIME   1000000

# ifdef USE_IRDA_JOY
  static irda_joy_t   loc_irda_joy_event;
# endif
 static SceCtrlData    loc_button_data;
 static unsigned int   loc_last_event_time = 0;
#ifdef USE_PSP_IRKEYB
 static unsigned int   loc_last_irkbd_event_time = 0;
#endif
 static long           first_time_stamp = -1;
 static long           first_time_batt_stamp = -1;
 static long           first_time_auto_stamp = -1;
 static char           loc_button_press[ KBD_MAX_BUTTONS ]; 
 static char           loc_button_release[ KBD_MAX_BUTTONS ]; 

 unsigned int kbd_button_mask[ KBD_MAX_BUTTONS ] =
 {
   PSP_CTRL_UP         , /*  KBD_UP         */
   PSP_CTRL_RIGHT      , /*  KBD_RIGHT      */
   PSP_CTRL_DOWN       , /*  KBD_DOWN       */
   PSP_CTRL_LEFT       , /*  KBD_LEFT       */
   PSP_CTRL_TRIANGLE   , /*  KBD_TRIANGLE   */
   PSP_CTRL_CIRCLE     , /*  KBD_CIRCLE     */
   PSP_CTRL_CROSS      , /*  KBD_CROSS      */
   PSP_CTRL_SQUARE     , /*  KBD_SQUARE     */
   PSP_CTRL_SELECT     , /*  KBD_SELECT     */
   PSP_CTRL_START      , /*  KBD_START      */
   PSP_CTRL_HOME       , /*  KBD_HOME       */
   PSP_CTRL_HOLD       , /*  KBD_HOLD       */
   PSP_CTRL_LTRIGGER   , /*  KBD_LTRIGGER   */
   PSP_CTRL_RTRIGGER   , /*  KBD_RTRIGGER   */
 };

 char kbd_button_name[ KBD_ALL_BUTTONS ][20] =
 {
   "UP",
   "RIGHT",
   "DOWN",
   "LEFT",
   "TRIANGLE",
   "CIRCLE",
   "CROSS",
   "SQUARE",
   "SELECT",
   "START",
   "HOME",
   "HOLD",
   "LTRIGGER",
   "RTRIGGER",
   "JOY_UP",
   "JOY_RIGHT",
   "JOY_DOWN",
   "JOY_LEFT"
 };

 static char kbd_button_name_L[ KBD_ALL_BUTTONS ][20] =
 {
   "L_UP",
   "L_RIGHT",
   "L_DOWN",
   "L_LEFT",
   "L_TRIANGLE",
   "L_CIRCLE",
   "L_CROSS",
   "L_SQUARE",
   "L_SELECT",
   "L_START",
   "L_HOME",
   "L_HOLD",
   "L_LTRIGGER",
   "L_RTRIGGER",
   "L_JOY_UP",
   "L_JOY_RIGHT",
   "L_JOY_DOWN",
   "L_JOY_LEFT"
 };
 
  static char kbd_button_name_R[ KBD_ALL_BUTTONS ][20] =
 {
   "R_UP",
   "R_RIGHT",
   "R_DOWN",
   "R_LEFT",
   "R_TRIANGLE",
   "R_CIRCLE",
   "R_CROSS",
   "R_SQUARE",
   "R_SELECT",
   "R_START",
   "R_HOME",
   "R_HOLD",
   "R_LTRIGGER",
   "R_RTRIGGER",
   "R_JOY_UP",
   "R_JOY_RIGHT",
   "R_JOY_DOWN",
   "R_JOY_LEFT"
 };
 
  //FMS - Change default keys to joystick
  static int loc_default_mapping[ KBD_ALL_BUTTONS ] = {
        CPC_J0_UP           , /*  KBD_UP         */
        CPC_J0_RIGHT        , /*  KBD_RIGHT      */
        CPC_J0_DOWN         , /*  KBD_DOWN       */
        CPC_J0_LEFT         , /*  KBD_LEFT       */
    CPC_ENTER           , /*  KBD_TRIANGLE   */
    CPC_J0_FIRE2        , /*  KBD_CIRCLE     */
    CPC_J0_FIRE1        , /*  KBD_CROSS      */
    CPC_DEL             , /*  KBD_SQUARE     */
    -1                  , /*  KBD_SELECT     */
    -1                  , /*  KBD_START      */
    -1                  , /*  KBD_HOME       */
    -1                  , /*  KBD_HOLD       */
    KBD_LTRIGGER_MAPPING  , /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING  , /*  KBD_RTRIGGER   */
    CPC_J0_UP           , /*  KBD_JOY_UP     */
    CPC_J0_RIGHT        , /*  KBD_JOY_RIGHT  */
    CPC_J0_DOWN         , /*  KBD_JOY_DOWN   */
    CPC_J0_LEFT           /*  KBD_JOY_LEFT   */
  };

  static int loc_default_mapping_L[ KBD_ALL_BUTTONS ] = {
    CAP32_INCY          , /*  KBD_UP         */
    CAP32_RENDER        , /*  KBD_RIGHT      */
    CAP32_DECY          , /*  KBD_DOWN       */
    CAP32_RENDER        , /*  KBD_LEFT       */
    CAP32_LOAD          , /*  KBD_TRIANGLE   */
    CAP32_JOY           , /*  KBD_CIRCLE     */
    CAP32_SAVE          , /*  KBD_CROSS      */
    CAP32_FPS           , /*  KBD_SQUARE     */
    -1                  , /*  KBD_SELECT     */
    -1                  , /*  KBD_START      */
    -1                  , /*  KBD_HOME       */
    -1                  , /*  KBD_HOLD       */
    KBD_LTRIGGER_MAPPING  , /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING  , /*  KBD_RTRIGGER   */
    CPC_J0_UP           , /*  KBD_JOY_UP     */
    CPC_J0_RIGHT        , /*  KBD_JOY_RIGHT  */
    CPC_J0_DOWN         , /*  KBD_JOY_DOWN   */
    CPC_J0_LEFT           /*  KBD_JOY_LEFT   */
  };

  static int loc_default_mapping_R[ KBD_ALL_BUTTONS ] = {
    CPC_CUR_UP          , /*  KBD_UP         */
    CAP32_INCFIRE       , /*  KBD_RIGHT      */
    CPC_CUR_DOWN        , /*  KBD_DOWN       */
    CAP32_DECFIRE       , /*  KBD_LEFT       */
    CPC_SPACE           , /*  KBD_TRIANGLE   */
    CPC_ESC             , /*  KBD_CIRCLE     */
    CAP32_AUTOFIRE      , /*  KBD_CROSS      */
    CPC_DEL             , /*  KBD_SQUARE     */
    -1                  , /*  KBD_SELECT     */
    -1                  , /*  KBD_START      */
    -1                  , /*  KBD_HOME       */
    -1                  , /*  KBD_HOLD       */
    KBD_LTRIGGER_MAPPING  , /*  KBD_LTRIGGER   */
    KBD_RTRIGGER_MAPPING  , /*  KBD_RTRIGGER   */
    CPC_J0_UP           , /*  KBD_JOY_UP     */
    CPC_J0_RIGHT        , /*  KBD_JOY_RIGHT  */
    CPC_J0_DOWN         , /*  KBD_JOY_DOWN   */
    CPC_J0_LEFT           /*  KBD_JOY_LEFT   */
  };


 int psp_kbd_mapping[ KBD_ALL_BUTTONS ];
 int psp_kbd_mapping_L[ KBD_ALL_BUTTONS ];
 int psp_kbd_mapping_R[ KBD_ALL_BUTTONS ];
 int psp_kbd_presses[ KBD_ALL_BUTTONS ];
 int kbd_ltrigger_mapping_active;
 int kbd_rtrigger_mapping_active;

 static int danzeff_cpc_key     = 0;
 static int danzeff_cpc_pending = 0;
        int danzeff_mode        = 0;

#ifdef USE_PSP_IRKEYB
# define IRKEYB_CPC_MAX_PENDING 20
 static int irkeyb_cpc_pending_max  = 0;
 static int irkeyb_cpc_pending_curr = 0;
 static int irkeyb_cpc_pending_keys[IRKEYB_CPC_MAX_PENDING];
# endif
# define ANDKEYB_CPC_MAX_PENDING 20
 static int andkeyb_cpc_pending_max  = 0;
 static int andkeyb_cpc_pending_curr = 0;
 static int andkeyb_cpc_pending_keys[ANDKEYB_CPC_MAX_PENDING];
 static unsigned int   loc_last_andkbd_event_time = 0;
# define KBD_MIN_AND_PENDING_TIME  100000


       char command_keys[ 128 ];
 static int command_mode        = 0;
 static int command_index       = 0;
 static int command_size        = 0;
 static int command_cpc_pending = 0;
 static int command_cpc_key     = 0;

void
psp_kbd_run_command(char *Command, int wait)
{
  strncpy(command_keys, Command, 128);
  command_size  = strlen(Command);
  command_index = 0;

  command_cpc_key     = 0;
  command_cpc_pending = 0;
  command_mode        = wait;
}

void
psp_kbd_default_settings()
{
  memcpy(psp_kbd_mapping  , loc_default_mapping, sizeof(loc_default_mapping));
  memcpy(psp_kbd_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_L));
  memcpy(psp_kbd_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));
}

int
psp_kbd_reset_hotkeys(void)
{
  int index;
  int key_id;
  for (index = 0; index < KBD_ALL_BUTTONS; index++) {
    key_id = loc_default_mapping[index];
    if ((key_id >= CAP32_FPS) && (key_id <= CAP32_SCREEN)) {
      psp_kbd_mapping[index] = key_id;
    }
    key_id = loc_default_mapping_L[index];
    if ((key_id >= CAP32_FPS) && (key_id <= CAP32_SCREEN)) {
      psp_kbd_mapping_L[index] = key_id;
    }
    key_id = loc_default_mapping_R[index];
    if ((key_id >= CAP32_FPS) && (key_id <= CAP32_SCREEN)) {
      psp_kbd_mapping_R[index] = key_id;
    }
  }
  return 0;
}

int
psp_kbd_load_mapping_file(FILE *KbdFile)
{
  char     Buffer[512];
  char    *Scan;
  int      tmp_mapping[KBD_ALL_BUTTONS];
  int      tmp_mapping_L[KBD_ALL_BUTTONS];
  int      tmp_mapping_R[KBD_ALL_BUTTONS];
  int      cpc_key_id = 0;
  int      kbd_id = 0;

  memcpy(tmp_mapping  , loc_default_mapping  , sizeof(loc_default_mapping));
  memcpy(tmp_mapping_L, loc_default_mapping_L, sizeof(loc_default_mapping_R));
  memcpy(tmp_mapping_R, loc_default_mapping_R, sizeof(loc_default_mapping_R));

  while (fgets(Buffer,512,KbdFile) != (char *)0) {
      
      Scan = strchr(Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(Buffer,'\r');
      if (Scan) *Scan = '\0';
      if (Buffer[0] == '#') continue;

      Scan = strchr(Buffer,'=');
      if (! Scan) continue;
    
      *Scan = '\0';
      cpc_key_id = atoi(Scan + 1);

      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,kbd_button_name[kbd_id])) {
          tmp_mapping[kbd_id] = cpc_key_id;
          //break;
        }
      }
      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,kbd_button_name_L[kbd_id])) {
          tmp_mapping_L[kbd_id] = cpc_key_id;
          //break;
        }
      }
      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,kbd_button_name_R[kbd_id])) {
          tmp_mapping_R[kbd_id] = cpc_key_id;
          //break;
        }
      }
  }

  memcpy(psp_kbd_mapping, tmp_mapping, sizeof(psp_kbd_mapping));
  memcpy(psp_kbd_mapping_L, tmp_mapping_L, sizeof(psp_kbd_mapping_L));
  memcpy(psp_kbd_mapping_R, tmp_mapping_R, sizeof(psp_kbd_mapping_R));
  
  return 0;
}

int
psp_kbd_load_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "r");
  error   = 1;

  if (KbdFile != (FILE*)0) {
    psp_kbd_load_mapping_file(KbdFile);
    error = 0;
    fclose(KbdFile);
  }

  kbd_ltrigger_mapping_active = 0;
  kbd_rtrigger_mapping_active = 0;
    
  return error;
}


int
psp_kbd_save_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      kbd_id = 0;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "w");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", kbd_button_name[kbd_id], psp_kbd_mapping[kbd_id]);
    }
    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", kbd_button_name_L[kbd_id], psp_kbd_mapping_L[kbd_id]);
    }
    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", kbd_button_name_R[kbd_id], psp_kbd_mapping_R[kbd_id]);
    }
    error = 0;
    fclose(KbdFile);
  }

  return error;
}

int
psp_kbd_enter_command()
{
  SceCtrlData  c;

  unsigned int command_key = 0;
  int          cpc_key     = 0;

  sceCtrlPeekBufferPositive(&c, 1);

  if (command_cpc_pending) 
  {
    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
      loc_last_event_time = c.TimeStamp;
      command_cpc_pending = 0;
      cpc_key_event(command_cpc_key, 0);
    }

    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_COMMAND_TIME) {
    loc_last_event_time = c.TimeStamp;

    if (command_mode > 1) {
      command_mode--;
      return 0;
    }

    if (command_index >= command_size) {

      command_mode  = 0;
      command_index = 0;
      command_size  = 0;

      command_cpc_pending = 0;
      command_cpc_key     = 0;

      return 0;
    }
  
    command_key = command_keys[command_index++];
    cpc_key = cpc_get_key_from_ascii(command_key);

    if (cpc_key != -1) {
      command_cpc_key     = cpc_key;
      command_cpc_pending = 1;
      cpc_key_event(command_cpc_key, 1);
    }

    return 1;
  }

  return 0;
}

int 
psp_kbd_is_danzeff_mode()
{
  return danzeff_mode;
}

int
psp_kbd_enter_danzeff()
{
  unsigned int danzeff_key = 0;
  int          cpc_key     = 0;
  SceCtrlData  c;

  if (! danzeff_mode) {
    psp_init_keyboard();
    danzeff_mode = 1;
  }

  sceCtrlPeekBufferPositive(&c, 1);
  c.Buttons &= PSP_ALL_BUTTON_MASK;

  if (danzeff_cpc_pending) 
  {
    if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_PENDING_TIME) {
      loc_last_event_time = c.TimeStamp;
      danzeff_cpc_pending = 0;
      cpc_key_event(danzeff_cpc_key, 0);
    }
    return 0;
  }

  if ((c.TimeStamp - loc_last_event_time) > KBD_MIN_DANZEFF_TIME) {
    loc_last_event_time = c.TimeStamp;
  
    sceCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;
# ifdef USE_PSP_IRKEYB
    psp_irkeyb_set_psp_key(&c);
# endif
    danzeff_key = danzeff_readInput(c);
  }

  if (danzeff_key > DANZEFF_START) {
    cpc_key = cpc_get_key_from_ascii(danzeff_key);

    if (cpc_key != -1) {
      danzeff_cpc_key     = cpc_key;
      danzeff_cpc_pending = 1;
      cpc_key_event(danzeff_cpc_key, 1);
    }

    return 1;

  } else if (danzeff_key == DANZEFF_START) {
    danzeff_mode        = 0;
    danzeff_cpc_pending = 0;
    danzeff_cpc_key     = 0;

    psp_kbd_wait_no_button();

  } else if (danzeff_key == DANZEFF_SELECT) {
    danzeff_mode        = 0;
    danzeff_cpc_pending = 0;
    danzeff_cpc_key     = 0;
    psp_main_menu();
    psp_init_keyboard();

    psp_kbd_wait_no_button();
  }

  return danzeff_key;
}

#ifdef USE_PSP_IRKEYB
int
psp_kbd_enter_irkeyb()
{
  int cpc_key   = 0;
  int psp_irkeyb = PSP_IRKEYB_EMPTY;

  SceCtrlData  c;
  sceCtrlPeekBufferPositive(&c, 1);

  if (irkeyb_cpc_pending_max) 
  {
    if ((c.TimeStamp - loc_last_irkbd_event_time) > KBD_MIN_IR_PENDING_TIME) {
      loc_last_irkbd_event_time = c.TimeStamp;
      cpc_key_event(irkeyb_cpc_pending_keys[irkeyb_cpc_pending_curr], 0);
      irkeyb_cpc_pending_curr++;
      if (irkeyb_cpc_pending_curr >= irkeyb_cpc_pending_max) {
        irkeyb_cpc_pending_curr = 0;
        irkeyb_cpc_pending_max  = 0;
      }
    }
  }

  psp_irkeyb = psp_irkeyb_read_key();
  if (psp_irkeyb != PSP_IRKEYB_EMPTY) {

    if (psp_irkeyb == 0x8) {
      cpc_key = CPC_DEL;
    } else
    if (psp_irkeyb == 0x9) {
      cpc_key = CPC_TAB;
    } else
    if (psp_irkeyb == 0xd) {
      cpc_key = CPC_RETURN;
    } else
    if (psp_irkeyb == 0x1b) {
      cpc_key = CPC_ESC;
    } else
    if (psp_irkeyb == PSP_IRKEYB_UP) {
      cpc_key = CPC_CUR_UP;
    } else
    if (psp_irkeyb == PSP_IRKEYB_DOWN) {
      cpc_key = CPC_CUR_DOWN;
    } else
    if (psp_irkeyb == PSP_IRKEYB_LEFT) {
      cpc_key = CPC_CUR_LEFT;
    } else
    if (psp_irkeyb == PSP_IRKEYB_RIGHT) {
      cpc_key = CPC_CUR_RIGHT;
    } else {
      cpc_key = cpc_get_key_from_ascii(psp_irkeyb);
    }
    if (cpc_key != -1) {
      if ((irkeyb_cpc_pending_max+1) < IRKEYB_CPC_MAX_PENDING) {
        irkeyb_cpc_pending_keys[irkeyb_cpc_pending_max++] = cpc_key;
        cpc_key_event(cpc_key, 1);
      }
    }
    return 1;
  }
  return 0;
}
# endif

int
and_kbd_enter_data(int andKey)
{
	int cpc_key   = 0;
	int retVal = 0;
	char buffer[128];
	u32 currTicks;

	currTicks = SDL_GetTicks() * 1000;

	  if (andkeyb_cpc_pending_max) 
	  {
	    if ((currTicks - loc_last_andkbd_event_time) > KBD_MIN_AND_PENDING_TIME) {
	    	loc_last_andkbd_event_time = currTicks;
		  sprintf(buffer,"Freeing %X",andkeyb_cpc_pending_keys[andkeyb_cpc_pending_curr]);
	      cpc_key_event(andkeyb_cpc_pending_keys[andkeyb_cpc_pending_curr], 0);
	      andkeyb_cpc_pending_curr++;
	    	  
	      if (andkeyb_cpc_pending_curr >= andkeyb_cpc_pending_max) {
	    	andkeyb_cpc_pending_curr = 0;
	        andkeyb_cpc_pending_max  = 0;
	      }
	      else
	      {
			  sprintf(buffer,"Pressing %X",andkeyb_cpc_pending_keys[andkeyb_cpc_pending_curr]);
	    	  cpc_key_event(andkeyb_cpc_pending_keys[andkeyb_cpc_pending_curr], 1);
	    	  retVal = 1;
	      }
	    }
	  }
	  else
	    	loc_last_andkbd_event_time = currTicks;		  
	  if (andKey != 0)
	  {
		if (andKey == 0x8) {
		  cpc_key = CPC_DEL;
		} else
		if (andKey == 0x9) {
		  cpc_key = CPC_TAB;
		} else
		if (andKey == 0xd) {
		  cpc_key = CPC_RETURN;
		} else
		if (andKey == 0x1b) {
		  cpc_key = CPC_ESC;
		} else {
		  cpc_key = cpc_get_key_from_ascii(andKey);
		}
		if (cpc_key != -1) {
		  if ((andkeyb_cpc_pending_max+1) < ANDKEYB_CPC_MAX_PENDING) {
			  andkeyb_cpc_pending_keys[andkeyb_cpc_pending_max++] = cpc_key;
			  if (andkeyb_cpc_pending_max==1)
			  {
				  cpc_key_event(cpc_key, 1);
				  sprintf(buffer,"Pressing %X",cpc_key);
				  retVal = 1;
			  }
		  }
		}
	  }
	  return retVal;
}

int
cpc_decode_key(int psp_b, int button_pressed)
{
  int wake = 0;
  int reverse_analog = CPC.psp_reverse_analog;

  if (reverse_analog) {
    if ((psp_b >= KBD_JOY_UP  ) &&
        (psp_b <= KBD_JOY_LEFT)) {
      psp_b = psp_b - KBD_JOY_UP + KBD_UP;
    } else
    if ((psp_b >= KBD_UP  ) &&
        (psp_b <= KBD_LEFT)) {
      psp_b = psp_b - KBD_UP + KBD_JOY_UP;
    }
  }

  if (psp_b == KBD_START) {
     if (button_pressed) psp_kbd_enter_danzeff();
  } else
  if (psp_b == KBD_SELECT) {
    if (button_pressed) {
      psp_main_menu();
      psp_init_keyboard();
    }
  } else {
 
    if (psp_kbd_mapping[psp_b] >= 0) {
      wake = 1;
      if (button_pressed) {
        // Determine which buton to press first (ie which mapping is currently active)
        if (kbd_ltrigger_mapping_active) {
          // Use ltrigger mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping_L[psp_b];
          cpc_key_event(psp_kbd_presses[psp_b], button_pressed);
        } else
        if (kbd_rtrigger_mapping_active) {
          // Use rtrigger mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping_R[psp_b];
          cpc_key_event(psp_kbd_presses[psp_b], button_pressed);
        } else {
          // Use standard mapping
          psp_kbd_presses[psp_b] = psp_kbd_mapping[psp_b];
          cpc_key_event(psp_kbd_presses[psp_b], button_pressed);
        }
      } else {
          // Determine which button to release (ie what was pressed before)
          cpc_key_event(psp_kbd_presses[psp_b], button_pressed);
      }

    } else {
      if (psp_kbd_mapping[psp_b] == KBD_LTRIGGER_MAPPING) {
        kbd_ltrigger_mapping_active = button_pressed;
        kbd_rtrigger_mapping_active = 0;
      } else
      if (psp_kbd_mapping[psp_b] == KBD_RTRIGGER_MAPPING) {
        kbd_rtrigger_mapping_active = button_pressed;
        kbd_ltrigger_mapping_active = 0;
      }
    }
  }
  return 0;
}

# define ANALOG_THRESHOLD 60

void 
kbd_get_analog_direction(int Analog_x, int Analog_y, int *x, int *y)
{
  int DeltaX = 255;
  int DeltaY = 255;
  int DirX   = 0;
  int DirY   = 0;

  *x = 0;
  *y = 0;

  if (Analog_x <=        ANALOG_THRESHOLD)  { DeltaX = Analog_x; DirX = -1; }
  else 
  if (Analog_x >= (255 - ANALOG_THRESHOLD)) { DeltaX = 255 - Analog_x; DirX = 1; }

  if (Analog_y <=        ANALOG_THRESHOLD)  { DeltaY = Analog_y; DirY = -1; }
  else 
  if (Analog_y >= (255 - ANALOG_THRESHOLD)) { DeltaY = 255 - Analog_y; DirY = 1; }

  *x = DirX;
  *y = DirY;
}

void
kbd_change_auto_fire(int auto_fire)
{
  CPC.cpc_auto_fire = auto_fire;
  if (CPC.cpc_auto_fire_pressed) {
    cpc_key_event(CPC_J0_FIRE1, 0);
    CPC.cpc_auto_fire_pressed = 0;
  }
}

int
kbd_scan_keyboard(void)
{
  SceCtrlData c;
  long        delta_stamp;
  int         event;
  int         b;

  int new_Lx;
  int new_Ly;
  int old_Lx;
  int old_Ly;

  event = 0;
  myCtrlFastPeekBufferPositive( &c, 1 );
  c.Buttons &= PSP_ALL_BUTTON_MASK;

# ifdef USE_PSP_IRKEYB
  psp_irkeyb_set_psp_key(&c);
# endif
# ifdef USE_IRDA_JOY
  if (CPC.psp_irdajoy_type != IRDAJOY_TYPE_NONE) {
    psp_irda_get_joy_event( &loc_irda_joy_event );
    psp_irda_joy_convert( &loc_irda_joy_event, &c );
  }
# endif

  if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
      (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
    /* Exit ! */
    psp_sdl_exit(0);
  }

  delta_stamp = c.TimeStamp - first_time_batt_stamp;
  if ((delta_stamp < 0) || (delta_stamp > KBD_MIN_BATTCHECK_TIME)) {
    first_time_batt_stamp = c.TimeStamp;
    if (psp_is_low_battery()) {
      psp_main_menu();
      psp_init_keyboard();
      return 0;
    }
  }

  if (CPC.cpc_auto_fire) {
    delta_stamp = c.TimeStamp - first_time_auto_stamp;
    if ((delta_stamp < 0) || 
        (delta_stamp > (KBD_MIN_AUTOFIRE_TIME / (1 + CPC.cpc_auto_fire_period)))) {
      first_time_auto_stamp = c.TimeStamp;
      cpc_key_event(CPC_J0_FIRE1, CPC.cpc_auto_fire_pressed);
      CPC.cpc_auto_fire_pressed = ! CPC.cpc_auto_fire_pressed;
    }
  }

  for (b = 0; b < KBD_MAX_BUTTONS; b++) 
  {
    if (c.Buttons & kbd_button_mask[b]) {
      if (!(loc_button_data.Buttons & kbd_button_mask[b])) {
        loc_button_press[b] = 1;
        event = 1;
      }
    } else {
      if (loc_button_data.Buttons & kbd_button_mask[b]) {
        loc_button_release[b] = 1;
        loc_button_press[b] = 0;
        event = 1;
      }
    }
  }
  memcpy(&loc_button_data,&c,sizeof(SceCtrlData));

  return event;
}

void
psp_kbd_wait_start(void)
{
  while (1)
  {
    SceCtrlData c;
    sceCtrlReadBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;
    if (c.Buttons & PSP_CTRL_START) break;
  }
  psp_kbd_wait_no_button();
}

void
psp_init_keyboard(void)
{
  cpc_kbd_reset();
  kbd_ltrigger_mapping_active = 0;
  kbd_rtrigger_mapping_active = 0;
}

void
psp_kbd_wait_no_button(void)
{
  SceCtrlData c;

  do {
   myCtrlPeekBufferPositive(&c, 1);
   c.Buttons &= PSP_ALL_BUTTON_MASK;

  } while (c.Buttons != 0);
} 

void
psp_kbd_wait_button(void)
{
  SceCtrlData c;

  do {
   sceCtrlReadBufferPositive(&c, 1);
   c.Buttons &= PSP_ALL_BUTTON_MASK;
  } while (c.Buttons == 0);
} 

int
psp_update_keys(void)
{
  int         b;

  static char first_time = 1;
  static int release_pending = 0;

  if (first_time) {

    SceCtrlData c;
    sceCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (first_time_stamp == -1) first_time_stamp = c.TimeStamp;

    first_time      = 0;
    release_pending = 0;

    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      loc_button_release[b] = 0;
      loc_button_press[b] = 0;
    }
    sceCtrlPeekBufferPositive(&loc_button_data, 1);
    loc_button_data.Buttons &= PSP_ALL_BUTTON_MASK;

    psp_main_menu();
    psp_init_keyboard();

    return 0;
  }

  if (command_mode) {
    return psp_kbd_enter_command();
  }

  if (danzeff_mode) {
    return psp_kbd_enter_danzeff();
  }

# ifdef USE_PSP_IRKEYB
  if (psp_kbd_enter_irkeyb()) {
    return 1;
  }
# endif

  if (and_kbd_enter_data(0)) {
    return 1;
  }

  if (release_pending)
  {
    release_pending = 0;
    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      if (loc_button_release[b]) {
        loc_button_release[b] = 0;
        cpc_decode_key(b, 0);
      }
    }
  }

  kbd_scan_keyboard();

  /* check press event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_press[b]) {
      loc_button_press[b] = 0;
      release_pending     = 0;
      cpc_decode_key(b, 1);
    }
  }
  /* check release event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_release[b]) {
      release_pending = 1;
      break;
    }
  }

  return 0;
}
