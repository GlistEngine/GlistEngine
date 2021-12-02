/*
 * gSharpen.cpp
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#include <posteffects/gSharpen.h>

gSharpen::gSharpen() {
	shader = renderer->getKernelEffectShader();

}

gSharpen::~gSharpen() {
	// TODO Auto-generated destructor stub
}

void gSharpen::use() {
	float kernel[9] = {
			-1.0f, -1.0f, -1.0f,
			-1.0f,  9.0f, -1.0f,
			-1.0f, -1.0f, -1.0f
	};
	float offset = 1.0f / 300.0f;

	shader->use();
	shader->setFloat("offset", offset);
	for(int i = 0; i < 9; i++) shader->setFloat("kernel[" + gToStr(i) + "]", kernel[i]);
}
