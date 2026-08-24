/*
 * gShadowsMidtonesHighlights.cpp
 *
 *  Created on: 18 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gShadowsMidtonesHighlights.h"

gShadowsMidtonesHighlights::gShadowsMidtonesHighlights(float shadowsrgb[3],
		float midtonesrgb[3], float highlightsrgb[3], float shadowscontrast,
		float midtonescontrast, float highlightscontrast) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setVec3("shadowscolor", shadowsrgb[0], shadowsrgb[1], shadowsrgb[2]);
	shader->setVec3("midtonescolor", midtonesrgb[0], midtonesrgb[1], midtonesrgb[2]);
	shader->setVec3("highlightscolor", highlightsrgb[0], highlightsrgb[1], highlightsrgb[2]);
	shader->setFloat("shadowscontrast", shadowscontrast);
	shader->setFloat("midtonescontrast", midtonescontrast);
	shader->setFloat("highlightscontrast", highlightscontrast);
}

gShadowsMidtonesHighlights::~gShadowsMidtonesHighlights() {
	delete shader;
}

void gShadowsMidtonesHighlights::use() {
	shader->use();
}

const std::string gShadowsMidtonesHighlights::getVertSrc() {
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

const std::string gShadowsMidtonesHighlights::getFragSrc() {
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
			"    vec3 shadowscolor;\n"
			"    vec3 midtonescolor;\n"
			"    vec3 highlightscolor;\n"
			"    float shadowscontrast;\n"
			"    float midtonescontrast;\n"
			"    float highlightscontrast;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform vec3 shadowscolor;\n"
			"uniform vec3 midtonescolor;\n"
			"uniform vec3 highlightscolor;\n"
			"uniform float shadowscontrast;\n"
			"uniform float midtonescontrast;\n"
			"uniform float highlightscontrast;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform vec3 shadowscolor;\n"
			"uniform vec3 midtonescolor;\n"
			"uniform vec3 highlightscolor;\n"
			"uniform float shadowscontrast;\n"
			"uniform float midtonescontrast;\n"
			"uniform float highlightscontrast;\n"
			"#endif\n"
			"void main()"
			"{"
			"	vec4 texcolor = texture(screenTexture, TexCoords);"
			"   vec3 newcolor = texcolor.rgb;"
			"   float avarage = 0.2126 * texcolor.r + 0.7152 * texcolor.g + 0.0722 * texcolor.b;"
			"   if(avarage < 0.33){"
			"   	float t = smoothstep(0.0, 0.33, avarage);"
			"   	newcolor = mix(newcolor.rgb, (newcolor.rgb + shadowscolor) * shadowscontrast, t);"
			"   }else if(avarage < 0.67){"
			"   	float t = smoothstep(0.33, 0.67, avarage);"
			"   	newcolor = mix(newcolor.rgb, (newcolor.rgb + midtonescolor) * midtonescontrast, t);"
			"   }else{"
			"   	float t = smoothstep(0.67, 1.0, avarage);"
			"   	newcolor = mix(newcolor.rgb, (newcolor.rgb + highlightscolor) * highlightscontrast , t);"
			"   }"
			"   FragColor = vec4(newcolor, 1.0);"
			"}";
	return std::string(shadersource);
}
