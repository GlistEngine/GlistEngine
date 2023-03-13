/*
 * gBaseApp.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef ENGINE_BASE_GBASEAPP_H_
#define ENGINE_BASE_GBASEAPP_H_

#include "gAppObject.h"
#include "gAppManager.h"


class gBaseApp : public gAppObject {
public:
	gBaseApp();
	gBaseApp(int argc, char **argv);
	virtual ~gBaseApp();

	virtual void setup();
	virtual void update();

	void setAppManager(gAppManager* appManager);
	gAppManager* getAppManager();

	void setCurrentCanvas(gBaseCanvas* currentCanvas);
	gBaseCanvas* getCurrentCanvas();

	void setFramerate(int targetFramerate);
	int getFramerate();
	double getElapsedTime();

	void enableVsync();
	void disableVsync();

protected:
	gAppManager *appmanager;
	int argc;
	char **argv;
};

#endif /* ENGINE_BASE_GBASEAPP_H_ */
