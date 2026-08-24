/*
 * gColorCurves.cpp
 *
 *  Created on: 18 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gColorCurves.h"

//rgbcurvepoint.x = brightness value of the channel, rgbcurvepoint.y = contrast value of the channel.
//default rgbcurvepoints = {{1.0, 1.0}, {1.0, 1.0}, {1.0, 1.0}}.
gColorCurves::gColorCurves(float rgbcurvepoints[3][2]) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setVec2("redpoint", rgbcurvepoints[0][0], rgbcurvepoints[0][1]);
	shader->setVec2("greenpoint", rgbcurvepoints[1][0], rgbcurvepoints[1][1]);
	shader->setVec2("bluepoint", rgbcurvepoints[2][0], rgbcurvepoints[2][1]);
}

gColorCurves::~gColorCurves() {
	delete shader;
}

void gColorCurves::use() {
	shader->use();
}

const std::string gColorCurves::getVertSrc() {
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

const std::string gColorCurves::getFragSrc() {
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
			"    vec2 redpoint;\n"
			"    vec2 greenpoint;\n"
			"    vec2 bluepoint;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform vec2 redpoint;\n"
			"uniform vec2 greenpoint;\n"
			"uniform vec2 bluepoint;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform vec2 redpoint;\n"
			"uniform vec2 greenpoint;\n"
			"uniform vec2 bluepoint;\n"
			"#endif\n"
			"void main()"
			"{"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			"   vec3 newcolor;"
			"	newcolor.r = mix(texcolor.r, (texcolor.r - 0.5) * redpoint.y + 0.5 - 1.0 + redpoint.x, smoothstep(0.0, 1.0, texcolor.r));"
			" 	newcolor.g = mix(texcolor.g, (texcolor.g - 0.5) * greenpoint.y + 0.5 - 1.0 + greenpoint.x, smoothstep(0.0, 1.0, texcolor.g));"
			" 	newcolor.b = mix(texcolor.b, (texcolor.b - 0.5) * bluepoint.y + 0.5 - 1.0 + bluepoint.x, smoothstep(0.0, 1.0, texcolor.b));"
			"	FragColor = vec4(newcolor, 1.0);"
			"}";
	return std::string(shadersource);
}
