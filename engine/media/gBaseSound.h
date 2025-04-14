/*
 * gBaseSound.h
 *
 *  Created on: 18 Kas 2020
 *      Author: Noyan Culum
 */

#ifndef MEDIA_GBASESOUND_H_
#define MEDIA_GBASESOUND_H_

#include <string>
#include "gObject.h"

// gBaseSound is an abstract interface class for all sounds.
class gBaseSound : public gObject {
public:
    enum LoopType : int {
        LOOPTYPE_DEFAULT = 0,
        LOOPTYPE_NONE = 1,
        LOOPTYPE_NORMAL = 2
    };

	virtual int load(const std::string& fullPath) = 0;
	virtual int loadSound(const std::string& soundPath) = 0;
	virtual void play() = 0;
	virtual bool isLoaded() = 0;
	virtual bool isPlaying() = 0;
	virtual void setPaused(bool isPaused) = 0;
	virtual bool isPaused() = 0;
	virtual void stop() = 0;
	virtual void close() = 0;

	virtual int getDuration() = 0;
	virtual void setPosition(int position) = 0;
	virtual int getPosition() = 0;

	virtual void setLoopType(LoopType loopType) = 0;
	virtual LoopType getLoopType() = 0;

	virtual void setVolume(float volume) = 0;
	virtual float getVolume() = 0;

	virtual const std::string& getPath() = 0;

};

#endif /* MEDIA_GBASESOUND_H_ */
