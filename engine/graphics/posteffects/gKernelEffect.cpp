/*
 * gKernelEffect.cpp
 *
 *  Created on: 7 Ara 2021
 *      Author: kayra
 */

#include <posteffects/gKernelEffect.h>

gKernelEffect::gKernelEffect() {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
}

gKernelEffect::~gKernelEffect() {
	// TODO Auto-generated destructor stub
}

const std::string gKernelEffect::getVertSrc() {
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

const std::string gKernelEffect::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			""
			"uniform float offset;"
			"uniform float kernel[9];"
			""
			"void main()"
			"{"
			"    vec2 offsets[9] = vec2[]("
			"        vec2(-offset,  offset),"
			"        vec2( 0.0f,    offset),"
			"        vec2( offset,  offset),"
			"        vec2(-offset,  0.0f),  "
			"        vec2( 0.0f,    0.0f),  "
			"        vec2( offset,  0.0f),  "
			"        vec2(-offset, -offset),"
			"        vec2( 0.0f,   -offset), "
			"        vec2( offset, -offset)  "
			"    );"
			""
			"    vec3 sampleTex[9];"
			"    for(int i = 0; i < 9; i++)"
			"    {"
			"        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));"
			"    }"
			"    vec3 col = vec3(0.0);"
			"    for(int i = 0; i < 9; i++)"
			"	 {"
			"		col += sampleTex[i] * kernel[i];"
			"	 }"
			""
			"    FragColor = vec4(col, 1.0);"
			"}";
	return std::string(shadersource);
}
