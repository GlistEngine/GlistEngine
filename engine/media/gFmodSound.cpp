/*
 * gFmodSound.cpp
 *
 *  Created on: 18 Kas 2020
 *      Author: Acer
 */

#include "gFmodSound.h"

gFmodSound::gFmodSound() {
	result = FMOD_OK;
	system = nullptr;
	sound1 = nullptr;
	version = 0;
}

gFmodSound::~gFmodSound() {
}

int gFmodSound::load(std::string fullPath) {
//    Common_Init(&extradriverdata);

    /*
        Create a System object and initialize
    */
    result = FMOD_System_Create(&system);
//    ERRCHECK(result);

    result = FMOD_System_GetVersion(system, &version);
//    ERRCHECK(result);

    if (version < FMOD_VERSION)
    {
//        Common_Fatal("FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION);
    }


    result = FMOD_System_Init(system, 32, FMOD_INIT_NORMAL, extradriverdata);
//    ERRCHECK(result);

    result = FMOD_System_CreateSound(system, fullPath.c_str(), FMOD_DEFAULT, 0, &sound1);
//    ERRCHECK(result);

    result = FMOD_Sound_GetLength(sound1, &duration, FMOD_TIMEUNIT_MS);

    result = FMOD_Sound_SetMode(sound1, LOOPTYPE_DEFAULT);    /* drumloop.wav has embedded loop points which automatically makes looping turn on, */
//    ERRCHECK(result);                           /* so turn it off here.  We could have also just put FMOD_LOOP_OFF in the above CreateSound call. */

    //result = FMOD_Sound_GetName(sound1, fullPath.c_str(), namelen);
    isloaded = true;

    return 1;
}

int gFmodSound::loadSound(std::string soundPath) {
	return load(gGetSoundsDir() + soundPath);
}

void gFmodSound::play() {
    result = FMOD_System_PlaySound(system, sound1, 0, false, &channel);
    isplaying = true;
//    ERRCHECK(result);
}

void gFmodSound::setPaused(bool isPaused) {
	gBaseSound::setPaused(isPaused);
	result = FMOD_Channel_SetPaused(channel, isPaused);
}

void gFmodSound::stop() {
	FMOD_Channel_Stop(channel);
	isplaying = false;
}

void gFmodSound::close() {
	FMOD_System_Close(system);
	FMOD_System_Release(system);
	isplaying = false;
	ispaused = false;
	isloaded = false;
}

bool gFmodSound::isPlaying() {
	FMOD_Channel_IsPlaying(channel, &ip);
	isplaying = ip;
	return isplaying;
}

int gFmodSound::getDuration() {
	return duration;
}

void gFmodSound::setPosition(int position) {
	gBaseSound::setPosition(position);
	FMOD_Channel_SetPosition(channel, position, FMOD_TIMEUNIT_MS);
}

int gFmodSound::getPosition() {
	FMOD_Channel_GetPosition(channel, &position, FMOD_TIMEUNIT_MS);
	return position;
}

void gFmodSound::setLoopType(int loopType) {
	gBaseSound::setLoopType(loopType);
	FMOD_Sound_SetMode(sound1, loopType);
}

void gFmodSound::setVolume(float volume) {
	gBaseSound::setVolume(volume);
	FMOD_Channel_SetVolume(channel, volume);
}


