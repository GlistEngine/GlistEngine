/*
 * gShadowMap.cpp
 *
 *  Created on: Mar 21, 2021
 *      Author: noyan
 */

#include "gShadowMap.h"
#include "gShader.h"
#include "gTracy.h"

gShadowMap::gShadowMap() {
	isallocated = false;
	isactivated = false;
	isenabled = false;
	camera = nullptr;
	light = nullptr;
	lightprojection = glm::mat4(1.0f);
	lightview = glm::mat4(1.0f);
	lightmatrix = lightprojection * lightview;
	width = 0;
	height = 0;
	shadowmaptextureslot = 9;
	updateshadows = false;
}

gShadowMap::~gShadowMap() {}

void gShadowMap::allocate(gLight* light, gCamera* camera, int width, int height) {
	// The Vulkan backend has no GL context and no shadowmap shader, so instead of
	// the depthfbo below it builds its own depth-only render target and pipeline.
	// Everything after that - the light matrices, the two render passes, the
	// enable/disable pairing - is shared with the OpenGL path.
	//
	// If the backend cannot provide one, isallocated stays false and every entry
	// point of this class turns into a no-op, so a game that asks for shadows
	// renders without them instead of crashing.
	if(renderer->isVulkan()) {
		if(!renderer->allocateShadowMap(width, height)) {
			gLogi("gShadowMap") << "The Vulkan backend could not allocate a shadow map; "
					<< "the scene will be drawn unshadowed.";
			return;
		}
		this->width = width;
		this->height = height;
		isallocated = true;
		this->camera = camera;
		this->light = light;
		lightposition = light->getPosition();
		setLightView(glm::lookAt(lightposition, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0)));
		setLightProjection(glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, 2.0f, 114.0f));
		return;
	}

	this->width = width;
	this->height = height;
	depthfbo.allocate(width, height, true);
	isallocated = true;
	this->camera = camera;
	this->light = light;
	lightposition = light->getPosition();
	setLightView(glm::lookAt(lightposition, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0)));
	setLightProjection(glm::ortho(-40.0f, 40.0f, -40.0f, 40.0f, 2.0f, 114.0f));
}

bool gShadowMap::isAllocated() const {
	return isallocated;
}

int gShadowMap::getWidth() const {
	return width;
}

int gShadowMap::getHeight() const {
	return height;
}

void gShadowMap::update() {
	lightposition = light->getPosition();
	setLightView(glm::lookAt(lightposition, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0)));
	renderpassno = 2;
	updateshadows = true;
}

void gShadowMap::setLight(gLight* light) {
	this->light = light;
	lightposition = light->getPosition();
	setLightView(glm::lookAt(lightposition, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0)));
}

void gShadowMap::setCamera(gCamera* camera) {
	this->camera = camera;
}

gLight* gShadowMap::getLight() const {
	return light;
}

gCamera* gShadowMap::getCamera() const {
	return camera;
}

void gShadowMap::activate() {
	isactivated = true;
	renderpassnum = 2;
	updateshadows = true;
}

void gShadowMap::deactivate() {
	renderpassnum = 1;
	updateshadows = false;
	disable();
	isactivated = false;
}

bool gShadowMap::isActivated() const {
	return isactivated;
}

void gShadowMap::enable() {
	G_PROFILE_ZONE_SCOPED_N("gShadowMap::enable()");
	if (!isallocated || !isactivated) return;

	isenabled = true;
	isshadowmappingenabled = true;

	// The Vulkan path does not switch shaders or bind an FBO here: which pass is
	// being recorded is decided by the frame loop, which opens the shadow render
	// pass for pass 0 and the screen one for pass 1. All this has to do is keep the
	// backend's copy of the light transform current, so the depth pass and the
	// shading pass agree on where the light is.
	if (renderer->isVulkan()) {
		renderer->setShadowMapState(true, lightmatrix, lightposition,
				renderer->isSoftShadowsEnabled());
		return;
	}

	if (updateshadows && renderpassno == 0) {
		glViewport(0, 0, depthfbo.getWidth(), depthfbo.getHeight());
		renderer->getShadowmapShader()->use();
		renderer->getShadowmapShader()->setMat4("lightMatrix", lightmatrix);
		depthfbo.bind();
	//	glViewport(0, 0, width, height);
		renderer->clearScreen(false, true);
	} else {
		glViewport(0, 0, renderer->getScreenWidth(), renderer->getScreenHeight());
		renderer->getColorShader()->use();
		renderer->getColorShader()->setInt("aUseShadowMap", 1);
		renderer->getColorShader()->setVec3("lightPos", lightposition);
		renderer->getColorShader()->setInt("shadowMap", shadowmaptextureslot);

		renderer->bindTexture(depthfbo.getTextureId(), shadowmaptextureslot);
		renderpassno = 1;
	}
}

void gShadowMap::disable() {
	G_PROFILE_ZONE_SCOPED_N("gShadowMap::disable()");
	if (!isallocated || !isactivated || renderpassno > 0) return;

	isenabled = false;
	isshadowmappingenabled = false;

	// Nothing to unbind on Vulkan: there is no bound FBO, and the render pass is
	// closed by the frame loop that opened it.
	if (renderer->isVulkan()) return;

	depthfbo.unbind();
}

bool gShadowMap::isEnabled() const {
	return isenabled;
}

void gShadowMap::setLightProjection(glm::mat4 lightProjection) {
	lightprojection = lightProjection;
	lightmatrix = lightprojection * lightview;
}

void gShadowMap::setLightProjection(float leftx, float rightx, float fronty, float backy, float nearz, float farz) {
	lightprojection = glm::ortho(leftx, rightx, fronty, backy, nearz, farz);
	lightmatrix = lightprojection * lightview;
}

glm::mat4 gShadowMap::getLightProjection() const {
	return lightprojection;
}

void gShadowMap::setLightView(glm::mat4 lightView) {
	lightview = lightView;
	lightmatrix = lightprojection * lightview;
}

glm::mat4 gShadowMap::getLightView() const {
	return lightview;
}

glm::mat4 gShadowMap::getLightMatrix() const {
	return lightmatrix;
}

gFbo& gShadowMap::getDepthFbo() {
	return depthfbo;
}


