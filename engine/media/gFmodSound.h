/*
 * gFmodSound.h
 *
 *  Created on: 18 Kas 2020
 *      Author: Acer
 */

#ifndef MEDIA_GFMODSOUND_H_
#define MEDIA_GFMODSOUND_H_

#include "gBaseSound.h"
#include "fmod.h"
#include "fmod_common.h"
#include "gObject.h"

class gFmodSound: public gBaseSound {
public:
	gFmodSound();
	~gFmodSound();

	int loadSound(std::string soundPath);
	int load(std::string fullPath);
	void play();
	void setPaused(bool isPaused);
	void stop();
	void close();

	void setLoopType(int loopType);

	int getDuration();
	void setPosition(int position);
	int getPosition();

	/**
	 * Please note that, one may change the volume after the sound starts playing
	 */
	void setVolume(float volume);

private:
	FMOD_SYSTEM *system;
	FMOD_SOUND *sound1;
    FMOD_CHANNEL *channel = 0;
    FMOD_RESULT result;
    unsigned int version;
    void *extradriverdata = 0;
};


#endif /* MEDIA_GFMODSOUND_H_ */
