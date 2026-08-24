/*
 * gColorAdjustments.cpp
 *
 *  Created on: 12 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gColorAdjustments.h"

gColorAdjustments::gColorAdjustments(float brightness, float contrast, float saturation, float exposure) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("brightness", brightness);
	shader->setFloat("contrast", contrast);
	shader->setFloat("saturation", saturation);
	shader->setFloat("exposure", exposure);
}

gColorAdjustments::~gColorAdjustments() {
	delete shader;
}

void gColorAdjustments::use() {
	shader->use();
}

const std::string gColorAdjustments::getVertSrc() {
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

const std::string gColorAdjustments::getFragSrc() {
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
			"    float brightness;\n"
			"    float contrast;\n"
			"    float saturation;\n"
			"    float exposure;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float brightness;\n"
			"uniform float contrast;\n"
			"uniform float saturation;\n"
			"uniform float exposure;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float brightness;\n"
			"uniform float contrast;\n"
			"uniform float saturation;\n"
			"uniform float exposure;\n"
			"#endif\n"
			"void main()"
			"{"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			" 	texcolor = pow(texcolor, vec4(2.2));"	// Apply gamma correction (gamma factor = 2.2f)
			"	float avarage = dot(texcolor.rgb, vec3(0.2126, 0.7152, 0.0722));"
			" 	texcolor = mix(vec4(avarage), texcolor, saturation);"	// Apply saturation adjustment
			"   texcolor *= pow(2.0, exposure - 1.0);"	// Apply exposure adjustment
			"   texcolor += brightness - 1.0;"	// Apply brightness adjustment
			"   texcolor = (texcolor - 0.5) * contrast + 0.5;"	// Apply contrast adjustment
			"   texcolor = pow(texcolor, vec4(1.0/2.2));" // Revert the gamma correction
			"	FragColor = texcolor;"
			"}";
	return std::string(shadersource);
}
