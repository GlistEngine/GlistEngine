/*
 * gBaseSound.cpp
 *
 *  Created on: 18 Kas 2020
 *      Author: Acer
 */

#include "gBaseSound.h"

gBaseSound::gBaseSound() {
	filepath = "";
	looptype = LOOPTYPE_DEFAULT;
	isloaded = false;
	isplaying = false;
	ispaused = false;
	position = 0;
	duration = false;
	volume = 0.5f;
}

gBaseSound::~gBaseSound() {
}

int gBaseSound::load(std::string fullPath) {
	filepath = fullPath;
	return 0;
}


int gBaseSound::loadSound(std::string soundPath) {
	filepath = soundPath;
	return 0;
}

void gBaseSound::play() {

}

bool gBaseSound::isLoaded() {
	return isloaded;
}

bool gBaseSound::isPlaying() {
	return isplaying;
}

void gBaseSound::setPaused(bool isPaused) {
	ispaused = isPaused;
}

bool gBaseSound::isPaused() {
	return ispaused;
}

void gBaseSound::stop() {

}

void gBaseSound::close() {

}

int gBaseSound::getDuration() {
	return duration;
}

void gBaseSound::setPosition(int position) {
	this->position = position;
}

int gBaseSound::getPosition() {
	return position;
}

void gBaseSound::setLoopType(int loopType) {
	looptype = loopType;
}

int gBaseSound::getLoopType() {
	return looptype;
}

void gBaseSound::setVolume(float volume) {
	this->volume = volume;
}

float gBaseSound::getVolume() {
	return volume;
}

std::string gBaseSound::getPath() {
	return filepath;
}

