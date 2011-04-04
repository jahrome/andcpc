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

# ifdef PSP_AUDIO

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <pspaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cap32.h"

 static int psp_audio_chid = -1;
 static int psp_audio_thid = -1;
 static int psp_audio_term = 0;

    extern volatile unsigned char *pbSndStream;

 static volatile int psp_audio_sleep = 0;
 static int psp_audio_empty[ 2048 ];

void
psp_audio_pause()
{
  psp_audio_sleep = 1;
}

void
psp_audio_resume()
{
  psp_audio_sleep = 0;
}


void
psp_audio_thread(int args, void *argp)
{
  int* snd_buff = 0;
	while (psp_audio_term == 0) {
    snd_buff = pbSndStream;
    if ((! snd_buff) || psp_audio_sleep || (! CPC.snd_enabled)) {
      snd_buff = psp_audio_empty;
    }
    //snd_buff = PSP_NORMAL_PTR( snd_buff );
    sceAudioOutputPannedBlocking(psp_audio_chid, PSP_AUDIO_VOLUME_MAX, PSP_AUDIO_VOLUME_MAX, snd_buff);
	}
	sceKernelExitThread(0);
}

void
psp_audio_init()
{
  psp_audio_chid = sceAudioChReserve(-1, 1024, 0);
  psp_audio_thid = sceKernelCreateThread( "psp_audio_thread",(void*)&psp_audio_thread,0x14,0x10000,0,NULL);
	sceKernelStartThread( psp_audio_thid, 0, 0);
}

void
psp_audio_end()
{
  psp_audio_term = 1;
  if (psp_audio_thid != -1) {
		sceKernelDeleteThread( psp_audio_thid );
    psp_audio_thid = -1;
  }
  if (psp_audio_chid != -1) {
    sceAudioChRelease( psp_audio_chid );
    psp_audio_chid = -1;
  }
}

# endif
