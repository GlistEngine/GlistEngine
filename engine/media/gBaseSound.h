/*
 * gBaseSound.h
 *
 *  Created on: 18 Kas 2020
 *      Author: Acer
 */

#ifndef MEDIA_GBASESOUND_H_
#define MEDIA_GBASESOUND_H_

#include <string>
#include "gObject.h"


class gBaseSound : public gObject {
public:
	static const int LOOPTYPE_DEFAULT = 0, LOOPTYPE_NONE = 1, LOOPTYPE_NORMAL = 2;

	gBaseSound();
	virtual ~gBaseSound();

	virtual int load(const std::string& fullPath);
	virtual int loadSound(const std::string& soundPath);
	virtual void play();
	virtual bool isLoaded();
	virtual bool isPlaying();
	virtual void setPaused(bool isPaused);
	virtual bool isPaused();
	virtual void stop();
	virtual void close();

	virtual int getDuration();
	virtual void setPosition(int position);
	virtual int getPosition();

	virtual void setLoopType(int loopType);
	virtual int getLoopType();

	virtual void setVolume(float volume);
	virtual float getVolume();

	const std::string& getPath();


protected:
	bool isplaying, isloaded;
	unsigned int position;
	unsigned int duration;
	bool ispaused;

private:
	std::string filepath;
	int looptype;
	float volume;
};

#endif /* MEDIA_GBASESOUND_H_ */
