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
			"layout (location = 0) in vec2 aPos;\n"
			"layout (location = 1) in vec2 aTexCoords;\n"
			"#if VULKAN\n"
			"layout (location = 0) out vec2 TexCoords;\n"
			"#endif\n"
			"#if GLES\n"
			"out vec2 TexCoords;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec2 TexCoords;\n"
			"#endif\n"
			"void main()\n"
			"{\n"
			"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
			"    TexCoords = aTexCoords;\n"
			"}\n";

	return std::string(shadersource);
}

const std::string gGrayscale::getFragSrc() {
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
			"layout (location = 0) in vec2 TexCoords;\n"
			"layout (location = 0) out vec4 FragColor;\n"
			"layout (set = 0, binding = 0) uniform sampler2D screenTexture;\n"
			"#endif\n"
			"#if GLES\n"
			"in vec2 TexCoords;\n"
			"out vec4 FragColor;\n"
			"uniform sampler2D screenTexture;\n"
			"#endif\n"
			"#if GLCORE\n"
			"in vec2 TexCoords;\n"
			"out vec4 FragColor;\n"
			"uniform sampler2D screenTexture;\n"
			"#endif\n"
			"void main()\n"
			"{\n"
			"	vec4 texcolor = texture(screenTexture, TexCoords);\n"
			"	float average = 0.2126 * texcolor.r + 0.7152 * texcolor.g + 0.0722 * texcolor.b;\n"
			"	FragColor = vec4(average, average, average, 1.0);\n"
			"}\n";
	return std::string(shadersource);
}
