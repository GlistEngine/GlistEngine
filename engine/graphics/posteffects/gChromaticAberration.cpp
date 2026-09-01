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

const std::string gChromaticAberration::getFragSrc() {
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
			"    float intensity;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float intensity;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float intensity;\n"
			"#endif\n"
			"void main()"
			"{"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			"	vec2 centervector = TexCoords - vec2(0.5, 0.5);"
			"\n#if VULKAN\n"
			"	vec2 offset = (intensity / 100) * vec2(1.0, centervector.y);\n"
			"#endif\n"
			"#if GLES\n"
			"	vec2 offset = (intensity / 100) * pow(centervector, vec2(0.0, 1.0));\n"
			"#endif\n"
			"#if GLCORE\n"
			"	vec2 offset = (intensity / 100) * pow(centervector, vec2(0.0, 1.0));\n"
			"#endif\n"
			"	float red = texture(screenTexture, TexCoords - offset).r;"
			"	float blue = texture(screenTexture, TexCoords + offset).b;"
			"	FragColor = vec4(red, texcolor.g, blue, 1.0);"
			"}";
	return std::string(shadersource);
}
