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
			"#version 330 core\n"
			"layout (location = 0) in vec2 aPos;"
			"layout (location = 1) in vec2 aTexCoords;"
			""
			"out vec2 TexCoords;"
			""
			"void main()"
			"{"
			"	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
			"   TexCoords = aTexCoords;"
			"}\n";
	return std::string(shadersource);
}

const std::string gVignette::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			"uniform float innerradius;"
			"uniform float outerradius;"
			"uniform float opacity;"
			""
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
