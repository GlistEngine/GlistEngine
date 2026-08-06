/*
 * gBasePostProcess.cpp
 *
 *  Created on: 29 Kas 2021
 *      Author: kayra
 */

#include <gBasePostProcess.h>

gBasePostProcess::gBasePostProcess() {
}

gBasePostProcess::~gBasePostProcess() {
}

gShader* gBasePostProcess::getShader() {
	return shader;
}

void gBasePostProcess::render(gFbo& src, gFbo& dst) {
	dst.bind();
	use();
	renderer->bindQuadVAO();
	src.getTexture().bind();
	renderer->drawFullscreenQuad();
}
