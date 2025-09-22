/*
 * gTexture.cpp
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#include "gTexture.h"
#include <iostream>
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#include "gPlane.h"
#include "gShader.h"
#include "gTracy.h"

#if defined(GLIST_MOBILE)
// todo alternatives?
static const int texturewrap[4] = {GL_REPEAT, GL_NEAREST, GL_NEAREST, GL_NEAREST};
static const int texturefilter[4] = {GL_LINEAR, GL_NEAREST, GL_NEAREST, GL_NEAREST};
#else
static const int texturewrap[4] = {GL_REPEAT, GL_CLAMP, GL_CLAMP_TO_EDGE, GL_NEAREST};
static const int texturefilter[4] = {GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST, GL_CLAMP};
#endif

gTexture::gTexture() {
	id = GL_NONE;
	internalformat = GL_RGBA;
	format = GL_RGBA;
	wraps = TEXTUREWRAP_REPEAT;
	wrapt = TEXTUREWRAP_REPEAT;
	filtermin = TEXTUREMINMAGFILTER_MIPMAPLINEAR;
	filtermag = TEXTUREMINMAGFILTER_LINEAR;
	type = TEXTURETYPE_DIFFUSE;
	path = "";
	width = 0;
	height = 0;
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
    int valuetype = GL_UNSIGNED_BYTE;
    internalformat = format;
    this->format = format;
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    if(format != GL_RGBA || format != GL_RGB)
    {
        if(format == GL_DEPTH_COMPONENT)
        {
            internalformat = GL_DEPTH_COMPONENT24;
            valuetype = GL_UNSIGNED_INT;
        }
        if(format == GL_DEPTH_STENCIL)
        {
            internalformat = GL_DEPTH24_STENCIL8;
            valuetype = GL_UNSIGNED_INT_24_8;
        }
    }
#endif
	wraps = TEXTUREWRAP_REPEAT;
	wrapt = TEXTUREWRAP_REPEAT;
	filtermin = TEXTUREMINMAGFILTER_LINEAR;
	filtermag = TEXTUREMINMAGFILTER_LINEAR;
	type = TEXTURETYPE_DIFFUSE;
	path = "";
	width = w;
	height = h;
	ismutable = false;
	isstbimage = false;
	isfbo = isFbo;
	ishdr = false;
	ismaskloaded = false;
	isloaded = false;
	masktexture = nullptr;
	if (format == GL_RED) componentnum = 1;
	else if (format == GL_GREEN) componentnum = 1;
	else if (format == GL_BLUE) componentnum = 1;
	else if (format == GL_RG) componentnum = 2;
	else if (format == GL_RGB) componentnum = 3;
	else if (format == GL_RGBA) componentnum = 4;
    data = nullptr;
	datahdr = nullptr;
	id = renderer->createTextures();
	istextureallocated = true;
	bind();

	renderer->texImage2D(GL_TEXTURE_2D, internalformat, width, height, format, valuetype, nullptr);

	// TODO: BEFORE SHADOWMAP GL_REPEAT
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
	                                      texturefilter[filtermin], texturefilter[filtermag]);
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
		id = renderer->createTextures();
		istextureallocated = true;
	}

	if (ishdr) {
		stbi_set_flip_vertically_on_load(true);
		float* datahdr = stbi_loadf(fullpath.c_str(), &width, &height, &componentnum, 0);
		setDataHDR(datahdr, false, true);
	} else {
		unsigned char* data = stbi_load(fullpath.c_str(), &width, &height, &componentnum, 0);
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
		id = renderer->createTextures();
		istextureallocated = true;
	}

	setData(textureData, isMutable, isStbImage);

	//	setupRenderData();
	return id;
}

void gTexture::setData(unsigned char* textureData, bool isMutable, bool isStbImage, bool clean) {
	if(clean) {
		cleanupData();
	}

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
		renderer->texImage2D(GL_TEXTURE_2D, format, width, height, format, GL_UNSIGNED_BYTE, data);
		renderer->generateMipMap();
		renderer->texImage2D(GL_TEXTURE_2D, format, getWidth(), getHeight(), format, GL_UNSIGNED_BYTE, data);
		renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_REPEAT, GL_REPEAT,
		                                     texturefilter[filtermin], texturefilter[filtermag]);

		if (format == GL_RG) {
			GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
			renderer->setSwizzleMask(swizzleMask);
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
	if(clean) {
		cleanupData();
	}

	ismutable = isMutable;
	isstbimage = isStbImage;
	ishdr = true;
	datahdr = textureData;
	if (datahdr) {
		bind();
		renderer->texImage2D(GL_TEXTURE_2D, GL_RGB32F, width, height, GL_RGB, GL_FLOAT, datahdr); // note how we specify the texture's data value to be float
		renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
		                                     texturefilter[filtermin], texturefilter[filtermag]);
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
	G_PROFILE_ZONE_SCOPED_N("gTexture::bind()");
	renderer->bindTexture(id);
}

void gTexture::bind(int textureSlotNo) const {
	G_PROFILE_ZONE_SCOPED_N("gTexture::bind(int)");
	renderer->bindTexture(id, textureSlotNo);
}

void gTexture::unbind() const {
	G_PROFILE_ZONE_SCOPED_N("gTexture::unbind()");
	renderer->unbindTexture();
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

void gTexture::setType(TextureType textureType) {
	type = textureType;
}

gTexture::TextureType gTexture::getType() const {
	return type;
}

void gTexture::setWrapping(int wrapS, int wrapT) {
	wraps = wrapS;
	wrapt = wrapT;
	bind();
	renderer->setWrapping(GL_TEXTURE_2D, texturewrap[wraps], texturewrap[wrapt]);
	unbind();
}

void gTexture::setFiltering(int minFilter, int magFilter) {
	filtermin = minFilter;
	filtermag = magFilter;
	bind();
	renderer->setFiltering(GL_TEXTURE_2D, texturefilter[filtermin], texturefilter[filtermag]);
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
	return getTypeName(type);
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
	issubpart = false;
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
	draw(position, size, glm::vec2(0.5f * size.x, 0.5f * size.y), rotate);
}

void gTexture::draw(glm::vec2 position, glm::vec2 size, glm::vec2 pivot, float rotate) {
	G_PROFILE_ZONE_SCOPED_N("gTexture::draw()");
	issubpart = false;
	beginDraw();
	imagematrix = glm::translate(imagematrix, glm::vec3(position, 0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

	imagematrix = glm::translate(imagematrix, glm::vec3(pivot.x, pivot.y, 0.0f));
	imagematrix = glm::rotate(imagematrix, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	imagematrix = glm::translate(imagematrix, glm::vec3(-pivot.x, -pivot.y, 0.0f));

	imagematrix = glm::scale(imagematrix, glm::vec3(size.x, size.y, 1.0f));
	endDraw();
}

void gTexture::drawSub(int x, int y, int sx, int sy, int sw, int sh) {
	drawSub(x, y, sw, sh, sx, sy, sw, sh);
}

void gTexture::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh) {
	drawSub(x, y, w, h, sx, sy, sw, sh, 0.0f);
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

void gTexture::drawSub(const gRect& src, const gRect& dst, glm::vec2 pivot, float rotate) {
	drawSub(dst.left(), dst.top(), dst.getWidth(), dst.getHeight(), src.left(), src.top(), src.getWidth(), src.getHeight(), pivot.x, pivot.y, rotate);
}

void gTexture::drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subPos, glm::vec2 subSize, float rotate) {
	drawSub(pos, size, subPos, subSize, glm::vec2(size.x / 2.0f, size.y / 2.0f), rotate);
}

void gTexture::drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subPos, glm::vec2 subSize, glm::vec2 pivot, float rotate) {
	issubpart = true;
	glm::vec2 texturesize = glm::vec2(getWidth(), getHeight());
	subscale = texturesize / subSize;
	subpos = subscale * (subPos / texturesize);

	beginDraw();
	imagematrix = glm::translate(imagematrix, glm::vec3(pos, 0.0f));  // first translate (transformations are: scale happens first, then rotation, and then final translation happens; reversed order)

	imagematrix = glm::translate(imagematrix, glm::vec3(pivot.x, pivot.y, 0.0f));
	imagematrix = glm::rotate(imagematrix, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	imagematrix = glm::translate(imagematrix, glm::vec3(-pivot.x, -pivot.y, 0.0f));

	imagematrix = glm::scale(imagematrix, glm::vec3(size.x, size.y, 1.0f));
	endDraw();
}

void gTexture::beginDraw() {
	G_PROFILE_ZONE_SCOPED_N("gTexture::beginDraw()");
	renderer->getImageShader()->use();
	imagematrix = glm::mat4(1.0f);
	renderer->setProjectionMatrix2d(glm::ortho(0.0f, (float)renderer->getWidth(), (float)renderer->getHeight(), 0.0f, -1.0f, 1.0f));
}

void gTexture::endDraw() {
	G_PROFILE_ZONE_SCOPED_N("gTexture::endDraw()");
	renderer->getImageShader()->setMat4("projection", renderer->getProjectionMatrix2d());
	renderer->getImageShader()->setMat4("model", imagematrix);
	renderer->getImageShader()->setVec4("spriteColor", glm::vec4(renderer->getColor()->r, renderer->getColor()->g, renderer->getColor()->b, renderer->getColor()->a));
	renderer->getImageShader()->setInt("image", 0);
	renderer->getImageShader()->setInt("maskimage", 1);
	renderer->getImageShader()->setBool("isAlphaMasking", ismaskloaded);
	renderer->getImageShader()->setBool("isSubPart", issubpart);
	if (issubpart) {
		renderer->getImageShader()->setVec2("subPos", subpos);
		renderer->getImageShader()->setVec2("subScale", subscale);
	}

	renderer->resetTexture();

	bind();
	renderer->getImageShader()->setBool("isAlphaMasking", ismaskloaded);
	if(ismaskloaded) {
		renderer->getImageShader()->setInt("maskimage", 1);
		renderer->activateTexture(1); // GL_TEXTURE1
		masktexture->bind(1);
	}
	if ((format == GL_RGBA || format == GL_RG || ismaskloaded) && !renderer->isAlphaBlendingEnabled()) {
		renderer->enableAlphaBlending();
	}

	renderer->bindVAO(quadVAO);
	renderer->drawFullscreenQuad();
	renderer->unbindVAO();

	if (((format == GL_RGBA || format == GL_RG || format == GL_RGB) && !renderer->isAlphaBlendingEnabled()) || ismaskloaded) {
		renderer->disableAlphaBlending();
	}
	unbind();
}

void gTexture::setupRenderData() {
	setupRenderData(0, 0, width, height);
}

void gTexture::cleanupAll() {
	G_PROFILE_ZONE_SCOPED_N("gTexture::cleanupAll()");
	if(isloaded) {
		renderer->deleteBuffer(quadVBO);
		renderer->deleteVAO(quadVAO);
		isloaded = false;
	}
	if(istextureallocated) {
		renderer->deleteTexture(id);
		istextureallocated = false;
	}
	cleanupData();
	delete masktexture;
	masktexture = nullptr;
}

void gTexture::cleanupData() {
	G_PROFILE_ZONE_SCOPED_N("gTexture::cleanupData()");
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
	G_PROFILE_ZONE_SCOPED_N("gTexture::setupRenderData()");
	if(!isloaded) {
		quadVAO = renderer->createVAO();
		quadVBO = renderer->genBuffers();
		isloaded = true;
	}
	if (isfbo || ishdr) {
		float vertices[] = {
				// pos      // tex
				0.0f, 1.0f, (float)sx / width, (float)(sy) / height,
				1.0f, 0.0f, (float)(sx + sw) / width, (float)(sy + sh) / height,
				0.0f, 0.0f, (float)sx / width, (float)(sy + sh) / height,

				0.0f, 1.0f, (float)sx / width, (float)(sy) / height,
				1.0f, 1.0f, (float)(sx + sw) / width, (float)(sy) / height,
				1.0f, 0.0f, (float)(sx + sw) / width, (float)(sy + sh) / height
		};
		renderer->setVertexBufferData(quadVBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	} else {
		float vertices[] = {
				// pos      // tex
				0.0f, 1.0f, (float)sx / width, (float)(sy + sh) / height,
				1.0f, 0.0f, (float)(sx + sw) / width, (float)sy / height,
				0.0f, 0.0f, (float)sx / width, (float)sy / height,

				0.0f, 1.0f, (float)sx / width, (float)(sy + sh) / height,
				1.0f, 1.0f, (float)(sx + sw) / width, (float)(sy + sh) / height,
				1.0f, 0.0f, (float)(sx + sw) / width, (float)sy / height
		};
		renderer->setVertexBufferData(quadVBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
	}

	renderer->bindVAO(quadVAO);
	renderer->enableVertexAttrib(0);
	renderer->setVertexAttribPointer(0, 4, GL_FLOAT, false, 4 * sizeof(float), (void*)0);

	renderer->unbindBuffer(GL_ARRAY_BUFFER);
	renderer->unbindVAO();
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

void gTexture::save(std::string fullpath) {
	G_PROFILE_ZONE_SCOPED_N("gTexture::save()");
	unsigned char* pixels = new unsigned char[width * height * componentnum];

	renderer->readTexturePixels(pixels, id, width, height, format);

	// flip back vertically if fbo or hdr
	if (isfbo || ishdr) {
		unsigned char* temppix = new unsigned char[width * componentnum];
		int linenum = height / 2;
		for(int i = 0; i < linenum; i++) {
			int afirst = i * width * componentnum;
			int alast = afirst + width * componentnum;
			int bfirst = (height - i - 1) * width * componentnum;
			int blast = bfirst + width * componentnum;
			std::copy(pixels + afirst, pixels + alast, temppix);
			std::copy(pixels + bfirst, pixels + blast, pixels + afirst);
			std::copy(temppix, temppix + (width * componentnum), pixels + bfirst);
		}
		delete[] temppix;
	}

	stbi_write_png(fullpath.c_str(), width, height, componentnum, pixels, width * componentnum * sizeof(unsigned char));
	delete[] pixels;
}

void gTexture::saveTexture(std::string fileName) {
	save(gGetTexturesDir() + fileName);
}

const std::string& gTexture::getTypeName(TextureType textureType) {
	static const std::string diffuse = "texture_diffuse";
	static const std::string specular = "texture_specular";
	static const std::string normal = "texture_normal";
	static const std::string height = "texture_height";
	static const std::string albedo = "texture_albedo";
	static const std::string roughness = "texture_roughness";
	static const std::string metalness = "texture_metalness";
	static const std::string ao = "texture_ao";
	static const std::string unknown = "texture_unknown";
	switch (textureType) {
	case TEXTURETYPE_DIFFUSE:
		return diffuse;
	case TEXTURETYPE_SPECULAR:
		return specular;
	case TEXTURETYPE_NORMAL:
	case TEXTURETYPE_PBR_NORMAL:  // Both return the same string
		return normal;
	case TEXTURETYPE_HEIGHT:
		return height;
	case TEXTURETYPE_PBR_ALBEDO:
		return albedo;
	case TEXTURETYPE_PBR_ROUGHNESS:
		return roughness;
	case TEXTURETYPE_PBR_METALNESS:
		return metalness;
	case TEXTURETYPE_PBR_AO:
		return ao;
	default:
		return unknown;
	}
}