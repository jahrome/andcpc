/*
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
#include <string.h>
#include <math.h>
#include <pspctrl.h>
#include <psptypes.h>
//#include <pspsircs.h>
#include <pspiofilemgr.h>
#include <pspthreadman.h>
//#include <pspmodulemgr.h>
#include <png.h>

#include "cap32.h"
#include "psp_irda.h"
#include "psp_joy.h"
#include "psp_kbd.h"

  static char loc_joy_name[ JOY_ALL_BUTTONS ][10] =
  {
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
    "FIRE",
    "PADDLE1P",
    "PADDLE1M",
    "PADDLE1F",
    "PADDLE2P",
    "PADDLE2M",
    "PADDLE2F",
  };

 int psp_joy_mapping[ JOY_ALL_BUTTONS ];

 static int joy_default_mapping[ JOY_ALL_BUTTONS ] = {
    KBD_JOY_UP       , /*  JOY_UP         */
    KBD_JOY_DOWN     , /*  JOY_DOWN       */
    KBD_JOY_LEFT     , /*  JOY_LEFT       */
    KBD_JOY_RIGHT    , /*  JOY_RIGHT      */
    KBD_CROSS        , /*  JOY_FIRE       */
    KBD_JOY_RIGHT    , /*  JOY_PADDLE1P   */
    KBD_JOY_LEFT     , /*  JOY_PADDLE1M   */
    KBD_CROSS        , /*  JOY_PADDLE1F   */
    KBD_JOY_UP       , /*  JOY_PADDLE2P   */
    KBD_JOY_DOWN     , /*  JOY_PADDLE2M   */
    KBD_CIRCLE       , /*  JOY_PADDLE2F   */
 };

static void
psp_convert_joy_to_psp_ctrl( int psp_key, SceCtrlData* psp_c )
{
  if (psp_key < 0) return;

  if (psp_key < KBD_MAX_BUTTONS) {
    psp_c->Buttons |= kbd_button_mask[ psp_key ];
  } else 
  if (psp_key < KBD_ALL_BUTTONS) {
    if (psp_key == KBD_JOY_LEFT ) psp_c->Lx = 0;
    else
    if (psp_key == KBD_JOY_RIGHT) psp_c->Lx = 255;
    else
    if (psp_key == KBD_JOY_UP   ) psp_c->Ly = 0;
    else
    if (psp_key == KBD_JOY_DOWN ) psp_c->Ly = 255;
  }
}

void
psp_joy_convert_joystick( irda_joy_t* i_joy, SceCtrlData* psp_c )
{
  int psp_key;
  if (i_joy->atariJ & JOY_ATARI_UP_MASK) {
    psp_key = psp_joy_mapping[ JOY_UP ];
    psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
  }
  if (i_joy->atariJ & JOY_ATARI_DOWN_MASK  ) {
    psp_key = psp_joy_mapping[ JOY_DOWN ];
    psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
  }
  if (i_joy->atariJ & JOY_ATARI_LEFT_MASK  ) {
    psp_key = psp_joy_mapping[ JOY_LEFT ];
    psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
  }
  if (i_joy->atariJ & JOY_ATARI_RIGHT_MASK ) {
    psp_key = psp_joy_mapping[ JOY_RIGHT ];
    psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
  }
  if (i_joy->atariJ & JOY_ATARI_BUTTON_MASK) {
    psp_key = psp_joy_mapping[ JOY_FIRE ];
    psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
  }
}

void
psp_joy_convert_paddle( irda_joy_t* i_joy, SceCtrlData* psp_c )
{
  int psp_key;
  if (i_joy->atariJ & JOY_ATARI_LEFT_MASK) {
    psp_key = psp_joy_mapping[ JOY_PADDLE1F ];
    psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
  }
  psp_key = psp_joy_mapping[ JOY_PADDLE1P ];
  if (psp_key >= 0) {
    if (psp_key < KBD_MAX_BUTTONS) {
      /* Emulate button press */
      if (i_joy->paddleA > 0x240) psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
    } else 
    if (i_joy->paddleA >= 0x1ff) {
      /* Emulate analog joystick */
      if (psp_key == KBD_JOY_RIGHT) psp_c->Lx = i_joy->paddleA >> 2;
      else
      if (psp_key == KBD_JOY_LEFT ) psp_c->Lx = (0x3ff - i_joy->paddleA) >> 2;
      else
      if (psp_key == KBD_JOY_UP   ) psp_c->Ly = i_joy->paddleA >> 2;
      else
      if (psp_key == KBD_JOY_DOWN ) psp_c->Ly = (0x3ff - i_joy->paddleA) >> 2;
    }
  }
  psp_key = psp_joy_mapping[ JOY_PADDLE1M ];
  if (psp_key >= 0) {
    if (psp_key < KBD_MAX_BUTTONS) {
      /* Emulate button press */
      if (i_joy->paddleA < 0x160) psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
    } else 
    if (i_joy->paddleA < 0x1ff) {
      /* Emulate analog joystick */
      if (psp_key == KBD_JOY_LEFT ) psp_c->Lx = i_joy->paddleA >> 2;
      else
      if (psp_key == KBD_JOY_RIGHT) psp_c->Lx = (0x3ff - i_joy->paddleA) >> 2;
      else
      if (psp_key == KBD_JOY_DOWN ) psp_c->Ly = i_joy->paddleA >> 2;
      else
      if (psp_key == KBD_JOY_UP   ) psp_c->Ly = (0x3ff - i_joy->paddleA) >> 2;
    }
  }
}

void
psp_joy_convert_dual_paddle( irda_joy_t* i_joy, SceCtrlData* psp_c )
{
  int psp_key;

  /* Paddle A */
  if (i_joy->atariJ & JOY_ATARI_LEFT_MASK) {
    psp_key = psp_joy_mapping[ JOY_PADDLE1F ];
    psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
  }
  psp_key = psp_joy_mapping[ JOY_PADDLE1P ];
  if (psp_key >= 0) {
    if (psp_key < KBD_MAX_BUTTONS) {
      /* Emulate button press */
      if (i_joy->paddleA > 0x240) psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
    } else 
    if (i_joy->paddleA >= 0x1ff) {
      /* Emulate analog joystick */
      if (psp_key == KBD_JOY_RIGHT) psp_c->Lx = i_joy->paddleA >> 2;
      else
      if (psp_key == KBD_JOY_LEFT ) psp_c->Lx = (0x3ff - i_joy->paddleA) >> 2;
      else
      if (psp_key == KBD_JOY_UP   ) psp_c->Ly = i_joy->paddleA >> 2;
      else
      if (psp_key == KBD_JOY_DOWN ) psp_c->Ly = (0x3ff - i_joy->paddleA) >> 2;
    }
  }
  psp_key = psp_joy_mapping[ JOY_PADDLE1M ];
  if (psp_key >= 0) {
    if (psp_key < KBD_MAX_BUTTONS) {
      /* Emulate button press */
      if (i_joy->paddleA < 0x160) psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
    } else 
    if (i_joy->paddleA < 0x1ff) {
      /* Emulate analog joystick */
      if (psp_key == KBD_JOY_LEFT ) psp_c->Lx = i_joy->paddleA >> 2;
      else
      if (psp_key == KBD_JOY_RIGHT) psp_c->Lx = (0x3ff - i_joy->paddleA) >> 2;
      else
      if (psp_key == KBD_JOY_DOWN ) psp_c->Ly = i_joy->paddleA >> 2;
      else
      if (psp_key == KBD_JOY_UP   ) psp_c->Ly = (0x3ff - i_joy->paddleA) >> 2;
    }
  }

  /* Paddle B */
  if (i_joy->atariJ & JOY_ATARI_RIGHT_MASK) {
    psp_key = psp_joy_mapping[ JOY_PADDLE2F ];
    psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
  }
  psp_key = psp_joy_mapping[ JOY_PADDLE2P ];
  if (psp_key >= 0) {
    if (psp_key < KBD_MAX_BUTTONS) {
      /* Emulate button press */
      if (i_joy->paddleB > 0x240) psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
    } else 
    if (i_joy->paddleB >= 0x1ff) {
      /* Emulate analog joystick */
      if (psp_key == KBD_JOY_RIGHT) psp_c->Lx = i_joy->paddleB >> 2;
      else
      if (psp_key == KBD_JOY_LEFT ) psp_c->Lx = (0x3ff - i_joy->paddleB) >> 2;
      else
      if (psp_key == KBD_JOY_UP   ) psp_c->Ly = i_joy->paddleB >> 2;
      else
      if (psp_key == KBD_JOY_DOWN ) psp_c->Ly = (0x3ff - i_joy->paddleB) >> 2;
    }
  }
  psp_key = psp_joy_mapping[ JOY_PADDLE2M ];
  if (psp_key >= 0) {
    if (psp_key < KBD_MAX_BUTTONS) {
      /* Emulate button press */
      if (i_joy->paddleB < 0x160) psp_convert_joy_to_psp_ctrl( psp_key, psp_c );
    } else 
    if (i_joy->paddleB < 0x1ff) {
      /* Emulate analog joystick */
      if (psp_key == KBD_JOY_LEFT ) psp_c->Lx = i_joy->paddleB >> 2;
      else
      if (psp_key == KBD_JOY_RIGHT) psp_c->Lx = (0x3ff - i_joy->paddleB) >> 2;
      else
      if (psp_key == KBD_JOY_DOWN ) psp_c->Ly = i_joy->paddleB >> 2;
      else
      if (psp_key == KBD_JOY_UP   ) psp_c->Ly = (0x3ff - i_joy->paddleB) >> 2;
    }
  }
}

int
psp_joy_load_settings(char *joy_filename)
{
  FILE    *JoyFile;
  int      error = 0;
  
  JoyFile = fopen(joy_filename, "r");
  error   = 1;

  if (JoyFile != (FILE*)0) {
    psp_joy_load_settings_file(JoyFile);
    error = 0;
    fclose(JoyFile);
  }

  return error;
}

void
psp_joy_default_settings()
{
  CPC.cpc_auto_fire         = 0;
  CPC.cpc_auto_fire_period  = 6;
  CPC.cpc_auto_fire_pressed = 0;
  CPC.psp_reverse_analog    = 0;
  CPC.psp_irdajoy_type      = 0;
  CPC.psp_irdajoy_debug     = 0;

  memcpy(psp_joy_mapping, joy_default_mapping, sizeof(joy_default_mapping));
}

int
psp_joy_load_settings_file(FILE *JoyFile)
{
  char     Buffer[512];
  char    *Scan;
  int      tmp_mapping[JOY_ALL_BUTTONS];
  int      psp_key_id = 0;
  int      Value = 0;
  int      joy_id = 0;

  memcpy(tmp_mapping, joy_default_mapping, sizeof(joy_default_mapping));

  while (fgets(Buffer,512,JoyFile) != (char *)0) {
      
    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;
    
    *Scan = '\0';
    Value = atoi(Scan + 1);

    if (!strcasecmp(Buffer,"psp_reverse_analog")) CPC.psp_reverse_analog = Value;
    else
    if (!strcasecmp(Buffer,"psp_irdajoy_type")) CPC.psp_irdajoy_type = Value;
    else
    if (!strcasecmp(Buffer,"psp_irdajoy_debug")) CPC.psp_irdajoy_debug = Value;
    else
    if (!strcasecmp(Buffer,"cpc_auto_fire_period")) CPC.cpc_auto_fire_period = Value;
    else {

      psp_key_id = Value;

      for (joy_id = 0; joy_id < JOY_ALL_BUTTONS; joy_id++) {
        if (!strcasecmp(Buffer,loc_joy_name[joy_id])) {
          tmp_mapping[joy_id] = psp_key_id;
          //break;
        }
      }
    }
  }

  memcpy(psp_joy_mapping, tmp_mapping, sizeof(psp_joy_mapping));
  
  return 0;
}

int
psp_joy_save_settings(char *joy_filename)
{
  FILE    *JoyFile;
  int      joy_id = 0;
  int      error = 0;

  JoyFile = fopen(joy_filename, "w");
  error   = 1;

  if (JoyFile != (FILE*)0) {

    for (joy_id = 0; joy_id < JOY_ALL_BUTTONS; joy_id++)
    {
      fprintf(JoyFile, "%s=%d\n", loc_joy_name[joy_id], psp_joy_mapping[joy_id]);
    }
    fprintf( JoyFile, "psp_reverse_analog=%d\n"   , CPC.psp_reverse_analog);
    fprintf( JoyFile, "psp_irdajoy_type=%d\n"     , CPC.psp_irdajoy_type);
    fprintf( JoyFile, "psp_irdajoy_debug=%d\n"    , CPC.psp_irdajoy_debug);
    fprintf( JoyFile, "cpc_auto_fire_period=%d\n" , CPC.cpc_auto_fire_period);

    error = 0;
    fclose(JoyFile);
  }

  return error;
}

