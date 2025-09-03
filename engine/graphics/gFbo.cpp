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
	    float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	        // positions   // texCoords
	        -1.0f,  1.0f,  0.0f, 1.0f,
	        -1.0f, -1.0f,  0.0f, 0.0f,
	         1.0f, -1.0f,  1.0f, 0.0f,

	        -1.0f,  1.0f,  0.0f, 1.0f,
	         1.0f, -1.0f,  1.0f, 0.0f,
	         1.0f,  1.0f,  1.0f, 1.0f
	    };

	    glGenVertexArrays(1, &gFbo::quadVAO);
	    glGenBuffers(1, &gFbo::quadVBO);
	    glBindVertexArray(gFbo::quadVAO);
	    glBindBuffer(GL_ARRAY_BUFFER, gFbo::quadVBO);
	    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	    glEnableVertexAttribArray(0);
	    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	    glEnableVertexAttribArray(1);
	    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	    gFbo::isvaoset = true;
	}
}

gFbo::~gFbo() {
	G_PROFILE_ZONE_SCOPED_N("gFbo::~gFbo()");
	delete texture;
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteVertexArrays(1, &gFbo::quadVAO);
	glDeleteBuffers(1, &gFbo::quadVBO);
}

void gFbo::allocate(int width, int height, bool isDepthMap) {
	G_PROFILE_ZONE_SCOPED_N("gFbo::allocate()");
	G_PROFILE_ZONE_VALUE(width);
	G_PROFILE_ZONE_VALUE(height);
	G_PROFILE_ZONE_VALUE(isDepthMap);
	// check if is not allocated
	if(isallocated) {
		delete texture;
		G_CHECK_GL(glDeleteRenderbuffers(1, &rbo));
		G_CHECK_GL(glDeleteFramebuffers(1, &framebuffer));

		texture = nullptr;
		isallocated = false;
	}

	this->width = width;
	this->height = height;
	isdepthmap = isDepthMap;

	G_CHECK_GL(glGenFramebuffers(1, &framebuffer));
	G_CHECK_GL(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));

	if(!isDepthMap) {
		// create a color attachment texture
		texture = new gTexture(width, height, GL_RGBA, true);
		texture->bind();
		G_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getId(), 0));

		// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
		G_CHECK_GL(glGenRenderbuffers(1, &rbo));
		G_CHECK_GL(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
		G_CHECK_GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height)); // use a single renderbuffer object for both a depth AND stencil buffer.
		G_CHECK_GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo)); // now actually attach it
	} else {
		// create a depth attachment texture
		texture = new gTexture(width, height, GL_DEPTH_COMPONENT, true);
		texture->bind();
		G_CHECK_GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->getId(), 0));
#if defined(GLIST_MOBILE)
		G_CHECK_GL(glDrawBuffers(0, GL_NONE));
#else
		G_CHECK_GL(glDrawBuffer(GL_NONE));
#endif
		G_CHECK_GL(glReadBuffer(GL_NONE));
	}

	// check if fbo complete
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		gLogi("gFbo") << "Framebuffer is not complete! status:" << gToHex(status, 4);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, gFbo::defaultfbo);
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
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, width, height);
}

void gFbo::unbind() {
	G_PROFILE_ZONE_SCOPED_N("gFbo::unbind()");
	glBindFramebuffer(GL_FRAMEBUFFER, gFbo::defaultfbo);
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
