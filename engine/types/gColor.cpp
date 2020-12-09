/*
 * gColor.cpp
 *
 *  Created on: May 9, 2020
 *      Author: noyan
 */

#include "gColor.h"


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
