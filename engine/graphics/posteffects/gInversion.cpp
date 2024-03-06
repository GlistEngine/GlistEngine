/*
 * gInversion.cpp
 *
 *  Created on: 29 Kas 2021
 *      Author: kayra
 */

#include "gInversion.h"

gInversion::gInversion() {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
}

gInversion::~gInversion() {
}

void gInversion::use() {
	shader->use();
}

const std::string gInversion::getVertSrc() {
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

const std::string gInversion::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			""
			"void main()"
			"{ "
			"    FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);"
			"}\n";
	return std::string(shadersource);
}
