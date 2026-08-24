/*
 * gVignette.cpp
 *
 *  Created on: 14 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gVignette.h"

gVignette::gVignette(float outerradius, float innerradius, float opacity) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("innerradius", innerradius);
	shader->setFloat("outerradius", outerradius);
	shader->setFloat("opacity", opacity);
}

gVignette::~gVignette() {
	delete shader;
}

void gVignette::use() {
	shader->use();
}

const std::string gVignette::getVertSrc() {
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

const std::string gVignette::getFragSrc() {
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
			"    float innerradius;\n"
			"    float outerradius;\n"
			"    float opacity;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float innerradius;\n"
			"uniform float outerradius;\n"
			"uniform float opacity;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float innerradius;\n"
			"uniform float outerradius;\n"
			"uniform float opacity;\n"
			"#endif\n"
			"void main()"
			"{"
			"   vec4 initialtexcolor = texture(screenTexture, TexCoords);"
			"   vec4 texcolor = initialtexcolor;"
			"   vec2 vignettecenter = vec2(0.5, 0.5);"
			"	float centerdistance = distance(TexCoords, vignettecenter);"
		    "	float vignette = smoothstep(outerradius, innerradius, centerdistance);"
			"   texcolor *= vignette;"
			"   texcolor = mix(initialtexcolor, texcolor, opacity);"
			"	FragColor = texcolor;"
			"}";
	return std::string(shadersource);
}
