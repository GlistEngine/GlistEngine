/*
 * gEdgeDetection.cpp
 *
 *  Created on: 2 Ara 2021
 *      Author: kayra
 */

#include <gEdgeDetection.h>

gEdgeDetection::gEdgeDetection() {
	float kernel[9] = {
			-1.0f, -1.0f, -1.0f,
			-1.0f,  8.0f, -1.0f,
			-1.0f, -1.0f, -1.0f
	};
	float offset = 1.0f / 300.0f;

	shader->use();
	shader->setFloat("offset", offset);
	for(int i = 0; i < 9; i++) shader->setFloat("kernel[" + gToStr(i) + "]", kernel[i]);
}

gEdgeDetection::~gEdgeDetection() {
	// TODO Auto-generated destructor stub
}

void gEdgeDetection::use() {
	shader->use();
}

const std::string gEdgeDetection::getVertSrc() {
	return "";
}

const std::string gEdgeDetection::getFragSrc() {
	return "";
}
