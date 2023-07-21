/*
 * gGrain.cpp
 *
 *  Created on: 11 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gGrain.h"

gGrain::gGrain(float intensity) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("intensity", intensity);
}

gGrain::~gGrain() {
	delete shader;
}

void gGrain::use() {
	shader->use();
}

const std::string gGrain::getVertSrc() {
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

const std::string gGrain::getFragSrc() {
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
			"	float noise = (fract(sin(dot(TexCoords, vec2(12.9898,78.233))) * 43758.5453123));"  //Perlin noise calculation.
			"	FragColor = texcolor - noise * intensity;"
			"}";
	return std::string(shadersource);
}
