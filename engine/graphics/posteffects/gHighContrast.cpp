/*
 * gHighContrast.cpp
 *
 *  Created on: 17 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gHighContrast.h"

gHighContrast::gHighContrast(float intensity, float midpoint, float threshold) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("intensity", intensity);
	shader->setFloat("midpoint", midpoint);
	shader->setFloat("threshold", threshold);
}

gHighContrast::~gHighContrast() {
	delete shader;
}

void gHighContrast::use() {
	shader->use();
}

const std::string gHighContrast::getVertSrc() {
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

const std::string gHighContrast::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			"uniform float intensity;"
			"uniform float midpoint;"
			"uniform float threshold;"
			""
			"void main()"
			"{"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			"   float gray = dot(texcolor.rgb, vec3(0.2126, 0.7152, 0.0722));"
			" 	float contrast = (gray - midpoint) * intensity + midpoint;"
			"   vec3 newcolor;"
		    "   if(gray < 1 - threshold){"
		    "   	newcolor = mix(vec3(0.0), texcolor.rgb, contrast);"
		    "	}else{"
			"    	newcolor = mix(vec3(1.0), texcolor.rgb, 1.0 - contrast);"
			"	}"
			"   newcolor = clamp(newcolor, 0.0, 1.0);"
			"   FragColor = vec4(newcolor, 1.0);"
			"}";
	return std::string(shadersource);
}
