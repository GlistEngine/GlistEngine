/*
 * gColor.cpp
 *
 *  Created on: May 9, 2020
 *      Author: noyan
 */

#include "gColor.h"
#include <algorithm>

const gColor gColor::RED 	 	(1.0f,	0.0f,	0.0f,	1.0f);
const gColor gColor::GREEN 		(0.0f,	1.0f, 	0.0f,	1.0f);
const gColor gColor::BLUE 	 	(0.0f, 	0.0f,	1.0f, 	1.0f);
const gColor gColor::CYAN 	 	(0.0f, 	1.0f, 	1.0f, 	1.0f);
const gColor gColor::MAGENTA  	(1.0f, 	0.0f, 	1.0f, 	1.0f);
const gColor gColor::YELLOW		(1.0f, 	1.0f, 	0.0f,	1.0f);
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
	this->r = (float)r / 255.0f;
	this->g = (float)g / 255.0f;
	this->b = (float)b / 255.0f;
	this->a = (float)a / 255.0f;
}

void gColor::set(gColor* color) {
	this->r = color->r;
	this->g = color->g;
	this->b = color->b;
	this->a = color->a;
}

gColorHSL gColorHSL::from(const gColor& rgb) {
    float r = rgb.r, g = rgb.g, b = rgb.b;
    float max = std::max({r, g, b}), min = std::min({r, g, b});
    float delta = max - min;

    float h = 0;
    if (delta > 0.0f) {
        if (max == r) h = fmodf(((g - b) / delta), 6.0f);
        else if (max == g) h = ((b - r) / delta) + 2.0f;
        else h = ((r - g) / delta) + 4.0f;
        h *= 60.0f;
        if (h < 0) h += 360.0f;
    }

    float l = (max + min) * 0.5f;
    float s = (delta == 0) ? 0.0f : delta / (1.0f - fabsf(2.0f * l - 1.0f));

    return gColorHSL(h, s, l);
}

gColor gColorHSL::toRGB() const {
    float c = (1 - fabsf(2 * l - 1)) * s;
    float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
    float m = l - c / 2.0f;

    float r1, g1, b1;
    if (h < 60)      { r1 = c; g1 = x; b1 = 0; }
    else if (h < 120){ r1 = x; g1 = c; b1 = 0; }
    else if (h < 180){ r1 = 0; g1 = c; b1 = x; }
    else if (h < 240){ r1 = 0; g1 = x; b1 = c; }
    else if (h < 300){ r1 = x; g1 = 0; b1 = c; }
    else             { r1 = c; g1 = 0; b1 = x; }

    return gColor(r1 + m, g1 + m, b1 + m);
}