/*
 * gSharpen.cpp
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#include <gSharpen.h>

gSharpen::gSharpen() {
	float kernel[9] = {
			 0.0f, -1.0f,  0.0f,
			-1.0f,  5.0f, -1.0f,
			 0.0f, -1.0f,  0.0f
	};
	float offset = 1.0f / 300.0f;

	shader->use();
	shader->setFloat("offset", offset);
	for(int i = 0; i < 9; i++) shader->setFloat("kernel[" + gToStr(i) + "]", kernel[i]);
}

gSharpen::~gSharpen() {
	// TODO Auto-generated destructor stub
}

void gSharpen::use() {
	shader->use();
}

const std::string gSharpen::getVertSrc() {
	return "";
}

const std::string gSharpen::getFragSrc() {
	return "";
}
