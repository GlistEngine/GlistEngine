/*
 * gGrayscale.cpp
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#include <posteffects/gGrayscale.h>

gGrayscale::gGrayscale() {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
}

gGrayscale::~gGrayscale() {
	delete shader;
}

void gGrayscale::use() {
	shader->use();
}

const std::string gGrayscale::getVertSrc() {
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

const std::string gGrayscale::getFragSrc() {
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
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			"	float average = 0.2126 * texcolor.r + 0.7152 * texcolor.g + 0.0722 * texcolor.b;"
			"	FragColor = vec4(average, average, average, 1.0);"
			"}";
	return std::string(shadersource);
}
