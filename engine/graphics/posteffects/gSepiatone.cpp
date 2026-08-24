/*
 * gSepiatone.cpp
 *
 *  Created on: 10 Tem 2023
 *      Author: Batuhan Yigit
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

const std::string gSepiatone::getFragSrc() {
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
				"#endif\n"
				"#if GLES\n"
				"out vec4 FragColor;\n"
				"in vec2 TexCoords;\n"
				"uniform sampler2D screenTexture;\n"
				"#endif\n"
				"#if GLCORE\n"
				"out vec4 FragColor;\n"
				"in vec2 TexCoords;\n"
				"uniform sampler2D screenTexture;\n"
				"#endif\n"
				"void main()"
				"{ "
				"	vec4 texcolor = texture(screenTexture, TexCoords);"
				"	float avarage = dot(texcolor.rgb, vec3(0.2126, 0.7152, 0.0722));"
				" 	vec3 sepiargb = vec3(avarage * 1.2, avarage * 1.0, avarage * 0.8);"
				"	FragColor = vec4(sepiargb, 1.0);"
				"}";
	return std::string(shadersource);
}

