/*
 * gWhiteBalance.cpp
 *
 *  Created on: 18 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gWhiteBalance.h"

gWhiteBalance::gWhiteBalance(float temperature, float tint) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("temperature", temperature);
	shader->setFloat("tint", tint);
}

gWhiteBalance::~gWhiteBalance() {
	delete shader;
}

void gWhiteBalance::use() {
	shader->use();
}

const std::string gWhiteBalance::getVertSrc() {
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
			"	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
			"   TexCoords = aTexCoords;"
			"}\n";
	return std::string(shadersource);
}

const std::string gWhiteBalance::getFragSrc() {
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
			"    float temperature;\n"
			"    float tint;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float temperature;\n"
			"uniform float tint;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float temperature;\n"
			"uniform float tint;\n"
			"#endif\n"
			"void main()"
			"{"
			"   float tintvalue = 0.0;"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			"	texcolor = pow(texcolor, vec4(2.2));"
			"	if(temperature > 0){"
			"		texcolor = mix(texcolor, texcolor + vec4(0.4, 0.22, 0.1, 1.0), temperature / 10);"
			"	}else{"
			"   	texcolor = mix(texcolor, texcolor + vec4(0.1, 0.22, 0.4, 1.0), abs(temperature / 10));"
			"	}"
			"   tintvalue = texcolor.r * (tint / 10.0) + texcolor.b * (tint / 10.0);"
			"   texcolor += vec4(tintvalue, 0.0, tintvalue, 1.0);"
			"	texcolor = pow(texcolor, vec4(1.0 / 2.2));"
			"	FragColor = texcolor;"
			"}";
	return std::string(shadersource);
}
