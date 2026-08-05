/*
* gPostProcessManager.cpp
 *
 *  Created on: 29 Kas 2021
 *      Author: kayra
 */

#include <gPostProcessManager.h>

const int gPostProcessManager::fbocount = 2;

gPostProcessManager::gPostProcessManager() {
	fbos = nullptr;
	fbotoread = 0;
	fbotowrite = 0;
	lastwrittenfbo = 0;
}

gPostProcessManager::~gPostProcessManager() {
	delete[] fbos;
}

void gPostProcessManager::setDimensions(int width, int height) {
	this->width = width;
	this->height = height;
	fbos = new gFbo[fbocount];
	for(int i = 0; i < fbocount; i++) fbos[i].allocate(width, height, false);
}

void gPostProcessManager::addEffect(gBasePostProcess *effect) {
	effects.push_back(effect);
}

void gPostProcessManager::enable() {
	fbos[0].bind(); // this is where we will draw our affected objects.
	renderer->clearScreen(true, true);
}

void gPostProcessManager::disable() {
	renderer->endSSAO();

	renderer->disableDepthTest();
	fbotoread = 0;
	fbotowrite = 1;

	// Effects sample from slot 0, the scene draws may have left another one active
	renderer->resetTexture();

	for(int i = 0; i < effects.size(); i++) {
		fbos[fbotowrite].bind();
		renderer->clearScreen(true, false);
		effects[i]->render(fbos[fbotoread], fbos[fbotowrite]);
		lastwrittenfbo = fbotowrite;
		int temp = fbotoread;
		fbotoread = fbotowrite;
		fbotowrite = temp;
		}

	renderer->bindDefaultFramebuffer();
	renderer->clearScreen(true, false);

	renderer->getFboShader()->use();
	renderer->bindQuadVAO();
	fbos[lastwrittenfbo].getTexture().bind();
	renderer->drawFullscreenQuad();
}
