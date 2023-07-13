/*
 * gColorChannelMixer.cpp
 *
 *  Created on: 13 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gColorChannelMixer.h"

gColorChannelMixer::gColorChannelMixer(float rgbmatrix[3][3]) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setMat3("rgbmatrix",
			{{rgbmatrix[0][0], rgbmatrix[0][1], rgbmatrix[0][2]},
			 {rgbmatrix[1][0], rgbmatrix[1][1], rgbmatrix[1][2]},
			 {rgbmatrix[2][0], rgbmatrix[2][1], rgbmatrix[2][2]}}
	);
}

gColorChannelMixer::~gColorChannelMixer() {
	delete shader;
}

void gColorChannelMixer::use() {
	shader->use();
}

const std::string gColorChannelMixer::getVertSrc() {
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

const std::string gColorChannelMixer::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			"uniform mat3 rgbmatrix;"
			""
			"void main()"
			"{"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			"	float redchannelvalue = rgbmatrix[0][0] * texcolor.r + rgbmatrix[0][1] * texcolor.g + rgbmatrix[0][2] * texcolor.b;"
			"   float greenchannelvalue = rgbmatrix[1][0] * texcolor.r + rgbmatrix[1][1] * texcolor.g + rgbmatrix[1][2] * texcolor.b;"
			"   float bluechannelvalue = rgbmatrix[2][0] * texcolor.r + rgbmatrix[2][1] * texcolor.g + rgbmatrix[2][2] * texcolor.b;"
			"	FragColor = vec4(redchannelvalue, greenchannelvalue, bluechannelvalue, 1.0);"
			"}";
	return std::string(shadersource);
}
