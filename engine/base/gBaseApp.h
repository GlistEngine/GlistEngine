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
#include "gBaseCanvas.h"


class gBaseApp : public gAppObject {
public:
	gBaseApp();
#ifndef ANDROID
	gBaseApp(int argc, char **argv);
#endif
	virtual ~gBaseApp();

	virtual void setup();
	virtual void update();

#ifdef ANDROID
	virtual void pause();
	virtual void resume();
#endif
	virtual void start();
	virtual void stop();

	void setCurrentCanvas(gBaseCanvas* currentCanvas);
	gBaseCanvas* getCurrentCanvas();

	void setTargetFramerate(int targetFramerate);
	int getFramerate();
	int getTargetFramerate();
	double getElapsedTime();

	void enableVsync();
	void disableVsync();

	static gAppManager* getAppManager() { return appmanager; }
protected:
	int argc;
	char **argv;
};

#endif /* ENGINE_BASE_GBASEAPP_H_ */
