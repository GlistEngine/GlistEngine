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
	// Android specific functions

	/**
	 * Called when current activity is invisible.
	 * Application will stop rendering after this but will
	 * still receive updates.
	 */
	virtual void pause();
	/**
	 * Called when current activity is visible again.
	 * Application will continue rendering.
	 */
	virtual void resume();
#endif
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
	int getTargetFramerate() const;

	/**
	 * @return Current frames per second value.
	 */
	int getFramerate() const;

	/**
	 * @return Elapsed time between this frame and the previous one, as seconds. Also known as deltaTime.
	 */
	double getElapsedTime() const;

	void enableVsync();
	void disableVsync();

	static gAppManager* getAppManager() { return appmanager; }
protected:
	int argc;
	char **argv;
};

#endif /* ENGINE_BASE_GBASEAPP_H_ */
