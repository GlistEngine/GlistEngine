/*
 * gipPlugin.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef PLUGINS_GIPEMPTY_SRC_GIPPLUGIN_H_
#define PLUGINS_GIPEMPTY_SRC_GIPPLUGIN_H_

#include "gBasePlugin.h"


class gipPlugin : public gBasePlugin {
public:
	gipPlugin(gBaseApp* root);
	~gipPlugin();

	void setup();
	void update();
};

#endif /* PLUGINS_GIPEMPTY_SRC_GIPPLUGIN_H_ */
