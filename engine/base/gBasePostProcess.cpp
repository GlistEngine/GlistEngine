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
