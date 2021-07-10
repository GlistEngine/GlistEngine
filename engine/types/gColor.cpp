/*
 * gColor.cpp
 *
 *  Created on: May 9, 2020
 *      Author: noyan
 */

#include "gColor.h"

const gColor gColor::RED 	 	(1.0f,	0.0f,	0.0f,	1.0f);
const gColor gColor::GREEN 		(0.0f,	1.0f, 	0.0f,	1.0f);
const gColor gColor::BLUE 	 	(0.0f, 	0.0f,	1.0f, 	1.0f);
const gColor gColor::CYAN 	 	(0.0f, 	1.0f, 	1.0f, 	1.0f);
const gColor gColor::MAGENTA  	(1.0f, 	0.0f, 	1.0f, 	1.0f);
const gColor gColor::YELLOW 	(1.0f, 	1.0f, 	0.0f,	1.0f);
const gColor gColor::BLACK 	 	(0.0f,	0.0f,	0.0f,	1.0f);
const gColor gColor::WHITE 	 	(1.0f, 	1.0f, 	1.0f, 	1.0f);
const gColor gColor::GRAY 	 	(0.5f, 	0.5f, 	0.5f, 	1.0f);
const gColor gColor::DARK_GRAY	(0.25f, 0.25f, 	0.25f, 	1.0f);
const gColor gColor::LIGHT_GRAY	(0.75f, 0.75f, 	0.75f, 	1.0f);
const gColor gColor::ORANGE		(1.0f, 	0.25f, 	0.0f, 	1.0f);
const gColor gColor::PINK 		(1.0f, 	0.69f, 	0.773f,	1.0f);
const gColor gColor::VIOLET		(0.5f,	0.0f, 	1.0f, 	1.0f);

gColor::gColor() {
	this->r = 1.0f;
	this->g = 1.0f;
	this->b = 1.0f;
	this->a = 1.0f;
}

gColor::gColor(float r, float g, float b, float a) {
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

gColor::gColor(gColor* color) {
	this->r = color->r;
	this->g = color->g;
	this->b = color->b;
	this->a = color->a;
}

gColor::~gColor() {
}

void gColor::set(float r, float g, float b, float a) {
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

void gColor::set(int r, int g, int b, int a) {
	this->r = (float)r / 255;
	this->g = (float)g / 255;
	this->b = (float)b / 255;
	this->a = (float)a / 255;
}

void gColor::set(gColor* color) {
	this->r = color->r;
	this->g = color->g;
	this->b = color->b;
	this->a = color->a;
}
