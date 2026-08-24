/*
 * gFbo.cpp
 *
 *  Created on: 23 Feb 2021
 *      Author: Noyan Culum
 */

#include "gFbo.h"

#include "gTracy.h"

int gFbo::defaultfbo = 0;

gFbo::gFbo() {
	width = 0;
	height = 0;
	framebuffer = 0;
	rbo = 0;
	isdepthmap = false;
	usedepthtexture = false;
	texture = nullptr;
	depthtexture = nullptr;
	isallocated = false;

}

gFbo::~gFbo() {
	G_PROFILE_ZONE_SCOPED_N("gFbo::~gFbo()");
	delete texture;
	delete depthtexture;
	if (!usedepthtexture) renderer->deleteRenderbuffer(rbo);
	renderer->deleteFramebuffer(framebuffer);
}

void gFbo::allocate(int width, int height, bool isDepthMap, bool useDepthTexture) {
	G_PROFILE_ZONE_SCOPED_N("gFbo::allocate()");
	G_PROFILE_ZONE_VALUE(width);
	G_PROFILE_ZONE_VALUE(height);
	G_PROFILE_ZONE_VALUE(isDepthMap);
	// check if is not allocated
	if(isallocated) {
		delete texture;
		delete depthtexture;
		if (!usedepthtexture) renderer->deleteRenderbuffer(rbo);
		renderer->deleteFramebuffer(framebuffer);

		texture = nullptr;
		depthtexture = nullptr;
		isallocated = false;
	}

	this->width = width;
	this->height = height;
	isdepthmap = isDepthMap;
	// Vulkan has no renderbuffer object, so the depth attachment is always a
	// sampleable texture there. The public FBO API is unchanged either way.
	usedepthtexture = useDepthTexture || renderer->isVulkan();

	framebuffer = renderer->createFramebuffer();
	renderer->bindFramebuffer(framebuffer);

	if(!isDepthMap) {
		// create a color attachment texture
		texture = new gTexture(width, height, GL_RGBA, true);
		texture->bind();
		renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getId());

		if(usedepthtexture) {
			// create a sampleable depth texture attachment
			depthtexture = new gTexture(width, height, GL_DEPTH_COMPONENT, true);
			depthtexture->bind();
			renderer->attachTextureToFramebuffer(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtexture->getId());
		} else {
			// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
			rbo = renderer->createRenderbuffer();
			renderer->bindRenderbuffer(rbo);
			renderer->setRenderbufferStorage(GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
			renderer->attachRenderbufferToFramebuffer(GL_DEPTH_STENCIL_ATTACHMENT, rbo); // now actually attach it
		}
	} else {
		// create a depth attachment texture
		texture = new gTexture(width, height, GL_DEPTH_COMPONENT, true);
		texture->bind();
		renderer->attachTextureToFramebuffer(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture->getId());
		renderer->setDrawBufferNone();
		renderer->setReadBufferNone();
	}
#if defined(DEBUG) || defined(ENGINE_OPENGL_CHECKS)
	renderer->checkFramebufferStatus();
#endif
	renderer->bindDefaultFramebuffer();
	isallocated = true;
}

void gFbo::allocateMipChain(int width, int height, int miplevels) {
	if(isallocated) {
		delete texture;
		delete depthtexture;
		if (!usedepthtexture) renderer->deleteRenderbuffer(rbo);
		renderer->deleteFramebuffer(framebuffer);

		texture = nullptr;
		depthtexture = nullptr;
		isallocated = false;
	}

	/*	@brief Depth + Stencil allocation does not implemented
	 * 	This mipchain designed for post process shader
	 */

	this->width = width;
	this->height = height;
	this->miplevels = miplevels;
	isdepthmap = false;
	// A Vulkan pipeline is built against a render pass and can only be recorded
	// into a compatible one, and every pipeline the engine builds is aimed at the
	// screen pass - which carries a depth attachment. So a target with colour alone
	// would be one nothing could draw into. The same reasoning as in allocate(),
	// and like it, OpenGL is left exactly as it was: no depth attachment there,
	// because a post-process chain never tests depth.
	usedepthtexture = renderer->isVulkan();

	framebuffer = renderer->createFramebuffer();
	texture = new gTexture(width, height, miplevels, GL_RGBA, true);

	renderer->bindFramebuffer(framebuffer);
	renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getId(), 0);

	if(usedepthtexture) {
		// One full-size depth attachment for the whole chain: a framebuffer's
		// attachments may be larger than the framebuffer itself, so the smaller
		// levels reuse it rather than each needing one of their own.
		depthtexture = new gTexture(width, height, GL_DEPTH_COMPONENT, true);
		depthtexture->bind();
		renderer->attachTextureToFramebuffer(GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtexture->getId());
		texture->bind();
	}

#if defined(DEBUG) || defined(ENGINE_OPENGL_CHECKS)
	renderer->checkFramebufferStatus();
#endif

	renderer->bindDefaultFramebuffer();
	isallocated = true;
}

void gFbo::bindMip(int level) {
	renderer->bindFramebuffer(framebuffer);
	renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->getId(), level);

	int w = std::max(1, width >> level);
	int h = std::max(1, height >> level);
	renderer->setViewport(0, 0, w, h);
}

unsigned int gFbo::getId() const {
	return framebuffer;
}

int gFbo::getWidth() const {
	return width;
}

int gFbo::getHeight() const {
	return height;
}

unsigned int gFbo::getTextureId() const {
#ifdef DEBUG
	// have you called fbo.allocate(...) ?
	assert(texture);
#endif
	return texture->getId();
}

unsigned int gFbo::getDepthTextureId() const {
#ifdef DEBUG
	assert(depthtexture);
#endif
	return depthtexture->getId();
}

void gFbo::bind() {
	G_PROFILE_ZONE_SCOPED_N("gFbo::bind()");
	renderer->bindFramebuffer(framebuffer);
	renderer->setViewport(0, 0, width, height);
}

void gFbo::unbind() {
	G_PROFILE_ZONE_SCOPED_N("gFbo::unbind()");
	renderer->bindDefaultFramebuffer();
	renderer->setViewport(0, 0, renderer->getScreenWidth(), renderer->getScreenHeight());
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

