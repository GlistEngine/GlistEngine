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
	// setDimensions() owns the allocation. Without it there is nothing to bind, and
	// fbos[0] below would read through a null array.
	if(fbos == nullptr) return;
	if (this->width != renderer->getScreenWidth() || this->height != renderer->getScreenHeight()) {
			this->width = renderer->getScreenWidth();
			this->height = renderer->getScreenHeight();

			if (fbos != nullptr) {
				for(int i = 0; i < fbocount; i++) {
					fbos[i].allocate(width, height, false);
				}
			}
		}
		fbos[0].bind();
		renderer->clearScreen(true, true);
}

void gPostProcessManager::disable() {
	// Mirrors the guard in enable(): nothing was bound, so nothing is unbound.
	if(fbos == nullptr) return;
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

	// The final resolve to the screen. Both backends create this shader - OpenGL in
	// gRenderer::init(), Vulkan in gVKRenderEngine::init() - but a backend that
	// failed to build it must not be dereferenced through a null.
	gShader* fboshader = renderer->getFboShader();
	if(fboshader == nullptr) return;
	fboshader->use();
	renderer->bindQuadVAO();
	fbos[lastwrittenfbo].getTexture().bind();
	renderer->drawFullscreenQuad();
}
