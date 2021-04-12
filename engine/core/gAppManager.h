/****************************************************************************
 * Copyright (c) 2014 Nitra Games Ltd., Istanbul, Turkey                    *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice should not be      *
 * deleted from the source form of the Software.                            *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 * Author: Noyan Culum, 2014-on                                             *
 ****************************************************************************/

/*
 * gAppManager.h
 *
 *  Created on: May 6, 2014
 *      Author: noyan
 */

#ifndef ENGINE_CORE_GAPPMANAGER_H_
#define ENGINE_CORE_GAPPMANAGER_H_

#include "gObject.h"
#include <iostream>
#include <chrono>
class gBaseWindow;
class gBaseApp;
class gBaseCanvas;
class gCanvasManager;


void gStartEngine(gBaseApp* baseApp, std::string appName = "GlistApp", int windowMode = 2, int width = gDefaultWidth(), int height = gDefaultHeight());
void gStartEngine(gBaseApp* baseApp, std::string appName, int windowMode, int width, int height, int screenScaling, int unitWidth, int unitHeight);


class gAppManager : gObject {
public:
	gAppManager();
	virtual ~gAppManager();

	void runApp(std::string appName, gBaseApp *baseApp, int width, int height, int windowMode, int unitWidth, int unitHeight, int screenScaling);
	void setWindow(gBaseWindow * baseWindow);
	void setCurrentCanvas(gBaseCanvas *baseCanvas);
	gBaseCanvas* getCurrentCanvas();
	void setScreenSize(int width, int height);

	void setFramerate(int targetFramerate);
	int getFramerate();
	double getElapsedTime();

	std::string getAppName();

	void onKeyEvent(int key, int action);
	void onMouseMoveEvent(double xpos, double ypos);
	void onMouseButtonEvent(int button, int action, double xpos, double ypos);
	void onMouseEnterEvent(int entered);

	gCanvasManager* getCanvasManager();

private:
	std::string appname;
	gBaseWindow* window;
	gBaseApp* app;
	gBaseCanvas* canvas;
	gCanvasManager* canvasmanager;
	gBaseCanvas* tempbasecanvas;
	bool ismouseentered;
	bool buttonpressed[3];
	int pressed;
	int myPow (int x, int p);
	int mpi, mpj;
	std::chrono::high_resolution_clock::time_point starttime;
	std::chrono::duration<double, std::milli> timediff, timediff2;
	float millisecondsperframe, delaycoef;
	std::chrono::duration<double, std::milli> minWorkTime, delay;
	int framerate;
	int upi, upj;
};

#endif /* ENGINE_CORE_GAPPMANAGER_H_ */
