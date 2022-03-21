/*
 * gFbo.cpp
 *
 *  Created on: 23 �ub 2021
 *      Author: Acer
 */

#include "gFbo.h"

bool gFbo::isvaoset = false;
unsigned int gFbo::quadVAO = 0;
unsigned int gFbo::quadVBO = 0;

gFbo::gFbo() {
	width = 0;
	height = 0;
	framebuffer = 0;
	isdepthmap = false;
	textureid = 0;

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
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteVertexArrays(1, &gFbo::quadVAO);
	glDeleteBuffers(1, &gFbo::quadVBO);
}

void gFbo::allocate(int width, int height, bool isDepthMap) {
	this->width = width;
	this->height = height;
	isdepthmap = isDepthMap;

    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    if (!isDepthMap) {
        // create a color attachment texture
        texture = gTexture(width, height, GL_RGBA, true);
        textureid = texture.getId();
        texture.bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.getId(), 0);

        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    } else {
        texture = gTexture(width, height, GL_DEPTH_COMPONENT, true);
        textureid = texture.getId();
        texture.bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture.getId(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // check if fbo complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) loge("Framebuffer is not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	return textureid;
}

void gFbo::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, width, height);
}

void gFbo::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, renderer->getWidth(), renderer->getHeight());
}

void gFbo::draw(int x, int y) {
	if(isdepthmap) return;
	texture.draw(x, y);
}

void gFbo::draw(int x, int y, int w, int h) {
	if(isdepthmap) return;
	texture.draw(x, y, w, h);
}

void gFbo::draw(int x, int y, int w, int h, float rotate){
	if(isdepthmap) return;
	texture.draw(x, y, w, h, rotate);
}

void gFbo::draw(glm::vec2 position, glm::vec2 size, float rotate) {
	if(isdepthmap) return;
	texture.draw(position, size, rotate);
}

void gFbo::drawSub(int x, int y, int sx, int sy, int sw, int sh) {
	if(isdepthmap) return;
	texture.drawSub(x, y, sx, sy, sw, sh);
}

void gFbo::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh) {
	if(isdepthmap) return;
	texture.drawSub(x, y, w, h, sx, sy, sw, sh);
}

void gFbo::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh, float rotate) {
	if(isdepthmap) return;
	texture.drawSub(x, y, w, h, sx, sy, sw, sh, rotate);
}

void gFbo::drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, float rotate) {
	if(isdepthmap) return;
	texture.drawSub(pos, size, subpos, subsize, rotate);
}

void gFbo::drawSub(const gRect& src, const gRect& dst, float rotate) {
	if(isdepthmap) return;
	texture.drawSub(src, dst, rotate);
}

const gTexture& gFbo::getTexture() const {
	return texture;
}

unsigned int gFbo::getQuadVao() {
	return gFbo::quadVAO;
}
