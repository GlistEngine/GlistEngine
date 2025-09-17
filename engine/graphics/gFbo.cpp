/*
 * gFbo.cpp
 *
 *  Created on: 23 �ub 2021
 *      Author: Acer
 */

#include "gFbo.h"

#include "gTracy.h"

bool gFbo::isvaoset = false;
unsigned int gFbo::quadVAO = 0;
unsigned int gFbo::quadVBO = 0;
int gFbo::defaultfbo = 0;

gFbo::gFbo() {
	width = 0;
	height = 0;
	framebuffer = 0;
	isdepthmap = false;
	texture = nullptr;
	isallocated = false;

	if(!isvaoset) {
		renderer->createFullscreenQuad(gFbo::quadVAO, gFbo::quadVBO);
	    gFbo::isvaoset = true;
	}
}

gFbo::~gFbo() {
	G_PROFILE_ZONE_SCOPED_N("gFbo::~gFbo()");
	delete texture;
	renderer->deleteRenderbuffer(rbo);
	renderer->deleteFramebuffer(framebuffer);
	renderer->deleteVAO(gFbo::quadVAO);
	renderer->deleteBuffer(gFbo::quadVBO);
}

void gFbo::allocate(int width, int height, bool isDepthMap) {
	G_PROFILE_ZONE_SCOPED_N("gFbo::allocate()");
	G_PROFILE_ZONE_VALUE(width);
	G_PROFILE_ZONE_VALUE(height);
	G_PROFILE_ZONE_VALUE(isDepthMap);
	// check if is not allocated
	if(isallocated) {
		delete texture;
		renderer->deleteRenderbuffer(rbo);
		renderer->deleteFramebuffer(framebuffer);

		texture = nullptr;
		isallocated = false;
	}

	this->width = width;
	this->height = height;
	isdepthmap = isDepthMap;

	framebuffer = renderer->createFramebuffer();
	renderer->bindFramebuffer(framebuffer);

	if(!isDepthMap) {
		// create a color attachment texture
		texture = new gTexture(width, height, GL_RGBA, true);
		texture->bind();
		renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getId());

		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		rbo = renderer->createRenderbuffer();
		renderer->bindRenderbuffer(rbo);
		renderer->setRenderbufferStorage(GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
		renderer->attachRenderbufferToFramebuffer(GL_DEPTH_STENCIL_ATTACHMENT, rbo); // now actually attach it
	} else {
		// create a depth attachment texture
		texture = new gTexture(width, height, GL_DEPTH_COMPONENT, true);
		texture->bind();
		renderer->attachTextureToFramebuffer(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->getId());
		renderer->setDrawBufferNone();
		renderer->setReadBufferNone();
	}
	renderer->checkFramebufferStatus();
	renderer->bindDefaultFramebuffer();
	isallocated = true;
}

unsigned int gFbo::getId() {
	return framebuffer;
}

int gFbo::getWidth() {
	return width;
}

int gFbo::getHeight() {
	return height;
}

unsigned int gFbo::getTextureId() {
#ifdef DEBUG
	// have you called fbo.allocate(...) ?
	assert(texture);
#endif
	return texture->getId();
}

void gFbo::bind() {
	G_PROFILE_ZONE_SCOPED_N("gFbo::bind()");
	renderer->bindFramebuffer(framebuffer);
	glViewport(0, 0, width, height);
}

void gFbo::unbind() {
	G_PROFILE_ZONE_SCOPED_N("gFbo::unbind()");
	renderer->bindDefaultFramebuffer();
	glViewport(0, 0, renderer->getScreenWidth(), renderer->getScreenHeight());
}

void gFbo::draw(int x, int y) {
	if(isdepthmap) return;
	texture->draw(x, y);
}

void gFbo::draw(int x, int y, int w, int h) {
	if(isdepthmap) return;
	texture->draw(x, y, w, h);
}

void gFbo::draw(int x, int y, int w, int h, float rotate){
	if(isdepthmap) return;
	texture->draw(x, y, w, h, rotate);
}

void gFbo::draw(glm::vec2 position, glm::vec2 size, float rotate) {
	if(isdepthmap) return;
	texture->draw(position, size, rotate);
}

void gFbo::drawSub(int x, int y, int sx, int sy, int sw, int sh) {
	if(isdepthmap) return;
	texture->drawSub(x, y, sx, sy, sw, sh);
}

void gFbo::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh) {
	if(isdepthmap) return;
	texture->drawSub(x, y, w, h, sx, sy, sw, sh);
}

void gFbo::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh, float rotate) {
	if(isdepthmap) return;
	texture->drawSub(x, y, w, h, sx, sy, sw, sh, rotate);
}

void gFbo::drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, float rotate) {
	if(isdepthmap) return;
	texture->drawSub(pos, size, subpos, subsize, rotate);
}

void gFbo::drawSub(const gRect& src, const gRect& dst, float rotate) {
	if(isdepthmap) return;
	texture->drawSub(src, dst, rotate);
}

gTexture& gFbo::getTexture() {
#ifdef DEBUG
	assert(texture); // Texture cannot be null since this function returns a reference.
#endif
	return *texture;
}

unsigned int gFbo::getQuadVao() {
	return gFbo::quadVAO;
}
