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
	gBaseApp(int argc, char **argv);
	virtual ~gBaseApp();

	virtual void setup();
	virtual void update();

	/**
	 * Called after window is initialized and all resources
	 * have been reallocated (if required).
	 */
	virtual void start();
	/**
	 * Called right before renderer and other gpu resources
	 * is destroyed.
	 */
	virtual void stop();

	void setCurrentCanvas(gBaseCanvas* currentCanvas);
	gBaseCanvas* getCurrentCanvas();

	/**
	 * @param framerate Target frames per second value
	 */
	void setTargetFramerate(int framerate);

	/**
	 * @return Target frames per second value
	 */
	int getTargetFramerate();

	/**
	 * @return Current frames per second value.
	 */
	int getFramerate();

	/**
	 * @return Elapsed time between this frame and the previous one, as seconds. Also known as deltaTime.
	 */
	double getElapsedTime();

	void enableVsync();
	void disableVsync();

	static gAppManager* getAppManager() { return appmanager; }
	static void setAppManager(gAppManager* appManager) { appmanager = appManager; }

	static gGUIManager* getGUIManager() { return appmanager->getGUIManager(); }

protected:
	int argc;
	char **argv;
};

#endif /* ENGINE_BASE_GBASEAPP_H_ */
