/*
* gScreenSpaceReflection.cpp
*
*  Created on: August 5, 2026
*      Author: Hasan Cem Eren
*/

#include "gScreenSpaceReflection.h"

gScreenSpaceReflection::gScreenSpaceReflection() {
	capturenearclip = 0.01f;
	capturefarclip = 1000.0f;

	reflectivity = 0.65f;
	fresnelbias = 0.05f;
	fresnelpower = 4.0f;
	fallbackcolor = glm::vec3(0.55f, 0.57f, 0.6f);

	currentreflectionindex = 0;
	hasvalidhistory = false;
	frameindex = 0;
	previousviewprojection = glm::mat4(1.0f);
}

gScreenSpaceReflection::~gScreenSpaceReflection() {
}

void gScreenSpaceReflection::setup() {
	renderer->disableSSAO();

	ssrshader.load(gObject::gGetShadersDir() + "screenspacereflection_vert.glsl", gObject::gGetShadersDir() + "screenspacereflection_frag.glsl");
	resize(renderer->getScreenWidth(), renderer->getScreenHeight());
}

void gScreenSpaceReflection::resize(int width, int height) {
	scenefbo.allocate(width, height, false, true);

	renderer->bindTexture(scenefbo.getDepthTextureId());
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	renderer->bindTexture(scenefbo.getTextureId());
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	renderer->generateMipMap();

	reflectionfbo[0].allocate(width, height);
	reflectionfbo[1].allocate(width, height);
	renderer->bindTexture(reflectionfbo[0].getTextureId());
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	renderer->bindTexture(reflectionfbo[1].getTextureId());
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	renderer->resetTexture();

	hasvalidhistory = false;
}

void gScreenSpaceReflection::attachTo(gMesh& mesh) {
	mesh.getMaterial()->addShader(&ssrshader);
}

void gScreenSpaceReflection::beginCapture() {
	renderer->activateTexture(8);
	renderer->resetTexture();
	renderer->activateTexture(9);
	renderer->resetTexture();
	renderer->activateTexture(10);
	renderer->resetTexture();
	renderer->activateTexture(11);
	renderer->resetTexture();

	gRenderObject::disableExtraShaders();

	scenefbo.bind();
	renderer->clearScreen(true, true);
}

void gScreenSpaceReflection::captureCameraMatrices(gCamera& camera) {
	captureprojection = renderer->getProjectionMatrix();
	captureview = renderer->getViewMatrix();
	capturenearclip = camera.getNearClip();
	capturefarclip = camera.getFarClip();
}

void gScreenSpaceReflection::endCapture() {
	scenefbo.unbind();

	renderer->bindTexture(scenefbo.getTextureId());
	renderer->generateMipMap();

	gRenderObject::enableExtraShaders();
}

void gScreenSpaceReflection::beginReflectionPass(gSkybox* skybox) {
	int previndex = 1 - currentreflectionindex;

	ssrshader.use();
	ssrshader.setInt("sspass", 0);
	ssrshader.setInt("validhistory", hasvalidhistory ? 1 : 0);
	ssrshader.setMat4("projection", captureprojection);
	ssrshader.setMat4("invview", glm::inverse(captureview));
	ssrshader.setMat4("previousviewprojection", previousviewprojection);
	ssrshader.setVec2("screensize", glm::vec2(scenefbo.getWidth(), scenefbo.getHeight()));
	ssrshader.setFloat("nearclip", capturenearclip);
	ssrshader.setFloat("farclip", capturefarclip);
	ssrshader.setFloat("reflectivity", reflectivity);
	ssrshader.setFloat("fresnelbias", fresnelbias);
	ssrshader.setFloat("fresnelpower", fresnelpower);
	ssrshader.setVec3("fallbackcolor", fallbackcolor);
	ssrshader.setFloat("ditherphase", float(frameindex % 64) * 0.6180339887f);
	frameindex++;

	ssrshader.setInt("scenecolor", 8);
	renderer->activateTexture(8);
	renderer->bindTexture(scenefbo.getTextureId());

	ssrshader.setInt("scenedepth", 9);
	renderer->activateTexture(9);
	renderer->bindTexture(scenefbo.getDepthTextureId());

	ssrshader.setInt("hasskymap", skybox != nullptr ? 1 : 0);
	if(skybox != nullptr) {
		ssrshader.setInt("skymap", 10);
		renderer->bindSkyTexture(skybox->getTextureId(), GL_TEXTURE0 + 10);
	}

	ssrshader.setInt("reflectionbuffer", 11);
	renderer->activateTexture(11);
	renderer->bindTexture(reflectionfbo[previndex].getTextureId());

	renderer->disableAlphaBlending();

	reflectionfbo[currentreflectionindex].bind();
	renderer->clearScreen(true, true);

	renderer->enableDepthTestEqual();
}

void gScreenSpaceReflection::endReflectionPass() {
	renderer->setDepthTestFunc(gRenderer::DEPTHTESTTYPE_LESS);

	reflectionfbo[currentreflectionindex].unbind();
	renderer->enableAlphaBlending();

	previousviewprojection = captureprojection * captureview;
	hasvalidhistory = true;
	currentreflectionindex = 1 - currentreflectionindex;
}

void gScreenSpaceReflection::beginComposite() {
	ssrshader.use();
	ssrshader.setInt("sspass", 1);
	ssrshader.setVec2("screensize", glm::vec2(scenefbo.getWidth(), scenefbo.getHeight()));

	ssrshader.setInt("scenecolor", 8);
	renderer->activateTexture(8);
	renderer->bindTexture(scenefbo.getTextureId());

	ssrshader.setInt("reflectionbuffer", 11);
	renderer->activateTexture(11);
	renderer->bindTexture(reflectionfbo[1 - currentreflectionindex].getTextureId());

	renderer->enableDepthTestEqual();
}

void gScreenSpaceReflection::endComposite() {
	renderer->setDepthTestFunc(gRenderer::DEPTHTESTTYPE_LESS);
}

void gScreenSpaceReflection::setReflectivity(float reflectivity) {
	this->reflectivity = reflectivity;
}

float gScreenSpaceReflection::getReflectivity() const {
	return reflectivity;
}

void gScreenSpaceReflection::setFresnelBias(float fresnelbias) {
	this->fresnelbias = fresnelbias;
}

float gScreenSpaceReflection::getFresnelBias() const {
	return fresnelbias;
}

void gScreenSpaceReflection::setFresnelPower(float fresnelpower) {
	this->fresnelpower = fresnelpower;
}

float gScreenSpaceReflection::getFresnelPower() const {
	return fresnelpower;
}

void gScreenSpaceReflection::setFallbackColor(const glm::vec3& color) {
	fallbackcolor = color;
}

glm::vec3 gScreenSpaceReflection::getFallbackColor() const {
	return fallbackcolor;
}
