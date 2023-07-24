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
	shader->setVec4("rect[0]", 0.0, 0.0, getScreenWidth(), getScreenHeight());
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

const std::string gBloom::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"#define MAX_RECT 10\n"
			""
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			"uniform float intensity;"
			"uniform float bloomsize;"
			"uniform float threshold;"
			"uniform float screenwidth;"
			"uniform float screenheight;"
			""
			"uniform vec3 rgbmask;"
			"uniform bool isrgbmask;"
			""
			"uniform vec4 rects[MAX_RECT];"
			"uniform int rectssize;"
			""
			"void main()"
			"{"
			"	const float rgbrangeoffset = 0.01;"
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
			"	0.0625, 0.0125, 0.0625"
			"   );"
			""
			"   vec4 texcolor = texture(screenTexture, TexCoords);"
			" 	float brightness = dot(texcolor.rgb, vec3(0.2126, 0.7152, 0.0722));"
			"   vec4 blurcolor = vec4(0.0, 0.0, 0.0, 1.0);"
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


