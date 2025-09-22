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
#include <cmath>

// Since we send the color to the GPU via Uniform Buffer Objects, this has to be aligned and packed
// It should not have a vtable data (no virtual functions)
class alignas(16) gColor {
public:
	static const gColor
	RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW,
	BLACK, WHITE, GRAY, DARK_GRAY, LIGHT_GRAY, ORANGE, PINK, VIOLET;

	gColor();
	gColor(float r, float g, float b, float a = 1.0f);
	gColor(gColor* color);
	~gColor();

	void set(float r, float g, float b, float a = 1.0f);
	void set(int r, int g, int b, int a = 255);
	void set(gColor* color);

	glm::vec4 asVec4() const {
		return {r, g, b, a};
	}

	glm::vec3 asVec3() const {
		return {r, g, b};
	}

	float r, g, b, a;
} __attribute__((packed));

inline bool operator!=(const gColor& v1, const gColor& v2) {
	return v1.r != v2.r || v1.g != v2.g || v1.b != v2.b || v1.a != v2.a;
}

inline bool operator==(const gColor& v1, const gColor& v2) {
	return v1.r == v2.r && v1.g == v2.g && v1.b == v2.b && v1.a == v2.a;
}

// This is a utility class, HSL color space is much more useful when doing color animations
// You can simply rotate the hue and have a good-looking rainbow.
class gColorHSL {
public:
    gColorHSL() : h(0), s(0), l(0) {}
    gColorHSL(float h, float s, float l) : h(h), s(s), l(l) {}

    static gColorHSL from(const gColor& rgb);

    gColor toRGB() const;

	float h, s, l; // Hue [0–360], Saturation [0–1], Lightness [0–1]
};



#endif /* ENGINE_GRAPHICS_GCOLOR_H_ */
