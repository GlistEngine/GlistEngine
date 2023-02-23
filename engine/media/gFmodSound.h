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

/**
 * gFmodSound, contains functions for sound playing using Fmod library.
 *
 * Uses gBaseSound as Parent
 */

class gFmodSound: public gBaseSound {
public:
	gFmodSound();
	~gFmodSound();

	/**
	 * Sets the sound data by loading the sound file from the provided path.
	 *
	 * @param soundPath the local project path for the sound file.
	 *
	 * @return returns generated id of the sound file.
	 */
	int loadSound(const std::string& soundPath);

	/**
	 * Sets the sound data by loading the sound file from the provided path.
	 *
	 * @param fullPath the local computer path for the sound file.
	 *
	 * @return returns generated id of the sound file.
	 */
	int load(const std::string& fullPath);

	/**
	 * Plays the sound file.
	 */
	void play();

	/**
	 * Pauses the sound file that is currently playing also resumes it.
	 *
	 * @param isPaused checks and returns bool variable whether the sound is stopped or not.
	 */
	void setPaused(bool isPaused);

	/**
	 * Stops the currently playing sound file.
	 */
	void stop();

	/**
	 * Closes the currently playing sound file.
	 */
	void close();

	/**
	 * Returns bool variable whether the sound file is playing or not.
	 */
	bool isPlaying();

	/**
	 * The loop type.
	 *
	 * @param loopType parameter for the loop type.
	 */
	void setLoopType(int loopType);

	/**
	 * Returns the duration of the sound file.
	 *
	 * @return The duration.
	 */
	int getDuration() const;

	/**
	 * The position in time for the playing sound file.
	 *
	 * @param position Parameter for position.
	 */
	void setPosition(int position);

	/**
	 * Returns the position in time for the playing sound file.
	 * Please note that, one may change the position after the sound starts playing.
	 *
	 * @return the position in time.
	 */
	int getPosition();

	/**
	 * Sets the volume for the playing sound file.
	 * Please note that, one may change the volume after the sound starts playing
	 *
	 * @param volume Parameter for volume..
	 */
	void setVolume(float volume);

private:
	FMOD_SYSTEM *system;
	FMOD_SOUND *sound1;
    FMOD_CHANNEL *channel = 0;
    FMOD_RESULT result;
    unsigned int version;
    void *extradriverdata = 0;
    int ip;
};


#endif /* MEDIA_GFMODSOUND_H_ */
