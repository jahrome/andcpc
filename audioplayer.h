/*
   Copyright (C) 2010 Jerome Marty (jahrome11@gmail.com)

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

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

class AudioPlayer {
public:
	virtual bool init(unsigned int rate, int bits, int channels) = 0;
	virtual void destroy() = 0;

	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void pause() = 0;
	virtual void play(void *data, int size) = 0;
	virtual ~AudioPlayer() {};
};
extern "C" __attribute__((visibility("default")))
AudioPlayer *createPlayer();

#endif

