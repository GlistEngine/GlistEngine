/*
 * gShadowMap.cpp
 *
 *  Created on: Mar 21, 2021
 *      Author: noyan
 */

#include <gShadowMap.h>

gShadowMap::gShadowMap() {
	isallocated = false;
	isactivated = false;
	isenabled = false;
	lightprojection = glm::mat4(1.0f);
	lightview = glm::mat4(1.0f);
	lightmatrix = lightprojection * lightview;
	lightposition = glm::vec3(0.0f);
	viewposition = glm::vec3(0.0f);
	width = 0;
	height = 0;
	shadowmaptextureslot = 9;
}

gShadowMap::~gShadowMap() {}

void gShadowMap::allocate(int width, int height) {
	this->width = width;
	this->height = height;
	depthfbo.allocate(width, height, true);
	isallocated = true;
}

bool gShadowMap::isAllocated() {
	return isallocated;
}

int gShadowMap::getWidth() {
	return width;
}

int gShadowMap::getHeight() {
	return height;
}

void gShadowMap::activate() {
	isactivated = true;
	renderpassnum = 2;
}

void gShadowMap::deactivate() {
	renderpassnum = 1;
	disable();
	isactivated = false;
}

bool gShadowMap::isActivated() {
	return isactivated;
}

void gShadowMap::enable() {
	if (!isallocated || !isactivated) return;

	isenabled = true;
	isshadowmappingenabled = true;

	if (renderpassno == 0) {
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
		renderer->getColorShader()->setVec3("viewPos", viewposition);
//		renderer->getColorShader()->setVec3("viewPos", glm::vec3(0, 1, 0));

		glActiveTexture(GL_TEXTURE0 + shadowmaptextureslot);
		glBindTexture(GL_TEXTURE_2D, depthfbo.getTextureId());
	}
}

void gShadowMap::disable() {
	if (!isallocated || !isactivated || renderpassno > 0) return;

	isenabled = false;
	isshadowmappingenabled = false;

	depthfbo.unbind();
}

bool gShadowMap::isEnabled() {
	return isenabled;
}

void gShadowMap::setLightPosition(glm::vec3 lightPosition) {
	lightposition = lightPosition;
}

glm::vec3 gShadowMap::getLightPosition() {
	return lightposition;
}

void gShadowMap::setLightProjection(glm::mat4 lightProjection) {
	lightprojection = lightProjection;
	lightmatrix = lightprojection * lightview;
}

glm::mat4 gShadowMap::getLightProjection() {
	return lightprojection;
}

void gShadowMap::setLightView(glm::mat4 lightView) {
	lightview = lightView;
	lightmatrix = lightprojection * lightview;
}

glm::mat4 gShadowMap::getLightView() {
	return lightview;
}

glm::mat4 gShadowMap::getLightMatrix() {
	return lightmatrix;
}

void gShadowMap::setViewPosition(glm::vec3 viewPosition) {
	viewposition = viewPosition;
}

glm::vec3 gShadowMap::getViewPosition() {
	return viewposition;
}

gFbo& gShadowMap::getDepthFbo() {
	return depthfbo;
}


