/*
 * gPostProcessManager.h
 *
 *  Created on: 29 Kas 2021
 *      Author: kayra
 */

#ifndef CORE_GPOSTPROCESSMANAGER_H_
#define CORE_GPOSTPROCESSMANAGER_H_

#include "gFbo.h"
#include "gObject.h"
#include "gBasePostProcess.h"

class gPostProcessManager : public gObject{
public:
	gPostProcessManager();
	virtual ~gPostProcessManager();

	void setDimensions(int width, int height);
	void addEffect(gBasePostProcess *effect);

	void enable();
	void disable();

	static gPostProcessManager *getInstance();
private:
	static const int fbocount;

	static gPostProcessManager* instance;
	std::vector<gBasePostProcess*> effects;

	gFbo *fbos;
	int fbotoread, fbotowrite, lastwrittenfbo;

	int width, height;
};

#endif /* CORE_GPOSTPROCESSMANAGER_H_ */
