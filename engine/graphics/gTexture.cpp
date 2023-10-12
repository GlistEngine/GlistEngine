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
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
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

#if(ANDROID)
// todo alternatives?
static const int texturewrap[3] = {GL_REPEAT, GL_NEAREST, GL_NEAREST};
static const int texturefilter[3] = {GL_LINEAR, GL_NEAREST, GL_NEAREST};
#else
static const int texturewrap[3] = {GL_REPEAT, GL_CLAMP, GL_CLAMP_TO_EDGE};
static const int texturefilter[3] = {GL_LINEAR, GL_CLAMP, GL_CLAMP_TO_EDGE};
#endif

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
	isstbimage = false;
	isfbo = false;
	ishdr = false;
	ismaskloaded = false;
	isloaded = false;
	masktexture = nullptr;
	componentnum = 0;
	istextureallocated = false;
	data = nullptr;
	datahdr = nullptr;
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
	isstbimage = false;
	isfbo = isFbo;
	ishdr = false;
	ismaskloaded = false;
	isloaded = false;
	masktexture = nullptr;
	componentnum = 0;
    data = nullptr;
	datahdr = nullptr;
	glGenTextures(1, &id);
	istextureallocated = true;
	bind();
	G_CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr));

	// TODO: BEFORE SHADOWMAP GL_REPEAT
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	setupRenderData();
}

gTexture::~gTexture() {
	cleanupAll();
}

unsigned int gTexture::load(const std::string& fullPath) {
	cleanupData();

	fullpath = fullPath;
	directory = getDirName(fullpath);
	path = getFileName(fullpath);
	ishdr = false;
	if (gToLower(fullpath.substr(fullpath.length() - 3, 3)) == "hdr") ishdr = true;

	if (!istextureallocated) {
		glGenTextures(1, &id);
		istextureallocated = true;
	}

	if (ishdr) {
		stbi_set_flip_vertically_on_load(true);
		datahdr = stbi_loadf(fullpath.c_str(), &width, &height, &componentnum, 0);
		setDataHDR(datahdr, false, true);
	} else {
		data = stbi_load(fullpath.c_str(), &width, &height, &componentnum, 0);
		setData(data, false, true);
	}

	//	setupRenderData();
	return id;
}

unsigned int gTexture::loadTexture(const std::string& texturePath) {
	return load(gGetTexturesDir() + texturePath);
}

unsigned int gTexture::loadMask(const std::string& fullPath) {
	masktexture = new gTexture();
	ismaskloaded = true;
	return masktexture->load(fullPath);
}

unsigned int gTexture::loadMaskTexture(const std::string& maskTexturePath) {
	masktexture = new gTexture();
	ismaskloaded = true;
	return masktexture->load(gGetTexturesDir() + maskTexturePath);
}

unsigned int gTexture::loadData(unsigned char* textureData, int width, int height, int componentNum, bool isMutable, bool isStbImage) {
	this->width = width;
	this->height = height;
	this->componentnum = componentNum;

	if (!istextureallocated) {
		glGenTextures(1, &id);
		istextureallocated = true;
	}

	setData(textureData, isMutable, isStbImage);

	//	setupRenderData();
	return id;
}

void gTexture::setData(unsigned char* textureData, bool isMutable, bool isStbImage, bool clean) {
	if(clean) cleanupData();

	ismutable = isMutable;
	isstbimage = isStbImage;
	ishdr = false;
	data = textureData;
	if (componentnum == 1) format = GL_RED;
	else if (componentnum == 2) format = GL_RG;
	else if (componentnum == 3) format = GL_RGB;
	else if (componentnum == 4) format = GL_RGBA;

	if (data) {
		bind();
		G_CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
		G_CHECK_GL(glGenerateMipmap(GL_TEXTURE_2D));

		G_CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, format, getWidth(), getHeight(), 0, format, GL_UNSIGNED_BYTE, data));

		G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
		G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
		G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
		G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		if (format == GL_RG) {
			GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
#if(ANDROID)
			G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, swizzleMask[0]));
			G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, swizzleMask[1]));
			G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, swizzleMask[2]));
			G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, swizzleMask[3]));
#else
			G_CHECK_GL(glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask));
#endif
		}

		if (isstbimage && !ismutable) {
			stbi_image_free(data);
			data = nullptr;
		}
		unbind();
	} else {
		gLoge("gTexture") << "Texture failed to load at path: " << fullpath;
	}

	setupRenderData();
}

void gTexture::setDataHDR(float* textureData, bool isMutable, bool isStbImage, bool clean) {
	if(clean)cleanupData();

	ismutable = isMutable;
	isstbimage = isStbImage;
	ishdr = true;
	datahdr = textureData;
	if (datahdr) {
		bind();
		G_CHECK_GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, datahdr)); // note how we specify the texture's data value to be float

		G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		if (isstbimage && !ismutable) {
			stbi_image_free(datahdr);
			datahdr = nullptr;
		}
		unbind();
	} else {
		gLoge("gTexture") << "Failed to load HDR image at path: " << fullpath;
	}

	setupRenderData();
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

void gTexture::bind() const {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_2D, id));
}

void gTexture::bind(int textureSlotNo) const {
	G_CHECK_GL(glActiveTexture(GL_TEXTURE0 + textureSlotNo));
	G_CHECK_GL(glBindTexture(GL_TEXTURE_2D, id));
}

void gTexture::unbind() const {
	G_CHECK_GL(glBindTexture(GL_TEXTURE_2D, 0));
}

unsigned int gTexture::getId() const {
	return id;
}

bool gTexture::isHDR() const {
	return ishdr;
}

unsigned int gTexture::getInternalFormat() const {
	return internalformat;
}

unsigned int gTexture::getFormat() const {
	return format;
}

void gTexture::setType(int textureType) {
	type = textureType;
}

int gTexture::getType() const {
	return type;
}

void gTexture::setWrapping(int wrapS, int wrapT) {
	wraps = wrapS;
	wrapt = wrapT;
	bind();
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texturewrap[wraps]));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texturewrap[wrapt]));
	unbind();
}

void gTexture::setFiltering(int minFilter, int magFilter) {
	filtermin = minFilter;
	filtermag = magFilter;
	bind();
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texturefilter[filtermin]));
	G_CHECK_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texturefilter[filtermag]));
	unbind();
}

int gTexture::getWrapS() const {
	return wraps;
}

int gTexture::getWrapT() const {
	return wrapt;
}

int gTexture::getFilterMin() const {
	return filtermin;
}

int gTexture::getFilterMag() const {
	return filtermag;
}


const std::string& gTexture::getTypeName() const {
	return texturetype[type];
}

const std::string& gTexture::getTypeName(int textureType) const {
	return texturetype[textureType];
}

const std::string& gTexture::getFilename() const {
	return path;
}

const std::string& gTexture::getDir() const {
	return directory;
}

const std::string& gTexture::getFullPath() const {
	return fullpath;
}

int gTexture::getWidth() const {
	return width;
}

int gTexture::getHeight() const {
	return height;
}

int gTexture::getComponentNum() const {
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

void gTexture::draw(int x, int y, int w, int h, int pivotx, int pivoty, float rotate) {
	draw(glm::vec2(x, y), glm::vec2(w, h), glm::vec2(pivotx, pivoty), rotate);
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

void gTexture::draw(glm::vec2 position, glm::vec2 size, glm::vec2 pivotPointCoords, float rotate) {
	beginDraw();
	imagematrix = glm::translate(imagematrix, glm::vec3(position, 0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

	imagematrix = glm::translate(imagematrix, glm::vec3(pivotPointCoords.x, pivotPointCoords.y, 0.0f));
	imagematrix = glm::rotate(imagematrix, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	imagematrix = glm::translate(imagematrix, glm::vec3(-pivotPointCoords.x, -pivotPointCoords.y, 0.0f));

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

void gTexture::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh, int pivotx, int pivoty, float rotate) {
	drawSub(glm::vec2(x, y), glm::vec2(w, h), glm::vec2(sx, sy), glm::vec2(sw, sh), glm::vec2(pivotx, pivoty), rotate);
}

void gTexture::drawSub(const gRect& src, const gRect& dst, float rotate) {
	drawSub(dst.left(), dst.top(), dst.getWidth(), dst.getHeight(), src.left(), src.top(), src.getWidth(), src.getHeight(), rotate);
}

void gTexture::drawSub(const gRect& src, const gRect& dst, int pivotx, int pivoty, float rotate) {
	drawSub(dst.left(), dst.top(), dst.getWidth(), dst.getHeight(), src.left(), src.top(), src.getWidth(), src.getHeight(), pivotx, pivoty, rotate);
}

void gTexture::drawSub(const gRect& src, const gRect& dst, glm::vec2 pivotPointCoords, float rotate) {
	drawSub(dst.left(), dst.top(), dst.getWidth(), dst.getHeight(), src.left(), src.top(), src.getWidth(), src.getHeight(), pivotPointCoords.x, pivotPointCoords.y, rotate);
}

void gTexture::drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, float rotate) {
	setupRenderData(subpos.x, subpos.y, subsize.x, subsize.y);
	bsubpartdrawn = true;
	draw(pos, size, rotate);
}

void gTexture::drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, glm::vec2 pivotPointCoords, float rotate) {
	setupRenderData(subpos.x, subpos.y, subsize.x, subsize.y);
	bsubpartdrawn = true;
	draw(pos, size, pivotPointCoords, rotate);
}

void gTexture::beginDraw() {
	renderer->getImageShader()->use();
	imagematrix = glm::mat4(1.0f);
	renderer->setProjectionMatrix2d(glm::ortho(0.0f, (float)renderer->getWidth(), (float)renderer->getHeight(), 0.0f, -1.0f, 1.0f));
}

void gTexture::endDraw() {
	renderer->getImageShader()->setMat4("projection", renderer->getProjectionMatrix2d());
	renderer->getImageShader()->setMat4("model", imagematrix);
	renderer->getImageShader()->setVec4("spriteColor", glm::vec4(renderer->getColor()->r, renderer->getColor()->g, renderer->getColor()->b, renderer->getColor()->a));
	renderer->getImageShader()->setInt("image", 0);
	renderer->getImageShader()->setInt("maskimage", 1);
	renderer->getImageShader()->setBool("isAlphaMasking", ismaskloaded);

	G_CHECK_GL(glActiveTexture(GL_TEXTURE0));

	bind();
	if(ismaskloaded) {
		G_CHECK_GL(glActiveTexture(GL_TEXTURE0 + 1)); // GL_TEXTURE1
		masktexture->bind(1);
	}
	if ((format == GL_RGBA || format == GL_RG || ismaskloaded) && !renderer->isAlphaBlendingEnabled()) {
		G_CHECK_GL(glEnable(GL_BLEND));
		G_CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	}

	G_CHECK_GL(glBindVertexArray(quadVAO));
	G_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, 6));
	G_CHECK_GL(glBindVertexArray(0));

	if (((format == GL_RGBA || format == GL_RG || format == GL_RGB) && !renderer->isAlphaBlendingEnabled()) || ismaskloaded) {
		G_CHECK_GL(glDisable(GL_BLEND));
	}
	unbind();
	if(bsubpartdrawn) {	setupRenderData(); }
}

void gTexture::setupRenderData() {
	setupRenderData(0, 0, width, height);
	bsubpartdrawn = false;
}

void gTexture::cleanupAll() {
	if(isloaded) {
		G_CHECK_GL(glDeleteBuffers(1, &quadVBO));
		G_CHECK_GL(glDeleteVertexArrays(1, &quadVAO));
		isloaded = false;
	}
	if(istextureallocated) {
		G_CHECK_GL(glDeleteTextures(1, &id));
		istextureallocated = false;
	}
	cleanupData();
	delete masktexture;
	masktexture = nullptr;
}

void gTexture::cleanupData() {
	if(isstbimage) {
		if (datahdr) {
			stbi_image_free(datahdr);
			datahdr = nullptr;
		} else if (data) {
			stbi_image_free(data);
			data = nullptr;
		}
	} else if(ismutable) {
		delete[] data;
		data = nullptr;
		delete[] datahdr;
		datahdr = nullptr;
	}
}

void gTexture::setupRenderData(int sx, int sy, int sw, int sh) {
	if(!isloaded) {
//		G_CHECK_GL(glDeleteBuffers(1, &quadVBO));
//		G_CHECK_GL(glDeleteVertexArrays(1, &quadVAO));
		G_CHECK_GL(glGenVertexArrays(1, &quadVAO));
		G_CHECK_GL(glGenBuffers(1, &quadVBO));
		isloaded = true;
	}
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

	G_CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, quadVBO));
	if (isfbo || ishdr) {
		G_CHECK_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW));
	} else {
		G_CHECK_GL(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
	}

	G_CHECK_GL(glBindVertexArray(quadVAO));
	G_CHECK_GL(glEnableVertexAttribArray(0));
	G_CHECK_GL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
	G_CHECK_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	G_CHECK_GL(glBindVertexArray(0));
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

