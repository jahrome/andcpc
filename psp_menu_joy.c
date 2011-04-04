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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>

#include "cap32.h"
#include "psp_sdl.h"
#include "psp_kbd.h"
#include "psp_menu.h"
#include "psp_fmgr.h"
#include "psp_menu_kbd.h"
#include "psp_menu_set.h"
#include "psp_battery.h"
#include "psp_irkeyb.h"
#include "psp_kbd.h"
#include "psp_irda.h"
#include "psp_joy.h"

extern SDL_Surface *back_surface;
static int psp_menu_dirty = 1;

# define MENU_JOY_ANALOG        0
# define MENU_JOY_AUTOFIRE_T    1
# define MENU_JOY_AUTOFIRE_M    2

# define MENU_JOY_MODE          3
# define MENU_JOY_DEBUG         4
# define MENU_JOY_UP            5
# define MENU_JOY_DOWN          6
# define MENU_JOY_LEFT          7
# define MENU_JOY_RIGHT         8
# define MENU_JOY_FIRE          9
# define MENU_JOY_PADDLE1P     10
# define MENU_JOY_PADDLE1M     11
# define MENU_JOY_PADDLE1F     12
# define MENU_JOY_PADDLE2P     13
# define MENU_JOY_PADDLE2M     14
# define MENU_JOY_PADDLE2F     15

# define MENU_JOY_LOAD         16
# define MENU_JOY_SAVE         17
# define MENU_JOY_RESET        18
# define MENU_JOY_BACK         19

# define MAX_MENU_JOY_ITEM (MENU_JOY_BACK + 1)

  static menu_item_t menu_list[] =
  {
    { "Swap Analog/Cursor :"},
    { "Auto fire period   :"},
    { "Auto fire mode     :"},

    { "IRDA mode          :"},
    { "IRDA debug         :"},

    { "Joy Up             :"},
    { "Joy Down           :"},
    { "Joy Left           :"},
    { "Joy Right          :"},
    { "Joy Fire           :"},
    { "Paddle 1 +         :"},
    { "Paddle 1 -         :"},
    { "Paddle 1 Fire      :"},
    { "Paddle 2 +         :"},
    { "Paddle 2 -         :"},
    { "Paddle 2 Fire      :"},

    { "Load joystick"       },
    { "Save joystick"       },
    { "Reset joystick"      },

    { "Back to Menu"        }
  };

  static int cur_menu_id = MENU_JOY_LOAD;

  static int psp_irdajoy_type      = 0;
  static int psp_irdajoy_debug     = 0;
  static int psp_reverse_analog    = 0;
  static int cpc_auto_fire_period = 0;
  static int cpc_auto_fire_mode   = 0;
 
  static int loc_joy_mapping[ JOY_ALL_BUTTONS ];

static void
psp_joystick_menu_reset(void);

static void
psp_joystick_menu_autofire(int step)
{
  if (step > 0) {
    if (cpc_auto_fire_period < 19) cpc_auto_fire_period++;
  } else {
    if (cpc_auto_fire_period >  0) cpc_auto_fire_period--;
  }
}

static int
psp_joy_menu_id_to_key_id(int menu_id)
{
  int joy_id = 0;

  switch ( menu_id ) 
  {
    case MENU_JOY_UP       : joy_id = JOY_UP;       break;
    case MENU_JOY_DOWN     : joy_id = JOY_DOWN;     break;
    case MENU_JOY_LEFT     : joy_id = JOY_LEFT;     break;
    case MENU_JOY_RIGHT    : joy_id = JOY_RIGHT;    break;
    case MENU_JOY_FIRE     : joy_id = JOY_FIRE;     break;
    case MENU_JOY_PADDLE1P : joy_id = JOY_PADDLE1P; break;
    case MENU_JOY_PADDLE1M : joy_id = JOY_PADDLE1M; break;
    case MENU_JOY_PADDLE1F : joy_id = JOY_PADDLE1F; break;
    case MENU_JOY_PADDLE2P : joy_id = JOY_PADDLE2P; break;
    case MENU_JOY_PADDLE2M : joy_id = JOY_PADDLE2M; break;
    case MENU_JOY_PADDLE2F : joy_id = JOY_PADDLE2F; break;
  }
  return joy_id;
}

static void 
psp_display_screen_joystick_menu(void)
{
  char buffer[64];
  int menu_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  int joy_id  = 0;
  int kbd_key = 0;

  //if (psp_menu_dirty) 
  {

    psp_sdl_blit_help();
    psp_menu_dirty = 0;

    psp_sdl_draw_rectangle(5,5,390,230,PSP_MENU_BORDER_COLOR,0);

    psp_sdl_back2_print(30, 254, " 2: Cancel  1: Valid  Back: Back ", 
                       PSP_MENU_BORDER_COLOR);
  }
  
  x      = 20;
  y      = 10;
  y_step = 10;
  
  for (menu_id = 0; menu_id < MAX_MENU_JOY_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;

    psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);

    if (menu_id == MENU_JOY_ANALOG) {
      if (psp_reverse_analog) strcpy(buffer,"yes");
      else                    strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_JOY_AUTOFIRE_T) {
      sprintf(buffer,"%d", cpc_auto_fire_period+1);
      string_fill_with_space(buffer, 7);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_JOY_AUTOFIRE_M) {
      if (cpc_auto_fire_mode) strcpy(buffer,"yes");
      else                    strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
      //y += y_step;
    } else
    if (menu_id == MENU_JOY_MODE) {
      if (psp_irdajoy_type == IRDAJOY_TYPE_NONE    ) strcpy(buffer,"none");
      else
      if (psp_irdajoy_type == IRDAJOY_TYPE_JOYSTICK) strcpy(buffer,"joystick");
      else
      if (psp_irdajoy_type == IRDAJOY_TYPE_PADDLE  ) strcpy(buffer,"paddle");
      else                                           strcpy(buffer,"paddle x2");
      string_fill_with_space(buffer, 14);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_JOY_DEBUG) {
      if (psp_irdajoy_debug) strcpy(buffer,"yes");
      else                   strcpy(buffer,"no");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if ((menu_id >= MENU_JOY_UP      ) && 
        (menu_id <= MENU_JOY_PADDLE2F)) 
    {
      joy_id  = psp_joy_menu_id_to_key_id(menu_id);
      kbd_key = loc_joy_mapping[ joy_id ];
      if ((kbd_key >= 0) && (kbd_key < KBD_ALL_BUTTONS)) {
        strcpy(buffer, kbd_button_name[kbd_key]);
      } else
      if (kbd_key == KBD_UNASSIGNED) {
        sprintf(buffer, "UNASSIGNED");
      } else {
        sprintf(buffer, "KEY %d", kbd_key);
      }
      string_fill_with_space(buffer, 12);
      psp_sdl_back2_print(190, y, buffer, color);
      if (menu_id == MENU_JOY_PADDLE2F) {
        y += y_step;
      }
    } else
    if (menu_id == MENU_JOY_RESET) {
      //y += y_step;
    }

    y += y_step;
  }

  psp_menu_display_save_name();
}

static void
psp_joystick_menu_init(void)
{
  psp_irdajoy_type      = CPC.psp_irdajoy_type;
  psp_irdajoy_debug     = CPC.psp_irdajoy_debug;
  psp_reverse_analog    = CPC.psp_reverse_analog;
  cpc_auto_fire_period = CPC.cpc_auto_fire_period;
  cpc_auto_fire_mode   = CPC.cpc_auto_fire;

  memcpy(loc_joy_mapping  , psp_joy_mapping  , sizeof(psp_joy_mapping));
}

static void
psp_joystick_menu_validate(void)
{
  /* Validate */
  CPC.psp_irdajoy_type    = psp_irdajoy_type;
  CPC.psp_irdajoy_debug   = psp_irdajoy_debug;
  CPC.psp_reverse_analog  = psp_reverse_analog;
  CPC.cpc_auto_fire_period = cpc_auto_fire_period;
  if (cpc_auto_fire_mode != CPC.cpc_auto_fire) {
    kbd_change_auto_fire(cpc_auto_fire_mode);
  }

  memcpy(psp_joy_mapping, loc_joy_mapping, sizeof(psp_joy_mapping));
}

static void
psp_joystick_menu_load(int format)
{
  int ret;

  ret = psp_fmgr_menu(format, 0);
  if (ret ==  1) /* load OK */
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(270,  80, "File loaded !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
    psp_joystick_menu_init();
  }
  else 
  if (ret == -1) /* Load Error */
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(270,  80, "Can't load file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_joystick_menu_save_config()
{
  int error;

  psp_joystick_menu_validate();
  error = cap32_joy_save();

  if (! error) /* save OK */
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(270,  80, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(270,  80, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_joystick_menu_save()
{
  int error;

  psp_joystick_menu_validate();
  error = cap32_joy_save();

  if (! error) /* save OK */
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(270,  80, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_joystick_menu();
    psp_sdl_back2_print(270,  80, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_joystick_menu_reset(void)
{
  psp_display_screen_joystick_menu();
  psp_sdl_back2_print(270, 80, "Reset joystick !", 
                     PSP_MENU_WARNING_COLOR);
  psp_menu_dirty = 1;
  psp_sdl_flip();
  psp_joy_default_settings();
  psp_joystick_menu_init();
  sleep(1);
}

static void
psp_joystick_menu_mapping(int joy_id, int step)
{
  if (step < 0) loc_joy_mapping[joy_id]--;
  else 
  if (step > 0) loc_joy_mapping[joy_id]++;

  if (loc_joy_mapping[joy_id] < -1) loc_joy_mapping[joy_id] = KBD_ALL_BUTTONS-1;
  else
  if (loc_joy_mapping[joy_id] >= KBD_ALL_BUTTONS) loc_joy_mapping[joy_id] = -1;
}

int 
psp_joystick_menu(void)
{
  SceCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;
  int         joy_id;

  psp_kbd_wait_no_button();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;
  joy_id    = 0;

  psp_joystick_menu_init();

  psp_menu_dirty = 1;

  while (! end_menu)
  {
    psp_display_screen_joystick_menu();
    psp_sdl_flip();

    while (1)
    {
      myCtrlPeekBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;

# ifdef USE_PSP_IRKEYB
      psp_irkeyb_set_psp_key(&c);
# endif
      if (c.Buttons) break;
    }

    new_pad = c.Buttons;

    if ((old_pad != new_pad) || ((c.TimeStamp - last_time) > PSP_MENU_MIN_TIME)) {
      last_time = c.TimeStamp;
      old_pad = new_pad;

    } else continue;

    if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
        (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
      /* Exit ! */
      psp_sdl_exit(0);
    } else
    if ((new_pad & PSP_CTRL_CROSS ) || 
        (new_pad & PSP_CTRL_CIRCLE) || 
        (new_pad & PSP_CTRL_RIGHT ) ||
        (new_pad & PSP_CTRL_LEFT  )) 
    {
      int step;

      if (new_pad & PSP_CTRL_LEFT)  step = -1;
      else 
      if (new_pad & PSP_CTRL_RIGHT) step =  1;
      else                          step =  0;

      if ((cur_menu_id >= MENU_JOY_UP      ) &&
          (cur_menu_id <= MENU_JOY_PADDLE2F))
      {
        joy_id = psp_joy_menu_id_to_key_id(cur_menu_id);
        psp_joystick_menu_mapping( joy_id, step); 

      }
      else
      {
        switch (cur_menu_id ) 
        {
          case MENU_JOY_MODE : psp_irdajoy_type = (psp_irdajoy_type + 1) % (IRDAJOY_MAX_TYPE+1);
          break;              
          case MENU_JOY_ANALOG     : psp_reverse_analog = ! psp_reverse_analog;
          break;              
          case MENU_JOY_DEBUG       : psp_irdajoy_debug = ! psp_irdajoy_debug;
          break;              
          case MENU_JOY_AUTOFIRE_T  : psp_joystick_menu_autofire( step );
          break;              
          case MENU_JOY_AUTOFIRE_M  : cpc_auto_fire_mode = ! cpc_auto_fire_mode;
          break;              
          case MENU_JOY_LOAD       : psp_joystick_menu_load(FMGR_FORMAT_JOY);
                                     psp_menu_dirty = 1;
                                     old_pad = new_pad = 0;
          break;              
          case MENU_JOY_SAVE       : psp_joystick_menu_save();
                                     psp_menu_dirty = 1;
                                     old_pad = new_pad = 0;
          break;                     
          case MENU_JOY_RESET      : psp_joystick_menu_reset();
          break;                     
                                     
          case MENU_JOY_BACK       : end_menu = 1;
          break;                     
        }
      }

    } else
    if(new_pad & PSP_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_JOY_ITEM-1;

    } else
    if(new_pad & PSP_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_JOY_ITEM-1)) cur_menu_id++;
      else                                     cur_menu_id = 0;

    } else  
    if(new_pad & PSP_CTRL_SQUARE) {
      /* Cancel */
      end_menu = -1;
    } else 
    if(new_pad & PSP_CTRL_SELECT) {
      /* Back to Settings */
      end_menu = 1;
    }
  }
 
  if (end_menu > 0) {
    psp_joystick_menu_validate();
  }

  psp_kbd_wait_no_button();

  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();
  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();

  return 1;
}
