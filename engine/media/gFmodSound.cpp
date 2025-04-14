/*
 * gFmodSound.cpp
 *
 *  Created on: 18 Kas 2020
 *      Author: Noyan Culum
 */

#include "gFmodSound.h"

gFmodSound::gFmodSound() {
    filepath = "";
    looptype = LOOPTYPE_DEFAULT;
    isloaded = false;
    isplaying = false;
    ispaused = false;
    position = 0;
    duration = false;
    volume = 0.5f;

	result = FMOD_OK;
	system = nullptr;
	sound1 = nullptr;
	version = 0;
}

gFmodSound::~gFmodSound() {
}

int gFmodSound::load(const std::string& fullPath) {
//    Common_Init(&extradriverdata);

    /*
        Create a System object and initialize
    */
    result = FMOD_System_Create(&system, FMOD_VERSION);
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

int gFmodSound::loadSound(const std::string& soundPath) {
	return load(gGetSoundsDir() + soundPath);
}

void gFmodSound::play() {
    result = FMOD_System_PlaySound(system, sound1, 0, false, &channel);
	FMOD_Channel_SetVolume(channel, getVolume());
    isplaying = true;
//    ERRCHECK(result);
}

void gFmodSound::setPaused(bool isPaused) {
    this->ispaused = isPaused;
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
    FMOD_BOOL temp;
	FMOD_Channel_IsPlaying(channel, &temp);
	isplaying = temp;
	return isplaying;
}

void gFmodSound::setPosition(int position) {
    this->position = position;
	FMOD_Channel_SetPosition(channel, position, FMOD_TIMEUNIT_MS);
}

int gFmodSound::getPosition() {
	FMOD_Channel_GetPosition(channel, &position, FMOD_TIMEUNIT_MS);
	return position;
}

void gFmodSound::setLoopType(LoopType loopType) {
    this->looptype = loopType;
	FMOD_Sound_SetMode(sound1, loopType);
}

void gFmodSound::setVolume(float volume) {
    this->volume = volume;
	if(isPlaying()) {
		FMOD_Channel_SetVolume(channel, volume);
	}
}

int gFmodSound::getDuration() {
    return duration;
}

bool gFmodSound::isPaused() {
    return ispaused;
}

gFmodSound::LoopType gFmodSound::getLoopType() {
    return looptype;
}

float gFmodSound::getVolume() {
    return volume;
}

const std::string& gFmodSound::getPath() {
    return filepath;
}

