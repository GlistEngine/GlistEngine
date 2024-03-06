/*
 * gShadowMap.cpp
 *
 *  Created on: Mar 21, 2021
 *      Author: noyan
 */

#include "gShadowMap.h"
#include "gShader.h"

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
	if (!isallocated || !isactivated) return;

	isenabled = true;
	isshadowmappingenabled = true;

	if (updateshadows && renderpassno == 0) {
		glViewport(0, 0, depthfbo.getWidth(), depthfbo.getHeight());
		renderer->getShadowmapShader()->use();
		renderer->getShadowmapShader()->setMat4("lightMatrix", lightmatrix);
		depthfbo.bind();
	//	glViewport(0, 0, width, height);
		glClear(GL_DEPTH_BUFFER_BIT);
	} else {
		glViewport(0, 0, renderer->getScreenWidth(), renderer->getScreenHeight());
		renderer->getColorShader()->use();
		renderer->getColorShader()->setInt("aUseShadowMap", 1);
		renderer->getColorShader()->setVec3("lightPos", lightposition);
		renderer->getColorShader()->setVec3("shadowLightPos", lightposition);
		renderer->getColorShader()->setMat4("lightMatrix", lightmatrix);
		renderer->getColorShader()->setInt("shadowMap", shadowmaptextureslot);
		renderer->getColorShader()->setVec3("viewPos", camera->getPosition());
//		renderer->getColorShader()->setVec3("viewPos", glm::vec3(0, 1, 0));

		glActiveTexture(GL_TEXTURE0 + shadowmaptextureslot);
		glBindTexture(GL_TEXTURE_2D, depthfbo.getTextureId());
		renderpassno = 1;
	}
}

void gShadowMap::disable() {
	if (!isallocated || !isactivated || renderpassno > 0) return;

	isenabled = false;
	isshadowmappingenabled = false;

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


