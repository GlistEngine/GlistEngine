/*
 * gTiltShift.cpp
 *
 *  Created on: 19 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gTiltShift.h"

gTiltShift::gTiltShift(float upperheight, float lowerheight, float softness) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("upperheight", upperheight);
	shader->setFloat("lowerheight", lowerheight);
	shader->setFloat("softness", softness);
	shader->setFloat("screenheight", getScreenHeight());
}

gTiltShift::~gTiltShift() {
	delete shader;
}

void gTiltShift::use() {
	shader->use();
}

const std::string gTiltShift::getVertSrc() {
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

const std::string gTiltShift::getFragSrc() {
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
			"    float upperheight;\n"
			"    float lowerheight;\n"
			"    float softness;\n"
			"    float screenheight;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float upperheight;\n"
			"uniform float lowerheight;\n"
			"uniform float softness;\n"
			"uniform float screenheight;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float upperheight;\n"
			"uniform float lowerheight;\n"
			"uniform float softness;\n"
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
			""
			"   if(TexCoords.y * screenheight < lowerheight || TexCoords.y * screenheight > upperheight){"
			"   	vec3 sampleTex[9];"
			"   	for(int i = 0; i < 9; i++){"
			"       	sampleTex[i] = vec3(texture(screenTexture, TexCoords.xy + offsets[i]));"
			"   	}"
			"   	vec3 col = vec3(0.0);"
			"   	for(int i = 0; i < 9; i++){"
			"			col += sampleTex[i] * gaussianblur[i];"
			"		}"
			"       float bluramount;"
			"       if(TexCoords.y * screenheight < lowerheight){"
			"       	 bluramount = smoothstep(0.0, softness / 10 * screenheight, lowerheight - TexCoords.y * screenheight);"
			"       	 col = mix(texcolor.rgb, col, bluramount);"
			"       }else{"
			"            bluramount = smoothstep(0.0, softness / 10 * screenheight, TexCoords.y * screenheight - upperheight);"
			"       	 col = mix(texcolor.rgb, col, bluramount);"
			"       }"
			"   	FragColor = vec4(col, 1.0);"
			"	}else{"
			"   	FragColor = texcolor;"
			"	}"
			"}";
	return std::string(shadersource);
}
