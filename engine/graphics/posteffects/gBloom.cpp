/*
 * gBloom.cpp
 *
 *  Created on: 19 Tem 2023
 *      Author: Batuhan Yigit
 */

#include "gBloom.h"

gBloom::gBloom(float intensity, float bloomsize, float threshold) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("intensity", intensity);
	shader->setFloat("bloomsize", bloomsize);
	shader->setFloat("threshold", threshold);
	shader->setFloat("screenwidth", getScreenWidth());
	shader->setFloat("screenheight", getScreenHeight());
	shader->setInt("rectssize", 1);
	shader->setVec4("rects[0]", 0.0, 0.0, getScreenWidth(), getScreenHeight());
	shader->setBool("isrgbmask", false);
	isrectsset = false;
}

gBloom::gBloom(float intensity, float bloomsize, float threshold, std::vector<std::vector<float>> rects, float screenwidth, float screenheight) {
	this->rects = rects;
	rectssize = this->rects.size();

	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("intensity", intensity);
	shader->setFloat("bloomsize", bloomsize);
	shader->setFloat("threshold", threshold);
	shader->setFloat("screenwidth", screenwidth);
	shader->setFloat("screenheight", screenheight);
	shader->setInt("rectssize",  rectssize);
	for(int i = 0; i <  rectssize; i++){
		shader->setVec4("rects[" + gToStr(i) + "]", this->rects[i][0], this->rects[i][1], this->rects[i][2], this->rects[i][3]);
	}
	shader->setBool("isrgbmask", false);
	isrectsset = false;
}

gBloom::gBloom(float intensity, float bloomsize, float rgb[]) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setFloat("intensity", intensity);
	shader->setFloat("bloomsize", bloomsize);
	shader->setFloat("threshold", 0.0f);
	shader->setFloat("screenwidth", getScreenWidth());
	shader->setFloat("screenheight", getScreenHeight());
	shader->setVec3("rgbmask", rgb[0], rgb[1], rgb[2]);
	shader->setInt("rectssize", 1);
	shader->setVec4("rects[0]", 0.0, 0.0, getScreenWidth(), getScreenHeight());
	shader->setBool("isrgbmask", true);
	isrectsset = false;
}

gBloom::~gBloom() {
	delete shader;
}

void gBloom::use() {
	shader->use();
	if(isrectsset){
		for(int i = 0; i < rectssize; i++){
			shader->setVec4("rects[" + gToStr(i) + "]", rects[i][0], rects[i][1], rects[i][2], rects[i][3]);
		}
	}
}

void gBloom::setRect(int rectnum, float x, float y, float width, float height) {
	if(rectnum >= rectssize) return;
	isrectsset = true;
	rects[rectnum][0] = x;
	rects[rectnum][1] = y;
	rects[rectnum][2] = width;
	rects[rectnum][3] = height;
}

const std::string gBloom::getVertSrc() {
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

const std::string gBloom::getFragSrc() {
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
			"    float bloomsize;\n"
			"    float threshold;\n"
			"    float screenwidth;\n"
			"    float screenheight;\n"
			"    vec3 rgbmask;\n"
			"    bool isrgbmask;\n"
			"    vec4 rects[10];\n"
			"    int rectssize;\n"
			"};\n"
			"#endif\n"
			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float intensity;\n"
			"uniform float bloomsize;\n"
			"uniform float threshold;\n"
			"uniform float screenwidth;\n"
			"uniform float screenheight;\n"
			"uniform vec3 rgbmask;\n"
			"uniform bool isrgbmask;\n"
			"uniform vec4 rects[10];\n"
			"uniform int rectssize;\n"
			"#endif\n"
			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float intensity;\n"
			"uniform float bloomsize;\n"
			"uniform float threshold;\n"
			"uniform float screenwidth;\n"
			"uniform float screenheight;\n"
			"uniform vec3 rgbmask;\n"
			"uniform bool isrgbmask;\n"
			"uniform vec4 rects[10];\n"
			"uniform int rectssize;\n"
			"#endif\n"
			"void main()"
			"{"
			""
			"   float offset = bloomsize / 300.0;"
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
			"	0.0625, 0.125, 0.0625"
			"   );"
			""
			"   vec4 texcolor = texture(screenTexture, TexCoords);"
			" 	float brightness = dot(texcolor.rgb, vec3(0.2126, 0.7152, 0.0722));"
			"   vec4 blurcolor = vec4(0.0, 0.0, 0.0, 1.0);"
			"   const float rgbrangeoffset = 0.01;"
			" 	for(int j = 0; j < rectssize; j++){"
			" 		if(TexCoords.x * screenwidth >= rects[j].x && TexCoords.x * screenwidth <= rects[j].x + rects[j].z &&"
			"			TexCoords.y * screenheight <= screenheight - rects[j].y && TexCoords.y * screenheight >= screenheight - rects[j].y - rects[j].w){"
			"   		for(int i = 0; i < 9; i++){"
			"				vec4 sampletex = texture(screenTexture, TexCoords.xy + offsets[i]);"
			"           	float sampletexbrightness = dot(sampletex.rgb, vec3(0.2126, 0.7152, 0.0722));"
			""
			"				if(isrgbmask){"
			"           		if(!(sampletex.r >= rgbmask.r - rgbrangeoffset && sampletex.r <= rgbmask.r + rgbrangeoffset &&"
			" 						sampletex.g >= rgbmask.g - rgbrangeoffset && sampletex.g <= rgbmask.g + rgbrangeoffset && "
			"						sampletex.b >= rgbmask.b - rgbrangeoffset && sampletex.b <= rgbmask.b + rgbrangeoffset)){"
			"							sampletex.rgb = vec3(0.0);"
			"					}"
			"				}else{"
			"					if(sampletexbrightness < threshold){"
			"						sampletex.rgb = vec3(0.0);}"
			"				}"
			""
			"   			blurcolor.rgb += vec3(sampletex.rgb) * gaussianblur[i];"
			"   		}"
			" 		}"
			" 	}"
			"	FragColor = texcolor + blurcolor * intensity;"
			"}";
	return std::string(shadersource);
}


