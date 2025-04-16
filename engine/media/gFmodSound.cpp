/*
 * gFmodSound.cpp
 *
 *  Created on: 16 Nis 2025
 *      Author: Admin
 */

#include "gFmodSound.h"

gFmodSound::gFmodSound() {
	// TODO Auto-generated constructor stub
	std::cout << "gFmodSound is deprecated. Use gSound instead.\n";
}

gFmodSound::~gFmodSound() {
	// TODO Auto-generated destructor stub
}

void gFmodSound::play() {
    sound.play();
}

void gFmodSound::stop() {
    sound.stop();
}

int gFmodSound::load(const std::string &fullPath) {
	return sound.load(fullPath);
}

int gFmodSound::loadSound(const std::string &soundPath) {
	return sound.loadSound(soundPath);
}

void gFmodSound::setVolume(float volume) {
	sound.setVolume(volume);
}

bool gFmodSound::isPlaying() {
	return sound.isPlaying();
}

bool gFmodSound::isLoaded() {
	return sound.isLoaded();
}

bool gFmodSound::isPaused() {
	return sound.isPaused();
}

void gFmodSound::setPaused(bool isPaused) {
	sound.setPaused(isPaused);
}

void gFmodSound::close() {
	sound.close();
}

int gFmodSound::getDuration() {
	return sound.getDuration();
}

int gFmodSound::getPosition() {
	return sound.getPosition();
}

void gFmodSound::setPosition(int position) {
	sound.setPosition(position);
}

void gFmodSound::setLoopType(LoopType loopType) {
	sound.setLoopType(loopType);
}

float gFmodSound::getVolume() {
	return sound.getVolume();
}

const std::string& gFmodSound::getPath() {
	return sound.getPath();
}
