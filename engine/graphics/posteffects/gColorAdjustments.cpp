/*
 * gColorAdjustments.cpp
 *
 *  Created on: 12 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gColorAdjustments.h"

gColorAdjustments::gColorAdjustments(float brightness, float contrast, float saturation, float exposure) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("brightness", brightness);
	shader->setFloat("contrast", contrast);
	shader->setFloat("saturation", saturation);
	shader->setFloat("exposure", exposure);
}

gColorAdjustments::~gColorAdjustments() {
	delete shader;
}

void gColorAdjustments::use() {
	shader->use();
}

const std::string gColorAdjustments::getVertSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"layout (location = 0) in vec2 aPos;"
			"layout (location = 1) in vec2 aTexCoords;"
			""
			"out vec2 TexCoords;"
			""
			"void main()"
			"{"
			"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
			"    TexCoords = aTexCoords;"
			"}\n";
	return std::string(shadersource);
}

const std::string gColorAdjustments::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			"uniform float brightness;"
			"uniform float contrast;"
			"uniform float saturation;"
			"uniform float exposure;"
			""
			"void main()"
			"{"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			" 	texcolor = pow(texcolor, vec4(2.2));"	// Apply gamma correction (gamma factor = 2.2f)
			"	float avarage = dot(texcolor.rgb, vec3(0.2126, 0.7152, 0.0722));"
			" 	texcolor = mix(vec4(avarage), texcolor, saturation);"	// Apply saturation adjustment
			"   texcolor *= pow(2.0, exposure - 1.0);"	// Apply exposure adjustment
			"   texcolor += brightness - 1.0;"	// Apply brightness adjustment
			"   texcolor = (texcolor - 0.5) * contrast + 0.5;"	// Apply contrast adjustment
			"   texcolor = pow(texcolor, vec4(1.0/2.2));" // Revert the gamma correction
			"	FragColor = texcolor;"
			"}";
	return std::string(shadersource);
}
