/*
 * gSepiatone.cpp
 *
 *  Created on: 10 Tem 2023
 *      Author: BATUHAN
 */

#include "gSepiatone.h"

gSepiatone::gSepiatone() {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
}

gSepiatone::~gSepiatone() {
	delete shader;
}

void gSepiatone::use() {
	shader->use();
}

const std::string gSepiatone::getVertSrc() {
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

const std::string gSepiatone::getFragSrc() {
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
				"	float avarage = dot(texcolor.rgb, vec3(0.2126, 0.7152, 0.0722));"
				" 	vec3 sepiargb = vec3(avarage * 1.2, avarage * 1.0, avarage * 0.8);"
				"	FragColor = vec4(sepiargb, 1.0);"
				"}";
	return std::string(shadersource);
}

