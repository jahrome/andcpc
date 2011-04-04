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


#include <media/AudioTrack.h>
#include "audioplayer.h"
#include <utils/Log.h>

#define LOG_TAG "AndCPC"

using namespace android;

#ifdef __cplusplus
class AudioPlayerImpl : public AudioPlayer {
public:
	AudioPlayerImpl();
	virtual ~AudioPlayerImpl();

	virtual bool init(unsigned int rate, int bits, int channels);
	virtual void destroy();
	virtual void start();
	virtual void stop();
	virtual void pause();
	virtual void play(void *data, int size);

private:
	AudioTrack *track;
};
#else
	typedef struct AudioPlayerImpl AudioPlayerImpl;
#endif

AudioPlayerImpl::AudioPlayerImpl()
		: track(NULL)
{
}

AudioPlayerImpl::~AudioPlayerImpl()
{
	if (track != NULL)
		delete track;
}

bool AudioPlayerImpl::init(unsigned int rate, int bits, int channels)
{
	LOGD("Init %d, %d, %d", rate, bits, channels);

	int format = (bits == 16 ?
			AudioSystem::PCM_16_BIT : AudioSystem::PCM_8_BIT);

	if (track != NULL) {
		if (track->getSampleRate() == rate &&
			track->format() == format &&
			track->channelCount() == channels)
			return true;
		delete track;
	}

	if (channels == 1)
		track = new AudioTrack(AudioSystem::MUSIC, rate, format, AudioSystem::CHANNEL_OUT_FRONT_LEFT, 0);
	else
		track = new AudioTrack(AudioSystem::MUSIC, rate, format, AudioSystem::CHANNEL_OUT_FRONT_LEFT|AudioSystem::CHANNEL_OUT_FRONT_RIGHT, 0);
	if (track->initCheck() < 0) {
		delete track;
		track = NULL;
		return false;
	}
	return true;
}

void AudioPlayerImpl::destroy()
{
	delete this;
}

void AudioPlayerImpl::start()
{
	if (track != NULL)
		track->start();
}

void AudioPlayerImpl::stop()
{
	if (track != NULL) {
		track->stop();
		track->flush();
	}
}

void AudioPlayerImpl::pause()
{
	if (track != NULL)
		track->pause();
}

void AudioPlayerImpl::play(void *data, int size)
{
	if (track != NULL)
		track->write(data, size);
}


AudioPlayer *createPlayer()
{
	return new AudioPlayerImpl;
}
void audio_player_play(AudioPlayer* audioPlayer, void *data, int size)
{
	audioPlayer->play(data, size);
}
void audio_player_init(AudioPlayer* audioPlayer, unsigned int rate, int bits, int channels)
{
	audioPlayer->init(rate, bits, channels);
}
void audio_player_stop(AudioPlayer* audioPlayer)
{
	audioPlayer->stop();
}
void audio_player_destroy(AudioPlayer* audioPlayer)
{
	audioPlayer->destroy();
}
void audio_player_pause(AudioPlayer* audioPlayer)
{
	audioPlayer->pause();
}
void audio_player_start(AudioPlayer* audioPlayer)
{
	audioPlayer->start();
}
