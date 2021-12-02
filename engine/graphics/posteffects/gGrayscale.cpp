/*
 * gGrayscale.cpp
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#include <posteffects/gGrayscale.h>

gGrayscale::gGrayscale() {
	shader = renderer->getGrayscaleShader();
}

gGrayscale::~gGrayscale() {
	// TODO Auto-generated destructor stub
}

void gGrayscale::use() {
	shader->use();
}
