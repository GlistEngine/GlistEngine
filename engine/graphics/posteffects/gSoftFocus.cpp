/*
 * gSoftFocus.cpp
 *
 *  Created on: 13 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gSoftFocus.h"

gSoftFocus::gSoftFocus(float radius, float softness, float focuscenterx, float focuscentery) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("radius", radius);
	shader->setFloat("softness", softness);
	shader->setFloat("focuscenterx", focuscenterx);
	shader->setFloat("focuscentery", focuscentery);
	shader->setFloat("screenwidth", getScreenWidth());
	shader->setFloat("screenheight", getScreenHeight());
}

gSoftFocus::~gSoftFocus() {
	delete shader;
}

void gSoftFocus::use() {
	shader->use();
}

const std::string gSoftFocus::getVertSrc() {
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

const std::string gSoftFocus::getFragSrc() {
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
			"    float radius;\n"
			"    float softness;\n"
			"    float focuscenterx;\n"
			"    float focuscentery;\n"
			"    float screenwidth;\n"
			"    float screenheight;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float radius;\n"
			"uniform float softness;\n"
			"uniform float focuscenterx;\n"
			"uniform float focuscentery;\n"
			"uniform float screenwidth;\n"
			"uniform float screenheight;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float radius;\n"
			"uniform float softness;\n"
			"uniform float focuscenterx;\n"
			"uniform float focuscentery;\n"
			"uniform float screenwidth;\n"
			"uniform float screenheight;\n"
			"#endif\n"
			"void main()"
			"{"
			"   float offset = 1.0 / 300.0;"
			"   vec2 offsets[9] = vec2[]("
			"   	vec2(-offset,  offset),"
			"       vec2( 0.0,    offset),"
			"       vec2( offset,  offset),"
			"       vec2(-offset,  0.0),"
			"       vec2( 0.0,    0.0),"
			"       vec2( offset,  0.0),"
			"       vec2(-offset, -offset),"
			"       vec2( 0.0,   -offset),"
			"       vec2( offset, -offset)"
			"   );"
			""
			"   float gaussianblur[9] = float[9]("
			"	0.0625, 0.125, 0.0625,"
			"	0.125,  0.25,  0.125,"
			"	0.0625, 0.0125, 0.0625"
			"   );"
			""
			"   vec4 texcolor = texture(screenTexture, TexCoords);"
			"   vec2 focuscenter = vec2(focuscenterx, screenheight - focuscentery);"
			"	float centerdistance = sqrt(pow(TexCoords.x * screenwidth  - focuscenter.x, 2) + pow(TexCoords.y * screenheight - focuscenter.y, 2));"
			""
			"   if(centerdistance > radius){"
			"   	vec3 sampleTex[9];"
			"   	for(int i = 0; i < 9; i++){"
			"       	sampleTex[i] = vec3(texture(screenTexture, TexCoords.xy + offsets[i]));"
			"   	}"
			"   	vec3 col = vec3(0.0);"
			"   	for(int i = 0; i < 9; i++){"
			"			col += sampleTex[i] * gaussianblur[i];"
			"		}"
			"       float bluramount = smoothstep(radius, radius + softness, centerdistance);"
	        "       col = mix(texcolor.rgb, col, bluramount);"
			"   	FragColor = vec4(col, 1.0);"
			"	}else{"
			"   	FragColor = texcolor;"
			"	}"
			"}";
	return std::string(shadersource);
}
