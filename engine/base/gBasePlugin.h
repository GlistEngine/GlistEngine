/*
 * gBasePlugin.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef ENGINE_BASE_GBASEPLUGIN_H_
#define ENGINE_BASE_GBASEPLUGIN_H_

#include "gRenderObject.h"
#include "gBaseApp.h"
#include <deque>


class gBasePlugin : public gRenderObject {
public:
	gBasePlugin(gBaseApp *root);
	virtual ~gBasePlugin();

	virtual void setup();
	virtual void update();

	static std::deque<gBasePlugin*> usedplugins;

private:
	gBaseApp *root;
};

#endif /* ENGINE_BASE_GBASEPLUGIN_H_ */
