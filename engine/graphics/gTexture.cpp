/*
 * gTexture.cpp
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#include "gTexture.h"
#include <iostream>
#if defined(WIN32) || defined(LINUX)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#if defined(APPLE)
#include <OpenGL/gl.h>
#include <GL/glew.h>
#include <OpenGL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#endif
#include "gPlane.h"
//#ifndef STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#endif
#include "stb/stb_image.h"

const int gTexture::TEXTURETYPE_DIFFUSE = 0;
const int gTexture::TEXTURETYPE_SPECULAR = 1;
const int gTexture::TEXTURETYPE_NORMAL = 2;
const int gTexture::TEXTURETYPE_HEIGHT = 3;
const int gTexture::TEXTURETYPE_PBR_ALBEDO = 4;
const int gTexture::TEXTURETYPE_PBR_ROUGHNESS = 5;
const int gTexture::TEXTURETYPE_PBR_METALNESS = 6;
const int gTexture::TEXTURETYPE_PBR_NORMAL = 7;
const int gTexture::TEXTURETYPE_PBR_AO = 8;

const int gTexture::TEXTUREWRAP_REPEAT = 0;
const int gTexture::TEXTUREWRAP_CLAMP = 1;
const int gTexture::TEXTUREWRAP_CLAMPTOEDGE = 2;

const int gTexture::TEXTUREMINMAGFILTER_LINEAR = 0;
const int gTexture::TEXTUREMINMAGFILTER_MIPMAPLINEAR = 1;
const int gTexture::TEXTUREMINMAGFILTER_NEAREST = 2;

static const int texturewrap[3] = {GL_REPEAT, GL_CLAMP, GL_CLAMP_TO_EDGE};
static const int texturefilter[3] = {GL_LINEAR, GL_CLAMP, GL_CLAMP_TO_EDGE};

gTexture::gTexture() {
	id = GL_NONE;
	internalformat = GL_RGBA;
	format = GL_RGBA;
	wraps = TEXTUREWRAP_REPEAT;
	wrapt = TEXTUREWRAP_REPEAT;
	filtermin = TEXTUREMINMAGFILTER_LINEAR;
	filtermag = TEXTUREMINMAGFILTER_LINEAR;
	texturetype[0] = "texture_diffuse";
	texturetype[1] = "texture_specular";
	texturetype[2] = "texture_normal";
	texturetype[3] = "texture_height";
	type = TEXTURETYPE_DIFFUSE;
	path = "";
	width = 0;
	height = 0;
	bsubpartdrawn = false;
	ismutable = false;
	isfbo = false;
	ishdr = false;
	setupRenderData();
}

gTexture::gTexture(int w, int h, int format, bool isFbo) {
	id = GL_NONE;
	internalformat = format;
	this->format = format;
	wraps = TEXTUREWRAP_REPEAT;
	wrapt = TEXTUREWRAP_REPEAT;
	filtermin = TEXTUREMINMAGFILTER_LINEAR;
	filtermag = TEXTUREMINMAGFILTER_LINEAR;
	texturetype[0] = "texture_diffuse";
	texturetype[1] = "texture_specular";
	texturetype[2] = "texture_normal";
	texturetype[3] = "texture_height";
	type = TEXTURETYPE_DIFFUSE;
	path = "";
	width = w;
	height = h;
	bsubpartdrawn = false;
	ismutable = false;
	isfbo = isFbo;
    glGenTextures(1, &id);
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // TODO: BEFORE SHADOWMAP GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // TODO: BEFORE SHADOWMAP GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	setupRenderData();
}

gTexture::~gTexture() {
	if (ismutable) delete data;
}

unsigned int gTexture::load(std::string fullPath, bool isHDR) {
	fullpath = fullPath;
	directory = getDirName(fullpath);
	path = getFileName(fullpath);
	ishdr = isHDR;

    glGenTextures(1, &id);

    if (ishdr) {
    	stbi_set_flip_vertically_on_load(true);
    	datahdr = stbi_loadf(fullpath.c_str(), &width, &height, &componentnum, 0);
    	setDataHDR(datahdr, false);
    } else {
        data = stbi_load(fullpath.c_str(), &width, &height, &componentnum, 0);
        setData(data, false);
    }

	setupRenderData();
    return id;
}

unsigned int gTexture::loadTexture(std::string texturePath, bool isHDR) {
	return load(gGetTexturesDir() + texturePath, isHDR);
}

unsigned int gTexture::loadData(unsigned char* textureData, int width, int height, int componentNum) {
	this->width = width;
	this->height = height;
	this->componentnum = componentNum;

    glGenTextures(1, &id);

    data = textureData;
    setData(data, true);

	setupRenderData();
    return id;
}

void gTexture::setData(unsigned char* textureData, bool isMutable) {
	ismutable = isMutable;
	data = textureData;
    if (componentnum == 1) format = GL_RED;
    else if (componentnum == 2) format = GL_RG;
    else if (componentnum == 3) format = GL_RGB;
    else if (componentnum == 4) format = GL_RGBA;

    if (data) {
        bind();
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexImage2D(GL_TEXTURE_2D, 0, format, getWidth(), getHeight(), 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (format == GL_RG) {
            GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
            glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        }

        if (!ismutable) stbi_image_free(data);
        unbind();
    } else {
    	gLoge("gTexture") << "Texture failed to load at path: " << fullpath;
        stbi_image_free(data);
    }
}

void gTexture::setDataHDR(float* textureData, bool isMutable) {
	ismutable = isMutable;
	datahdr = textureData;
	if (datahdr) {
		bind();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, datahdr); // note how we specify the texture's data value to be float

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (!ismutable) stbi_image_free(datahdr);
		unbind();
	} else {
		gLoge("gTexture") << "Failed to load HDR image at path: " << fullpath;
		stbi_image_free(datahdr);
	}
}

unsigned char* gTexture::getData() {
	return data;
}

float* gTexture::getDataHDR() {
	return datahdr;
}

bool gTexture::isMutable() {
	return ismutable;
}

void gTexture::bind() {
	glBindTexture(GL_TEXTURE_2D, id);
}

void gTexture::bind(int textureSlotNo) {
	glActiveTexture(GL_TEXTURE0 + textureSlotNo);
	glBindTexture(GL_TEXTURE_2D, id);
}

void gTexture::unbind() {
	glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int gTexture::getId() {
	return id;
}

bool gTexture::isHDR() {
	return ishdr;
}

unsigned int gTexture::getInternalFormat() {
	return internalformat;
}

unsigned int gTexture::getFormat() {
	return format;
}

void gTexture::setType(int textureType) {
	type = textureType;
}

int gTexture::getType() {
	return type;
}

void gTexture::setWrapping(int wrapS, int wrapT) {
	wraps = wrapS;
	wrapt = wrapT;
	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texturewrap[wraps]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texturewrap[wrapt]);
	unbind();
}

void gTexture::setFiltering(int minFilter, int magFilter) {
	filtermin = minFilter;
	filtermag = magFilter;
	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texturefilter[filtermin]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unbind();
}

int gTexture::getWrapS() {
	return wraps;
}

int gTexture::getWrapT() {
	return wrapt;
}

int gTexture::getFilterMin() {
	return filtermin;
}

int gTexture::getFilterMag() {
	return filtermag;
}


std::string gTexture::getTypeName() {
	return texturetype[type];
}

std::string gTexture::getTypeName(int textureType) {
	return texturetype[textureType];
}

std::string gTexture::getFilename() {
	return path;
}

std::string gTexture::getDir() {
	return directory;
}

std::string gTexture::getFullPath() {
	return fullpath;
}

int gTexture::getWidth() {
	return width;
}

int gTexture::getHeight() {
	return height;
}

int gTexture::getComponentNum() {
	return componentnum;
}

void gTexture::draw(int x, int y) {
	draw(x, y, width, height);
}

void gTexture::draw(int x, int y, int w, int h) {
	beginDraw();
	imagematrix = glm::translate(imagematrix, glm::vec3(x, y, 0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)
	imagematrix = glm::scale(imagematrix, glm::vec3(w, h, 1.0f));
	endDraw();
}

void gTexture::draw(int x, int y, int w, int h, float rotate) {
	draw(glm::vec2(x, y), glm::vec2(w, h), rotate);
}

void gTexture::draw(glm::vec2 position, glm::vec2 size, float rotate) {
	beginDraw();
	imagematrix = glm::translate(imagematrix, glm::vec3(position, 0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

	imagematrix = glm::translate(imagematrix, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
	imagematrix = glm::rotate(imagematrix, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	imagematrix = glm::translate(imagematrix, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

	imagematrix = glm::scale(imagematrix, glm::vec3(size.x, size.y, 1.0f));
	endDraw();
}

void gTexture::drawSub(int x, int y, int sx, int sy, int sw, int sh) {
	drawSub(x, y, sw, sh, sx, sy, sw, sh);
}

void gTexture::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh) {
	setupRenderData(sx, sy, sw, sh);
	bsubpartdrawn = true;
	draw(x, y, w, h);
}

void gTexture::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh, float rotate) {
	drawSub(glm::vec2(x, y), glm::vec2(w, h), glm::vec2(sx, sy), glm::vec2(sw, sh), rotate);
}

void gTexture::drawSub(const gRect& src, const gRect& dst, float rotate) {
	drawSub(dst.left(), dst.top(), dst.getWidth(), dst.getHeight(), src.left(), src.top(), src.getWidth(), src.getHeight(), rotate);
}

void gTexture::drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, float rotate) {
	setupRenderData(subpos.x, subpos.y, subsize.x, subsize.y);
	bsubpartdrawn = true;
	draw(pos, size, rotate);
}

void gTexture::beginDraw() {
	renderer->getImageShader()->use();
	imagematrix = glm::mat4(1.0f);
	renderer->setProjectionMatrix2d(glm::ortho(0.0f, (float)renderer->getWidth(), (float)renderer->getHeight(), 0.0f, -1.0f, 1.0f));
}

void gTexture::endDraw() {
	renderer->getImageShader()->setMat4("projection", renderer->getProjectionMatrix2d());
	renderer->getImageShader()->setMat4("model", imagematrix);
	renderer->getImageShader()->setVec3("spriteColor", glm::vec3(renderer->getColor()->r, renderer->getColor()->g, renderer->getColor()->b));
	renderer->getImageShader()->setInt("image", 0);

	glActiveTexture(GL_TEXTURE0);
    bind();
    if (format == GL_RGBA || format == GL_RG) {
        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    if (format == GL_RGBA || format == GL_RG) glDisable(GL_BLEND);
    unbind();
    if(bsubpartdrawn) {	setupRenderData(); }
}

void gTexture::setupRenderData() {
	setupRenderData(0, 0, width, height);
	bsubpartdrawn = false;
}

void gTexture::setupRenderData(int sx, int sy, int sw, int sh) {
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &quadVAO);
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, (float)sx / width, (float)(sy + sh) / height,
        1.0f, 0.0f, (float)(sx + sw) / width, (float)sy / height,
        0.0f, 0.0f, (float)sx / width, (float)sy / height,

        0.0f, 1.0f, (float)sx / width, (float)(sy + sh) / height,
        1.0f, 1.0f, (float)(sx + sw) / width, (float)(sy + sh) / height,
        1.0f, 0.0f, (float)(sx + sw) / width, (float)sy / height
    };
    float vertices2[] = {
        // pos      // tex
        0.0f, 1.0f, (float)sx / width, (float)(sy) / height,
        1.0f, 0.0f, (float)(sx + sw) / width, (float)(sy + sh) / height,
        0.0f, 0.0f, (float)sx / width, (float)(sy + sh) / height,

        0.0f, 1.0f, (float)sx / width, (float)(sy) / height,
        1.0f, 1.0f, (float)(sx + sw) / width, (float)(sy) / height,
        1.0f, 0.0f, (float)(sx + sw) / width, (float)(sy + sh) / height
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    if (isfbo) glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
    else glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

std::string gTexture::getDirName(const std::string& fname) {
     size_t pos = fname.find_last_of("\\/");
     return (std::string::npos == pos)
         ? ""
         : fname.substr(0, pos);
}

std::string gTexture::getFileName(const std::string& fname) {
     size_t pos = fname.find_last_of("\\/");
     return (std::string::npos == pos)
         ? ""
         : fname.substr(pos + 1, fname.size());
}

