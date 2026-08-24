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

const std::string gHighContrast::getFragSrc() {
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
			"    float midpoint;\n"
			"    float threshold;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float intensity;\n"
			"uniform float midpoint;\n"
			"uniform float threshold;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float intensity;\n"
			"uniform float midpoint;\n"
			"uniform float threshold;\n"
			"#endif\n"
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
