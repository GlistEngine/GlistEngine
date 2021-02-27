/*
 * gFbo.cpp
 *
 *  Created on: 23 Þub 2021
 *      Author: Acer
 */

#include <gFbo.h>

gFbo::gFbo() {
}

gFbo::~gFbo() {
}

void gFbo::allocate(int width, int height) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // create a color attachment texture
    colortexture = gTexture(width, height);
    colortexture.bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colortexture.getId(), 0);
/*
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
*/

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) loge("Framebuffer is not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int gFbo::getId() {
	return framebuffer;
}

void gFbo::bind() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void gFbo::unbind() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void gFbo::draw(int x, int y) {
	colortexture.draw(x, y);
}

void gFbo::draw(int x, int y, int w, int h) {
	colortexture.draw(x, y, w, h);
}

void gFbo::draw(int x, int y, int w, int h, float rotate){
	colortexture.draw(x, y, w, h, rotate);
}

void gFbo::draw(glm::vec2 position, glm::vec2 size, float rotate) {
	colortexture.draw(position, size, rotate);
}

void gFbo::drawSub(int x, int y, int sx, int sy, int sw, int sh) {
	colortexture.drawSub(x, y, sx, sy, sw, sh);
}

void gFbo::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh) {
	colortexture.drawSub(x, y, w, h, sx, sy, sw, sh);
}

void gFbo::drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh, float rotate) {
	colortexture.drawSub(x, y, w, h, sx, sy, sw, sh, rotate);
}

void gFbo::drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, float rotate) {
	colortexture.drawSub(pos, size, subpos, subsize, rotate);
}

void gFbo::drawSub(const gRect& src, const gRect& dst, float rotate) {
	colortexture.drawSub(src, dst, rotate);
}

