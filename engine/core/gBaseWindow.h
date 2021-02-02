/*
 * gBaseWindow.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef ENGINE_BASE_GBASEWINDOW_H_
#define ENGINE_BASE_GBASEWINDOW_H_

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include "gObject.h"

class gAppManager;


class gBaseWindow : public gObject {
public:
	static const int WINDOWMODE_NONE = -1, WINDOWMODE_GAME = 0, WINDOWMODE_FULLSCREEN = 1, WINDOWMODE_APP = 2;

	gBaseWindow();
	virtual ~gBaseWindow();

	void setAppManager(gAppManager *appManager);

	virtual void initialize(int width, int height, int windowMode);
	virtual bool getShouldClose();
	virtual void update();
	virtual void close();

	void setSize(int width, int height);
	int getWidth();
	int getHeight();
	void setTitle(const std::string& windowTitle);
	const std::string& getTitle();

	void onKeyEvent(int key, int action);
	void onMouseMoveEvent(double xpos, double ypos);
	void onMouseButtonEvent(int button, int action, double xpos, double ypos);
	void onMouseEnterEvent(int entered);

protected:
	gAppManager *appmanager;
	int width, height;
	int windowmode;
	std::string title;

	static void sighandler(int signum);
	static std::string signalname[32];
};

#endif /* ENGINE_BASE_GBASEWINDOW_H_ */
