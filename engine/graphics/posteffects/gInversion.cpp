/*
 * gInversion.cpp
 *
 *  Created on: 29 Kas 2021
 *      Author: kayra
 */

#include <gInversion.h>

gInversion::gInversion() {
	shader = renderer->getInversionShader();
}

gInversion::~gInversion() {
	// TODO Auto-generated destructor stub
}


void gInversion::use() {
	shader->use();
}
