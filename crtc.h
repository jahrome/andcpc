/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2004 Ulrich Doewich

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef CRTC_H
#define CRTC_H

#include "z80.h"

void access_video_memory(int repeat_count);
# if 0 //LUDO:
void draw16bpp_mode0_half(u16 addr);
void draw16bpp_mode1_half(u16 addr);
void draw16bpp_mode2_half(u16 addr);
# endif
void render16bpp_smooth();
# if 0
void draw8bpp_border_half(void);
void draw8bpp_mode0_half(u16 addr);
void draw8bpp_mode1_half(u16 addr);
void draw8bpp_mode2_half(u16 addr);
void render8bpp_smooth();
# endif
#endif
