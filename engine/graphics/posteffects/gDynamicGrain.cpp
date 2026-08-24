/*
 * gDynamicGrain.cpp
 *
 *  Created on: 11 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gDynamicGrain.h"

gDynamicGrain::gDynamicGrain(float intensity, float speed) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("intensity", intensity);

	counter = 0.0;
	this->speed = speed;
}

gDynamicGrain::~gDynamicGrain() {
	delete shader;
}

void gDynamicGrain::use() {
	shader->use();
	counter += speed;
	shader->setFloat("counter", counter);
}

const std::string gDynamicGrain::getVertSrc() {
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

const std::string gDynamicGrain::getFragSrc() {
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
			"    float counter;\n"
			"    float intensity;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float counter;\n"
			"uniform float intensity;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float counter;\n"
			"uniform float intensity;\n"
			"#endif\n"
			"void main()"
			"{"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			"	float noise = (fract(sin(dot(TexCoords, vec2(12.9898,78.233))) * 43758.5453123 + counter));"  //Perlin noise calculation.
			"	FragColor = texcolor - noise * intensity;"
			"}";
	return std::string(shadersource);
}
