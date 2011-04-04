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
#include "psp_menu_joy.h"
#include "psp_run.h"
#include "psp_menu_set.h"
#include "psp_editor.h"
#include "psp_menu_cheat.h"
#include "psp_battery.h"
#include "psp_menu_help.h"
#include "psp_menu_disk.h"
#include "psp_irkeyb.h"

extern SDL_Surface *back_surface;
static int psp_menu_dirty = 1;

# define MENU_SCREENSHOT   0

# define MENU_LOAD_SLOT    1
# define MENU_SAVE_SLOT    2
# define MENU_DEL_SLOT     3
# define MENU_HELP         4

# define MENU_LOAD_SNA     5
# define MENU_LOAD_DSKA    6
# define MENU_EXPL_DSKA    7
# define MENU_LOAD_DSKB    8
# define MENU_COMMAND      9

# define MENU_EDITOR      10
# define MENU_CHEATS      11
# define MENU_KEYBOARD    12
# define MENU_JOYSTICK    13
# define MENU_SETTINGS    14
# define MENU_RESET       15
# define MENU_BACK        16

# define MENU_EXIT        17

# define MAX_MENU_ITEM (MENU_EXIT + 1)

  static menu_item_t menu_list[] =
  { 
    { "Save Screenshot :" },

    { "Load Slot" },
    { "Save Slot" },
    { "Delete Slot" },

    { "Help" },

    { "Load Snapshot" },
    { "Load Disk A" },     
    { "Explore Disk A" },
    { "Load Disk B" },     
    { "Command " },

    { "Comments" },
    { "Cheats"   },
    { "Keyboard" }, 
    { "Joystick" },
    { "Settings" },

    { "Reset CPC" },
    { "Back to CPC" },
    { "Exit" }
  };

  static int  cur_menu_id = MENU_LOAD_SNA;
  static int  cur_slot    = 0;
  static int  cur_command = 0;

#define MAX_COMMAND 3

  static char* command_name[MAX_COMMAND] = {
    "RUN\"", "|CPM", "CAT"
  };

void 
psp_menu_display_save_name()
{
  char buffer[128];
  int Length;

  snprintf(buffer, 20, "Game: %s", CPC.cpc_save_name);
  Length = strlen(buffer);
  psp_sdl_back2_print(380 - (8*Length), 25, buffer, PSP_MENU_TEXT2_COLOR);
}

void
string_fill_with_space(char *buffer, int size)
{
  int length = strlen(buffer);
  int index;

  for (index = length; index < size; index++) {
    buffer[index] = ' ';
  }
  buffer[size] = 0;
}

void
psp_display_screen_menu_battery(void)
{
  char buffer[64];

  int Length;
  int color;

  snprintf(buffer, 50, " [%s] ", psp_get_battery_string());
  Length = strlen(buffer);

  if (psp_is_low_battery()) color = PSP_MENU_RED_COLOR;
  else                      color = PSP_MENU_GREEN_COLOR;

  psp_sdl_back2_print(240 - ((8*Length) / 2), 6, buffer, color);
}

void 
psp_display_screen_menu(void)
{
  char buffer[64];
  int menu_id = 0;
  int slot_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  //if (psp_menu_dirty) 
  {
    psp_sdl_blit_background();
    psp_menu_dirty = 0;

    psp_sdl_draw_rectangle(5,5,390,230,PSP_MENU_BORDER_COLOR,0);

    psp_sdl_back2_print( 30, 2, " 5: Settings ", PSP_MENU_NOTE_COLOR);

    psp_sdl_back2_print( 270, 2, " 6: Reset ", PSP_MENU_WARNING_COLOR);

    psp_sdl_back2_print(70, 230, " 2: Cancel  1: Valid  Back: Back ", 
                       PSP_MENU_BORDER_COLOR);

  }
  
  x      = 20;
  y      = 12;
  y_step = 10;
  
  for (menu_id = 0; menu_id < MAX_MENU_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;
    else 
    if (menu_id == MENU_EXIT) color = PSP_MENU_WARNING_COLOR;
    else
    if (menu_id == MENU_HELP) color = PSP_MENU_GREEN_COLOR;

    if (menu_id == MENU_COMMAND) {
      strcpy(buffer, menu_list[menu_id].title);
      strcat(buffer, command_name[cur_command]);
      string_fill_with_space(buffer, 25);
      psp_sdl_back2_print(x, y, buffer, color);
    } else {
      psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);
    }

    if (menu_id == MENU_SCREENSHOT) {
      sprintf(buffer,"%d", CPC.psp_screenshot_id);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(160, y, buffer, color);
      //y += y_step;
    } else
    if (menu_id == MENU_DEL_SLOT) {
      y += y_step;
    } else
    if (menu_id == MENU_LOAD_DSKB) {
      //y += y_step;
    } else
    if (menu_id == MENU_COMMAND) {
      y += y_step;
    } else
    if (menu_id == MENU_SETTINGS) {
      y += y_step;
    } else
    if (menu_id == MENU_BACK) {
    } else
    if (menu_id == MENU_CHEATS) {
    } else
    if (menu_id == MENU_HELP) {
      y += y_step;
    }

    y += y_step;
  }
  y_step = 10;
  y      = 22;

  for (slot_id = 0; slot_id < CPC_MAX_SAVE_STATE; slot_id++) {
    if (slot_id == cur_slot) color = PSP_MENU_SEL2_COLOR;
    else                     color = PSP_MENU_TEXT_COLOR;

    if (CPC.cpc_save_state[slot_id].used) {
      ScePspDateTime* my_date = &CPC.cpc_save_state[slot_id].date;
# ifndef LINUX_MODE
      sprintf(buffer, "- %02d/%02d %02d:%02d:%02d",
         my_date->day, my_date->month, 
         my_date->hour, my_date->minute, my_date->second );
# else
      sprintf(buffer, "- used");
# endif
    } else {
      sprintf(buffer, "- empty");
    }
    string_fill_with_space(buffer, 32);
    psp_sdl_back2_print(120, y, buffer, color);

    y += y_step;
  }

  if (CPC.cpc_save_state[cur_slot].thumb) {
    psp_sdl_blit_thumb(260, 40, CPC.cpc_save_state[cur_slot].surface);
  } else {
    psp_sdl_blit_thumb(260, 40, thumb_surface);
  }

  psp_menu_display_save_name();
}

static void
psp_main_menu_reset(void)
{
  /* Reset ! */
  psp_display_screen_menu();
  psp_sdl_back2_print(140, 160, "Reset CPC !", PSP_MENU_WARNING_COLOR);
  psp_menu_dirty = 1;
  psp_sdl_flip();
  emulator_reset(false);
  reset_save_name();
  sleep(1);
}

static void
psp_main_menu_run_command(void)
{
  char  Buffer[MAX_PATH];
  char *RunName;

  RunName = psp_run_search(CPC.cpc_save_name);
  if (RunName == (char *)0 ) RunName = CPC.cpc_save_name;
  if (!strcasecmp(RunName, "|CPM")) strcpy(Buffer, "|CPM");
  else  snprintf(Buffer, MAX_PATH, "RUN\"%s", RunName);

  psp_kbd_run_command(Buffer, 1);
}

static void
psp_main_menu_cur_command(int step)
{
  if (step > 0) {
    cur_command++; 
    if (cur_command >= MAX_COMMAND) cur_command = 0;
  } else
  if (step < 0) {
    cur_command--; 
    if (cur_command < 0) cur_command = MAX_COMMAND - 1;
  }
}

static void
psp_main_menu_command(void)
{
  if (cur_command == 0) {
    psp_main_menu_run_command();   
  } else {
    char buffer[128];
    strcpy(buffer, command_name[cur_command]);
    strcat(buffer, "\n");
    psp_kbd_run_command(buffer, 1);
  }
}
static int
psp_main_menu_load(int format, char drive)
{
  int ret;

  ret = psp_fmgr_menu(format, drive);
  if (ret ==  1) /* load OK */
  {
    psp_menu_dirty = 1;
    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "File loaded !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
    return 1;
  }
  else 
  if (ret == -1) /* Load Error */
  {
    psp_menu_dirty = 1;
    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "Can't load file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  return 0;
}

/* used by CAP32_xxx keys */
int
psp_main_menu_load_current()
{
  if (CPC.cpc_save_state[cur_slot].used) {
    return cap32_snapshot_load_slot(cur_slot);
  }
  return -1;
}

int
psp_main_menu_save_current()
{
  cap32_save_back_to_blit();
  return cap32_snapshot_save_slot(cur_slot);
}

int
psp_main_menu_load_slot()
{
  int error;

  if (! CPC.cpc_save_state[cur_slot].used) {

    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "Slot is empty !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);

    return 0;
  }

  error = cap32_snapshot_load_slot(cur_slot);

  if (! error) /* load OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "File loaded !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);

    return 1;
  }
  else 
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "Can't load file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  return 0;
}

static void
psp_main_menu_save_slot()
{
  int error;
  error = cap32_snapshot_save_slot(cur_slot);

  if (! error) /* save OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_main_menu_del_slot()
{
  int error;

  if (! CPC.cpc_save_state[cur_slot].used) {

    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "Slot is empty !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);

    return;
  }

  error = cap32_snapshot_del_slot(cur_slot);

  if (! error) /* save OK */
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "File deleted !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_menu();
    psp_sdl_back2_print(140, 160, "Can't delete file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_main_menu_cur_slot(int step)
{
  if (step == 1) {
    cur_slot++; if (cur_slot >= CPC_MAX_SAVE_STATE) cur_slot = 0;
  } else if (step == -1) {
    cur_slot--; if (cur_slot < 0) cur_slot = CPC_MAX_SAVE_STATE - 1;
  }
}

static void
psp_main_menu_screenshot(void)
{
  psp_screenshot_mode = 10;
}

int
psp_main_menu_exit(void)
{
  SceCtrlData c;

  psp_display_screen_menu();
  psp_sdl_back2_print(140, 160, "press 1 to confirm !", PSP_MENU_WARNING_COLOR);
  psp_menu_dirty = 1;
  psp_sdl_flip();

  psp_kbd_wait_no_button();

  do
  {
    sceCtrlReadBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (c.Buttons & PSP_CTRL_CROSS) psp_sdl_exit(0);

  } while (c.Buttons == 0);

  psp_kbd_wait_no_button();

  return 0;
}

void
psp_main_menu_editor()
{
  char TmpFileName[MAX_PATH];
  sprintf(TmpFileName, "%s/txt/%s.txt", CPC.cpc_homedir, CPC.cpc_save_name);
  psp_editor_menu( TmpFileName );
}

int 
psp_main_menu(void)
{
  SceCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  audio_pause();

  cap32_save_back_to_blit();

  psp_kbd_wait_no_button();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  psp_menu_dirty = 1;

  while (! end_menu)
  {

	while (1)
	{
		psp_display_screen_menu();
		psp_sdl_flip();

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
      psp_settings_menu();
      psp_menu_dirty = 1;
      old_pad = new_pad = 0;
    } else
    if ((c.Buttons & PSP_CTRL_RTRIGGER) == PSP_CTRL_RTRIGGER) {
      psp_main_menu_reset();
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
        case MENU_LOAD_SLOT : if (step) psp_main_menu_cur_slot(step);
                              else if (psp_main_menu_load_slot()) end_menu = 1;
        break;
        case MENU_SAVE_SLOT : if (step) psp_main_menu_cur_slot(step);
                              else      psp_main_menu_save_slot();
        break;
        case MENU_DEL_SLOT  : if (step) psp_main_menu_cur_slot(step);
                              else      psp_main_menu_del_slot();
        break;

        case MENU_LOAD_SNA  : psp_menu_dirty = 1;
                              if (psp_main_menu_load(FMGR_FORMAT_SNA, 0)) end_menu = 1;
                              old_pad = new_pad = 0;
                              psp_menu_dirty = 1;
        break;              
        case MENU_LOAD_DSKA : psp_menu_dirty = 1;
                              if (psp_main_menu_load(FMGR_FORMAT_DSK, 'A')) {
                                if (CPC.psp_explore_disk == CPC_EXPLORE_LOAD) {
                                  if (psp_disk_menu()) {
                                    end_menu = 1;
                                  }
                                } else 
                                if (CPC.psp_explore_disk >= CPC_EXPLORE_AUTO) {
                                  if (psp_disk_auto()) {
                                    end_menu = 1;
                                  }
                                }
                              }
                              psp_menu_dirty = 1;
                              old_pad = new_pad = 0;
        break;

        case MENU_LOAD_DSKB : psp_main_menu_load(FMGR_FORMAT_DSK, 'B');
                              psp_menu_dirty = 1;
                              old_pad = new_pad = 0;
        break;

        case MENU_EXPL_DSKA : psp_menu_dirty = 1;
                              if (psp_disk_menu()) {
                                end_menu = 1;
                              }
                              psp_menu_dirty = 1;
                              old_pad = new_pad = 0;
        break;

        case MENU_EDITOR    : psp_menu_dirty = 1;
                              psp_main_menu_editor();
                              psp_menu_dirty = 1;
                              old_pad = new_pad = 0;
        break;

        case MENU_CHEATS    : psp_menu_dirty = 1;
                              psp_cheat_menu();
                              psp_menu_dirty = 1;
                              old_pad = new_pad = 0;
        break;

        case MENU_COMMAND   : if (step) psp_main_menu_cur_command(step);
                              else { 
                                 psp_main_menu_command();
                                 end_menu = 1;
                              }
        break;

        case MENU_KEYBOARD   : psp_keyboard_menu();
                               psp_menu_dirty = 1;
                               old_pad = new_pad = 0;
        break;
        case MENU_JOYSTICK   : psp_joystick_menu();
                               psp_menu_dirty = 1;
                               old_pad = new_pad = 0;
        break;
        case MENU_SETTINGS   : psp_menu_dirty = 1;
                               psp_settings_menu();
                               psp_menu_dirty = 1;
                               old_pad = new_pad = 0;
        break;

        case MENU_SCREENSHOT : psp_main_menu_screenshot();
                               end_menu = 1;
        break;              
        case MENU_RESET      : psp_main_menu_reset();
                               end_menu = 1;
        break;

        case MENU_BACK      : end_menu = 1;
        break;

        case MENU_EXIT      : psp_main_menu_exit();
        break;

        case MENU_HELP      : psp_menu_dirty = 1;
                              psp_help_menu();
                              old_pad = new_pad = 0;
                              psp_menu_dirty = 1;
        break;              
      }

    } else
    if(new_pad & PSP_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_ITEM-1;

    } else
    if(new_pad & PSP_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_ITEM-1)) cur_menu_id++;
      else                                 cur_menu_id = 0;

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
 
  psp_kbd_wait_no_button();

  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();
  psp_sdl_clear_screen( PSP_MENU_BLACK_COLOR );
  psp_sdl_flip();

  psp_sdl_clear_blit(0);

#ifndef LINUX_MODE
  psp_sdl_gu_init();
#endif

  audio_resume();

  return 1;
}

