/*
 * gColorChannelMixer.cpp
 *
 *  Created on: 13 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gColorChannelMixer.h"

//Default rgb matrix = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}.
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
			"#if VULKAN\n"
			"#version 450\n"
			"#endif\n"
			"#if GLES\n"
			"#version 300 es\n"
			"precision highp float;\n"
			"#endif\n"
			"#if GLCORE\n"
			"#version 330 core\n"
			"#endif\n"
			"#if VULKAN\n"
			"layout (location = 0) in vec2 aPos;\n"
			"layout (location = 1) in vec2 aTexCoords;\n"
			"layout (location = 0) out vec2 TexCoords;\n"
			"#endif\n"
			"#if GLES\n"
			"layout (location = 0) in vec2 aPos;\n"
			"layout (location = 1) in vec2 aTexCoords;\n"
			"out vec2 TexCoords;\n"
			"#endif\n"
			"#if GLCORE\n"
			"layout (location = 0) in vec2 aPos;\n"
			"layout (location = 1) in vec2 aTexCoords;\n"
			"out vec2 TexCoords;\n"
			"#endif\n"
			"void main()"
			"{"
			"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
			"    TexCoords = aTexCoords;"
			"}\n";
	return std::string(shadersource);
}

const std::string gColorChannelMixer::getFragSrc() {
	const char* shadersource =
			"#if VULKAN\n"
			"#version 450\n"
			"#endif\n"
			"#if GLES\n"
			"#version 300 es\n"
			"precision highp float;\n"
			"#endif\n"
			"#if GLCORE\n"
			"#version 330 core\n"
			"#endif\n"
			"#if VULKAN\n"
			"layout (location = 0) out vec4 FragColor;\n"
			"layout (location = 0) in vec2 TexCoords;\n"
			"layout (set = 0, binding = 0) uniform sampler2D screenTexture;\n"
			"layout (set = 0, binding = 1) uniform Params {\n"
			"    mat3 rgbmatrix;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform mat3 rgbmatrix;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform mat3 rgbmatrix;\n"
			"#endif\n"
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
