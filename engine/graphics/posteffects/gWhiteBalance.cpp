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

const std::string gWhiteBalance::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			"uniform float temperature;"
			"uniform float tint;"
			""
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
