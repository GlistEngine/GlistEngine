/*
 * gColor.h
 *
 *  Created on: May 9, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GCOLOR_H_
#define ENGINE_GRAPHICS_GCOLOR_H_


#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

class gColor {
public:
	static const gColor
	RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW,
	BLACK, WHITE, GRAY, DARK_GRAY, LIGHT_GRAY, ORANGE, PINK, VIOLET;

	gColor();
	gColor(float r, float g, float b, float a = 1.0f);
	gColor(gColor* color);
	virtual ~gColor();

	void set(float r, float g, float b, float a = 1.0f);
	void set(int r, int g, int b, int a = 255);
	void set(gColor* color);

	glm::vec4 asVec4() {
		return {r, g, b, a};
	}

	glm::vec3 asVec3() {
		return {r, g, b};
	}

	float r, g, b, a;
};

#endif /* ENGINE_GRAPHICS_GCOLOR_H_ */
