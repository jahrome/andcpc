/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
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
#include "video.h"
#include "psp_sdl.h"
#include "psp_kbd.h"
#include "psp_menu.h"
#include "psp_fmgr.h"
#include "psp_menu_kbd.h"
#include "psp_menu_set.h"
#include "psp_battery.h"
#include "psp_irkeyb.h"

extern SDL_Surface *back_surface;
static int psp_menu_dirty = 1;

# define MENU_SET_SOUND         0
# define MENU_SET_IRDA_MODE     1
# define MENU_SET_VIEW_FPS      2
# define MENU_SET_SPEED_LIMIT   3
# define MENU_SET_SKIP_FPS      4
# define MENU_SET_RAM_SIZE      5
# define MENU_SET_RENDER        6
# define MENU_SET_RENDER_DY     7
# define MENU_SET_GREEN         8
# define MENU_SET_BORDER        9
# define MENU_SET_CLOCK        10
# define MENU_SET_SPEED        11
# define MENU_SET_EXPLORE      12
# define MENU_SET_RESET_LOAD   13

# define MENU_SET_LOAD         14
# define MENU_SET_SAVE         15
# define MENU_SET_RESET        16
                                 
# define MENU_SET_BACK         17

# define MAX_MENU_SET_ITEM (MENU_SET_BACK + 1)

  static menu_item_t menu_list[] =
  {
    { "Sound enable       :"},
    { "IRDA device        :"},
    { "Display fps        :"},
    { "Speed limiter      :"},
    { "Skip frame         :"},
    { "Ram size           :"},
    { "Render mode        :"},
    { "Render delta Y     :"},
    { "Green color        :"},
    { "Display border     :"},
    { "Clock frequency    :"},
    { "Sound tick ratio   :"},
    { "Disk startup       :"},
    { "Reset on startup   :"},

    { "Load settings"        },
    { "Save settings"        },
    { "Reset settings"       },
    { "Back to Menu"         }
  };

  static int cur_menu_id = MENU_SET_LOAD;

  static int cpc_snd_enabled       = 0;
  static int cpc_speed_limiter     = 50;
  static int cpc_view_fps          = 0;
  static int cpc_green             = 0;
  static int cpc_render_mode       = 0;
  static int cpc_render_delta_y    = 0;
  static int cpc_display_border    = 0;
  static int cpc_speed             = DEF_SPEED_SETTING;
  static int psp_cpu_clock         = 222;
  static int cpc_skip_fps          = 0;
  static int cpc_ram_size          = 128;
  static int psp_explore_disk      = 0;
  static int cpc_reset_load_disk   = 1;
  static int psp_irda_mode       = 0;

static void
psp_settings_menu_reset(void);

static void 
psp_display_screen_settings_menu(void)
{
  char buffer[64];
  int menu_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  //if (psp_menu_dirty) 
  {

    psp_sdl_blit_help();
    psp_menu_dirty = 0;

    psp_sdl_draw_rectangle(5,5,390,230,PSP_MENU_BORDER_COLOR,0);

    psp_sdl_back2_print( 30, 2, " 5: Keyboard ", PSP_MENU_NOTE_COLOR);

    psp_sdl_back2_print( 270, 2, " 6: Reset ", PSP_MENU_WARNING_COLOR);

    psp_sdl_back2_print(70, 230, " 2: Cancel  1: Valid  Back: Back ", 
                       PSP_MENU_BORDER_COLOR);
  }
  
  x      = 20;
  y      = 10;
  y_step = 10;
  
  for (menu_id = 0; menu_id < MAX_MENU_SET_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;

    psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);

    if (menu_id == MENU_SET_SOUND) {
      if (cpc_snd_enabled) strcpy(buffer,"yes");
      else                 strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_VIEW_FPS) {
      if (cpc_view_fps) strcpy(buffer,"on ");
      else              strcpy(buffer,"off");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_GREEN) {
      if (cpc_green) strcpy(buffer,"yes");
      else              strcpy(buffer,"no");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SKIP_FPS) {
      sprintf(buffer,"%d", cpc_skip_fps);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_RAM_SIZE) {
      sprintf(buffer,"%d", cpc_ram_size);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SPEED_LIMIT) {
      if (cpc_speed_limiter == 0)  strcpy(buffer, "no");
      else sprintf(buffer, "%d fps", cpc_speed_limiter);
      string_fill_with_space(buffer, 7);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_BORDER) {
      if (cpc_display_border) strcpy(buffer,"yes");
      else                strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_EXPLORE) {
      if (psp_explore_disk == CPC_EXPLORE_AUTO  ) strcpy(buffer,"auto");
      else
      if (psp_explore_disk == CPC_EXPLORE_FULL_AUTO) strcpy(buffer,"full");
      else
      if (psp_explore_disk == CPC_EXPLORE_MANUAL) strcpy(buffer,"manual");
      else
      if (psp_explore_disk == CPC_EXPLORE_LOAD  ) strcpy(buffer,"load");
      string_fill_with_space(buffer, 10);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_RESET_LOAD) {
      if (cpc_reset_load_disk) strcpy(buffer,"yes");
      else                strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
      y += y_step;
    } else
    if (menu_id == MENU_SET_RENDER) {
      if (cpc_render_mode == CPC_RENDER_ULTRA    ) strcpy(buffer, "ultra");
      else 
      if (cpc_render_mode == CPC_RENDER_FAST     ) strcpy(buffer, "fast");
      else 
      if (cpc_render_mode == CPC_RENDER_NORMAL   ) strcpy(buffer, "normal");
      else 
      if (cpc_render_mode == CPC_RENDER_FIT_WIDTH) strcpy(buffer, "fit width");
      else
      if (cpc_render_mode == CPC_RENDER_FIT_HEIGHT) strcpy(buffer, "fit height");
      else
      if (cpc_render_mode == CPC_RENDER_X125     ) strcpy(buffer, "x1.25");
      else                                         strcpy(buffer, "x1.5");

      string_fill_with_space(buffer, 13);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_RENDER_DY) {
      sprintf(buffer,"%d", cpc_render_delta_y);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_CLOCK) {
      sprintf(buffer,"%d", psp_cpu_clock);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SPEED) {
      sprintf(buffer,"%d", cpc_speed);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_IRDA_MODE) {
      if (psp_irda_mode == 0) strcpy(buffer, "none");
      else
      if (psp_irda_mode == 1) strcpy(buffer, "keyboard");
      else                    strcpy(buffer, "joystick");
      string_fill_with_space(buffer, 10);
      psp_sdl_back2_print(190, y, buffer, color);
    } else
    if (menu_id == MENU_SET_RESET) {
      /* y += y_step; */
    }

    y += y_step;
  }

  psp_menu_display_save_name();
}


#define MAX_CLOCK_VALUES 5
static int clock_values[MAX_CLOCK_VALUES] = { 133, 222, 266, 300, 333 };

static void
psp_settings_menu_clock(int step)
{
  int index;
  for (index = 0; index < MAX_CLOCK_VALUES; index++) {
    if (psp_cpu_clock == clock_values[index]) break;
  }
  if (step > 0) {
    index++;
    if (index >= MAX_CLOCK_VALUES) index = 0;
    psp_cpu_clock = clock_values[index];

  } else {
    index--;

    if (index < 0) index = MAX_CLOCK_VALUES - 1;
    psp_cpu_clock = clock_values[index];
  }
}

static void
psp_settings_menu_skip_fps(int step)
{
  if (step > 0) {
    if (cpc_skip_fps < 25) cpc_skip_fps++;
  } else {
    if (cpc_skip_fps > 0) cpc_skip_fps--;
  }
}

static void
psp_settings_menu_speed(int step)
{
  if (step > 0) {
    if (cpc_speed < MAX_SPEED_SETTING) cpc_speed++;
  } else {
    if (cpc_speed > MIN_SPEED_SETTING) cpc_speed--;
  }
}

static void
psp_settings_menu_ram_size(int step)
{
  if (step > 0) {

    if (cpc_ram_size == 128) cpc_ram_size = 256;
    else
    if (cpc_ram_size == 256) cpc_ram_size = 576;

  } else {

    if (cpc_ram_size == 576) cpc_ram_size = 256;
    else
    if (cpc_ram_size == 256) cpc_ram_size = 128;
  }
}

void
psp_settings_menu_render(int step)
{
  if (step > 0) {
    if (cpc_render_mode < CPC_LAST_RENDER) cpc_render_mode++;
    else                                   cpc_render_mode = 0;
  } else {
    if (cpc_render_mode > 0) cpc_render_mode--;
    else                     cpc_render_mode = CPC_LAST_RENDER;
  }
}

void
psp_settings_menu_render_dy(int step)
{
  if (step > 0) {
    if (cpc_render_delta_y < 72) cpc_render_delta_y++;
    else                         cpc_render_delta_y = -72;
  } else {
    if (cpc_render_delta_y > -72) cpc_render_delta_y--;
    else                          cpc_render_delta_y = 72;
  }
}

static void
psp_settings_menu_explore(int step)
{
  if (step > 0) {
    if (psp_explore_disk < CPC_EXPLORE_FULL_AUTO) psp_explore_disk++;
    else                                    psp_explore_disk = 0;
  } else {
    if (psp_explore_disk > 0) psp_explore_disk--;
    else                     psp_explore_disk = CPC_EXPLORE_FULL_AUTO;
  }
}

static void
psp_settings_menu_irda_mode(int step)
{
  if (step > 0) {
    if (psp_irda_mode < 2) psp_irda_mode++;
    else                   psp_irda_mode = 0;
  } else {
    if (psp_irda_mode >  0) psp_irda_mode--;
    else                    psp_irda_mode  = 2;
  }
}

static void
psp_settings_menu_limiter(int step)
{
  if (step > 0) {
    if (cpc_speed_limiter < 60) cpc_speed_limiter++;
    else                        cpc_speed_limiter = 0;
  } else {
    if (cpc_speed_limiter > 0) cpc_speed_limiter--;
    else                       cpc_speed_limiter = 60;
  }
}

static void
psp_settings_menu_init(void)
{
  cpc_snd_enabled      = CPC.snd_enabled;
  cpc_view_fps         = CPC.cpc_view_fps;
  cpc_green            = CPC.cpc_green;
  cpc_speed_limiter    = CPC.cpc_speed_limiter;
  cpc_render_mode      = CPC.cpc_render_mode;
  cpc_render_delta_y   = CPC.cpc_render_delta_y;
  cpc_display_border   = CPC.cpc_display_border;
  psp_explore_disk     = CPC.psp_explore_disk;
  cpc_reset_load_disk  = CPC.cpc_reset_load_disk;
  cpc_skip_fps         = CPC.psp_skip_max_frame;
  psp_cpu_clock        = CPC.psp_cpu_clock;
  cpc_speed            = CPC.speed;
  cpc_ram_size         = CPC.ram_size;
  //psp_irda_mode       = psp_irda_get_saved_mode();
}

static void
psp_settings_menu_load(int format)
{
  int ret;

  ret = psp_fmgr_menu(format, 0);
  if (ret ==  1) /* load OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "File loaded !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
    psp_settings_menu_init();
  }
  else 
  if (ret == -1) /* Load Error */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "Can't load file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_settings_menu_validate(void)
{
  CPC.snd_enabled         = cpc_snd_enabled;
  CPC.cpc_view_fps        = cpc_view_fps;
  CPC.psp_cpu_clock       = psp_cpu_clock;
  CPC.psp_skip_max_frame  = cpc_skip_fps;
  CPC.cpc_speed_limiter   = cpc_speed_limiter;
  CPC.psp_skip_cur_frame  = 0;
  CPC.cpc_display_border  = cpc_display_border;
  CPC.psp_explore_disk    = psp_explore_disk;
  CPC.cpc_reset_load_disk = cpc_reset_load_disk;
  CPC.cpc_render_delta_y  = cpc_render_delta_y;

  /*if (psp_irda_set_saved_mode( psp_irda_mode )) {
      
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  105, "IRDA config saved", 
                       PSP_MENU_NOTE_COLOR);
    psp_sdl_back2_print(240,  115, "Emulator restart needed !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(2);
  }*/

  cap32_change_green_mode(cpc_green);
  cap32_change_render_mode(cpc_render_mode);

  if (CPC.speed != cpc_speed) {
    CPC.speed = cpc_speed;
    InitAYCounterVars();
  }

  if (CPC.ram_size != cpc_ram_size) {
    cap32_change_ram_size(cpc_ram_size);
  }

# ifndef PSP_ME
  scePowerSetClockFrequency(CPC.psp_cpu_clock, CPC.psp_cpu_clock, CPC.psp_cpu_clock/2);
# endif
}

static void
psp_settings_menu_save()
{
  int error;

  psp_settings_menu_validate();
  error = cap32_save_settings();

  if (! error) /* save OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_settings_menu_reset(void)
{
  psp_display_screen_settings_menu();
  psp_sdl_back2_print(270, 80, "Reset Settings !", 
                     PSP_MENU_WARNING_COLOR);
  psp_menu_dirty = 1;
  psp_sdl_flip();
  cap32_default_settings();
  psp_settings_menu_init();
  sleep(1);
}

int 
psp_settings_menu(void)
{
  SceCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  psp_kbd_wait_no_button();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  psp_settings_menu_init();

  psp_menu_dirty = 1;

  while (! end_menu)
  {
    psp_display_screen_settings_menu();
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
    if ((c.Buttons & PSP_CTRL_LTRIGGER) == PSP_CTRL_LTRIGGER) {
      psp_keyboard_menu();
      end_menu = 1;
    } else
    if ((c.Buttons & PSP_CTRL_RTRIGGER) == PSP_CTRL_RTRIGGER) {
      psp_settings_menu_reset();
      end_menu = 1;
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

      switch (cur_menu_id ) 
      {
        case MENU_SET_SOUND     : cpc_snd_enabled = ! cpc_snd_enabled;
        break;              
        case MENU_SET_SPEED_LIMIT : psp_settings_menu_limiter( step );
        break;              
        case MENU_SET_VIEW_FPS    : cpc_view_fps     = ! cpc_view_fps;
        break;              
        case MENU_SET_GREEN       : cpc_green     = ! cpc_green;
        break;              
        case MENU_SET_SPEED     : psp_settings_menu_speed( step );
        break;              
        case MENU_SET_SKIP_FPS  : psp_settings_menu_skip_fps( step );
        break;              
        case MENU_SET_RENDER    : psp_settings_menu_render( step );
        break;              
        case MENU_SET_RENDER_DY : psp_settings_menu_render_dy( step );
        break;              
        case MENU_SET_BORDER    : cpc_display_border = ! cpc_display_border;
        break;              
        case MENU_SET_EXPLORE   : psp_settings_menu_explore( step );
        break;              
        case MENU_SET_RESET_LOAD : cpc_reset_load_disk = ! cpc_reset_load_disk;
        break;              
        case MENU_SET_RAM_SIZE  : psp_settings_menu_ram_size( step );
        break;              
        case MENU_SET_CLOCK     : psp_settings_menu_clock( step );
        break;
        case MENU_SET_IRDA_MODE  : psp_settings_menu_irda_mode( step );
        break;
        case MENU_SET_LOAD       : psp_settings_menu_load(FMGR_FORMAT_SET);
                                   psp_menu_dirty = 1;
                                   old_pad = new_pad = 0;
        break;              
        case MENU_SET_SAVE       : psp_settings_menu_save();
                                   psp_menu_dirty = 1;
                                   old_pad = new_pad = 0;
        break;                     
        case MENU_SET_RESET      : psp_settings_menu_reset();
        break;                     
                                   
        case MENU_SET_BACK       : end_menu = 1;
        break;                     
      }

    } else
    if(new_pad & PSP_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_SET_ITEM-1;

    } else
    if(new_pad & PSP_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_SET_ITEM-1)) cur_menu_id++;
      else                                     cur_menu_id = 0;

    } else  
    if(new_pad & PSP_CTRL_SQUARE) {
      /* Cancel */
      end_menu = -1;
    } else 
    if((new_pad & PSP_CTRL_SELECT) == PSP_CTRL_SELECT) {
      /* Back to CPC */
      end_menu = 1;
    }
  }
 
  if (end_menu > 0) {
    psp_settings_menu_validate();
  }

  psp_kbd_wait_no_button();

  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();
  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();

  return 1;
}

