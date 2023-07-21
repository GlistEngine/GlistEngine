/*
 * gChromaticAberration.cpp
 *
 *  Created on: 17 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gChromaticAberration.h"

gChromaticAberration::gChromaticAberration(float intensity) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("intensity", intensity);
}

gChromaticAberration::~gChromaticAberration() {
	delete shader;
}

void gChromaticAberration::use() {
	shader->use();
}

const std::string gChromaticAberration::getVertSrc() {
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

const std::string gChromaticAberration::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			"uniform float intensity;"
			""
			"void main()"
			"{"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			"	vec2 centervector = TexCoords - vec2(0.5, 0.5);"
			"	vec2 offset = (intensity / 100) * pow(centervector, vec2(0.0, 1.0));"
			"	float red = texture(screenTexture, TexCoords - offset).r;"
			"	float blue = texture(screenTexture, TexCoords + offset).b;"
			"	FragColor = vec4(red, texcolor.g, blue, 1.0);"
			"}";
	return std::string(shadersource);
}
