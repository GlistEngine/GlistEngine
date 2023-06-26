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

#include "gBaseWindow.h"
#include "gEvent.h"
#include "gWindowEvents.h"
#include "gFont.h"
#include "gGUIManager.h"
#include "gObject.h"
#include <chrono>
#include <iostream>

class gBaseApp;
class gBaseCanvas;
class gGUIFrame;
class gCanvasManager;


/**
 * Sets the app settings for the engine according to given name, mode of window,
 * application window's widh and height. Then calls the runApp method to give
 * output to the screen. GlistEngine has scaling methods. User and developer can
 * have different resolutions, but the engine can adapt to users resolution by
 * scaling. In this function, screen scaling is done automatically
 * by the Engine. If developers want to set the scaling, they are encouraged to
 * use overrided gStartEngine function instead.
 *
 * @param baseApp Current app object.
 *
 * @param appName Name of the application.
 *
 * @param windowMode Type of the application window. This parameter can be one of
 * G_WINDOWMODE_GAME, G_WINDOWMODE_APP, G_WINDOWMODE_FULLSCREEN, G_WINDOWMODE_GUIAPP
 * and G_WINDOWMODE_FULLSCREENGUIAPP. This value can not be G_WINDOWMODE_NONE as
 * the app needs a window to work. The developer who needs a windowless console app
 * should use other gStartEngine function. This function defaults to G_WINDOWMODE_APP
 * in case of no window.
 *
 * @param width application window's width to be shown on the user's computer. User
 * should choose this parameter suitable according to their screen resolution
 * to prevent distorted images. Please note that some operating systems may scale
 * the framebuffer and create a bigger window than the developer's value.
 *
 * @param height application window's width to be shown on the user's computer.
 * User should choose this parameter suitable according to their screen resolution
 * to prevent distorted images. Please note that some operating systems may scale
 * the framebuffer and create a bigger window than the developer's value.
 */
void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int width, int height, bool isResizable = true);

/**
 * Sets the app settings for engine according to given name, mode of window,
 * application window's widh and height. Beside that, this is an overrided
 * method of gStartEngine, user should give scaling and unit parameters
 * addition to the baseApp object, app name, windowmode, width and height.
 *
 * Users are encouraged to use this function if they want to scale the
 * output.
 *
 * @param baseApp Current app object.
 *
 * @param appName Name of the application.
 *
 * @param windowMode Type of the application window. This parameter can be one of
 * G_WINDOWMODE_GAME, G_WINDOWMODE_APP, G_WINDOWMODE_FULLSCREEN, G_WINDOWMODE_GUIAPP
 * and G_WINDOWMODE_FULLSCREENGUIAPP. This value can not be G_WINDOWMODE_NONE as
 * the app needs a window to work. The developer who needs a windowless console app
 * should use other gStartEngine function. This function defaults to G_WINDOWMODE_APP
 * in case of no window.
 *
 * @param unitWidth width of the developer's application window. The resolution can
 * differ from developer to user. GlistEngine has scaling methods. User and
 * developer can have different resolutions, but the engine can adapt to users
 * resolution by scaling. Thus, unitWidth is developer's unit width.
 *
 * @param unitHeight height of the developer's application window. The resolution
 * can differ from developer to user. GlistEngine has scaling methods. User and
 * developer can have different resolutions, but the engine can adapt to users
 * resolution by scaling. Thus, unitHeight is developer's unit height.
 *
 * @param screenScaling how much everything should be enlarged when measured in
 * pixels.
 *
 * @param width application window's width to be shown on the user's computer. User
 * should choose this parameter suitable according to their screen resolution
 * to prevent distorted images. Please note that some operating systems may scale
 * the framebuffer and create a bigger window than the developer's value.
 *
 * @param height application window's width to be shown on the user's computer.
 * User should choose this parameter suitable according to their screen resolution
 * to prevent distorted images. Please note that some operating systems may scale
 * the framebuffer and create a bigger window than the developer's value.
 */
void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int unitWidth, int unitHeight, int screenScaling, int width, int height, bool isResizable = true);

/**
 * Creates a windowless console/service app.
 *
 * Developers should not forget to remove all calls to Canvas classes, plugins and
 * other rendering elements from the code and from the CMakeLists.
 *
 * The app's loop mode can be one of G_LOOPMODE_NORMAL and G_LOOPMODE_NONE.
 *
 * If the loopmode is G_LOOPMODE_NONE and if the developer needs a component to update,
 * he should invode the component's update() function manually.
 */
void gStartEngine(gBaseApp* baseApp, const std::string& appName, int loopMode);

/**
 * This class controls basically everything which is shown to user from beginning
 * to end. Process starts with gStartEngine function. gStartEngine gets the input
 * values and sends it to the runApp function. runApp then executes application
 * window with desired settings such as app name, width, height and if not sent,
 * a default gBaseApp object.
 *
 * gAppManager is synchronous. Thus, keeps getting updated every frame by engine
 * and keeps track of changes and inputs. Keeping track of current canvas,
 * getting user inputs, tracking elapsed time are some features of gAppManager.
 *
 * All the inputs of user is received instantly by methods of this class. This
 * updating process continues until the app is closed.
 *
 * gAppManager also handles synchronous draw function. If the canvas is not empty,
 * drawing process continues until the app is closed.
 *
 * Developers should be careful when working with this class. Any wrong input can
 * cause a huge difference in the app. Double checking the gStartEngine function
 * before doing anything else is recommended.
 *
 * Methods of this class should not be called directly from anywhere else, this may
 * cause unexpected errors, excluding gStartEngine, which is a free function
 */
class gAppManager : gObject {
public:

	/**
	 * GUIAPP and FULLSCREENGUIAPP is in progress. Do not use them for instance.
	 */
	static const int WINDOWMODE_NONE = -1, WINDOWMODE_GAME = 0, WINDOWMODE_FULLSCREEN = 1, WINDOWMODE_APP = 2, WINDOWMODE_FULLSCREENGUIAPP = 3, WINDOWMODE_GUIAPP = 4;

	/**
	 * Constructor of gAppManager class. Constructor is automatically called when
	 * object(instance of class) is created. A constructor will have exact same name as
	 * the class and it does not have any return type at all, not even void.
	 * Constructors can be very useful for setting initial values for certain member
	 * variables. In gAppManager constructor, inital values are set.
	 */
	gAppManager();

    /**
	 * Gathers all the settings from gStartEngine functions and creates a new window
	 * with given app name, width, height and window mode. Constructs a temp canvas
	 * with baseApp object. Then starts giving output to the user's screen. The
	 * starting time is being held for computing elapsed time and frame calculations.
	 *
	 * Setup method is executed just before the first output. Generally, setup method
	 * is useful for initializing a variable. When declaring a variable, variable must
	 * be initialized before doing anything else. Then for each frame main loop is
	 * executed.
	 * Main loop includes update and draw functions which is the most important part of
	 * the code. These functions are synchronous, update function is generally used for
	 * variable updates, and draw function for drawing textures.
	 *
	 * Developers should avoid any mathematical operations or declaring variables in
	 * the draw function. Since this function is executed each frame, this can cause
	 * performance loss. App continues to run unless user closes the app. Therefore
	 * any memory leaks should be avoided.
	 *
	 * @param appName Name of the application.
	 *
	 * @param baseApp Current app object.
	 *
	 * @param width application window's width to be shown on the user's computer. User
	 * should choose this parameter suitable according to their screen resolution
	 * to prevent distorted images.
	 *
	 * @param height application window's width to be shown on the user's computer.
	 * User should choose this parameter suitable according to their screen resolution
	 * to prevent distorted images.
	 *
	 * @param windowMode Type of the application window. Developers are encouraged
	 * to use this parameter to be able to see both codes and game window while
	 * developing apps, then later on switching for a suiting mode.
	 *
	 * @param unitWidth width of the developer's application window. The resolution can
     * differ from developer to user. GlistEngine has scaling methods. User and
     * developer can have different resolutions, but the engine can adapt to users
     * resolution by scaling. Thus, unitWidth is developer's unit width.
	 *
	 * @param unitHeight height of the developer's application window. The resolution
     * can differ from developer to user. GlistEngine has scaling methods. User and
     * developer can have different resolutions, but the engine can adapt to users
     * resolution by scaling. Thus, unitHeight is developer's unit height.
	 *
	 * @param screenScaling how much everything should be enlarged when measured in
	 * pixels.
	 */
	void runApp(const std::string& appName, gBaseApp *baseApp, int width, int height, int windowMode, int unitWidth, int unitHeight, int screenScaling, bool isResizable, int loopMode);

	/**
	 * Completely replace the current gBaseWindow with the specified gBaseWindow.
	 *
	 * @param baseWindow new gBaseWindow to replace.
	 */
	void setWindow(gBaseWindow * baseWindow);

	/**
	 * Returns the window mode of the application.
	 *
	 * @return window mode.
	 */
	int getWindowMode();

	void setCursor(int cursorId);
	void setCursorMode(int cursorMode);

	/**
	 * Completely replace the current gBaseCanvas with the specified gBaseCanvas.
	 *
	 * @param baseCanvas new gBaseCanvas to replace.
	 */
	void setCurrentCanvas(gBaseCanvas *baseCanvas);

	/**
	 * Returns the current Canvas.
	 *
	 * @return current Canvas.
	 */
	gBaseCanvas* getCurrentCanvas();

	/**
	 * Completely replace the current gGUIFrame with the specified gGUIFrame.
	 *
	 * @param guiFrame new gGUIFrame to replace.
	 */
	void setCurrentGUIFrame(gGUIFrame *guiFrame);

	/**
	 * Returns the current gGUIFrame.
	 *
	 * @return current gGUIFrame.
	 */
	gGUIFrame* getCurrentGUIFrame();

	/**
	 * Sets screen size by given width and height. GlistEngine scales the process.
	 *
	 * @param width new width value of screen.
	 *
	 * @param height new height value of screen.
	 */
	void setScreenSize(int width, int height);

	/**
	 * Sets current framerate into the given targetFramerate value.
	 *
	 * @param targetFramerate desired new frame rate value.
	 */
	void setFramerate(int targetFramerate);

	/**
	 * Returns current Framerate value.
	 *
	 * @return Framerate value.
	 */
	int getFramerate();

	/**
	 * Returns elapsed time between frames.
	 *
	 * @return elapsed time between each frame.
	 */
	double getElapsedTime();

	/**
	 * Returns current app name.
	 *
	 * @return app's name.
	 */
	std::string getAppName();

	/**
	 * Returns the current gCanvasManager object pointer.
	 *
	 * @return gCanvasManager object pointer.
	 */
	gCanvasManager* getCanvasManager();

	/**
	 * Returns the current gGUIManager object pointer.
	 *
	 * @return gGUIManager object pointer.
	 */
	gGUIManager* getGUIManager();

	void enableVsync();
	void disableVsync();

	void setClipboardString(std::string text);
	std::string getClipboardString();

	/**
	 * Returns the loop mode of the engine. The returning value can be
	 * one of G_LOOPMODE_NORMAL(0) or G_LOOPMODE_NONE(1)
	 *
	 * @return Loop mode
	 */
	int getLoopMode();

	bool isWindowFocused();

	EventHandlerFn getEventHandler();

	bool isJoystickConnected(int joystickId);
	int getJoystickAxesCount(int joystickId);
	const float* getJoystickAxes(int joystickId);
	bool isGamepadEnabled() { return isjoystickenabled; }
	bool isJoystickButtonPressed(int joystickId, int buttonId);
	int getMaxJoystickNum() { return maxjoysticknum; }
	int getMaxJoystickButtonNum() { return maxjoystickbuttonnum; }

	void setWindowSize(int width, int height);
	void setWindowResizable(bool isResizable);
	void setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight);

private:
	static const int maxjoysticknum = 4;
	static const int maxjoystickbuttonnum = 15;
	static const int maxmousebuttonnum = 3;

	using AppClock = std::chrono::steady_clock;
	using AppClockDuration = AppClock::duration;
	using AppClockTimePoint = AppClock::time_point;

	void internalUpdate();
	void preciseSleep(double seconds);
	void onEvent(gEvent& event);

	bool onWindowResizedEvent(gWindowResizeEvent&);
	bool onCharTypedEvent(gCharTypedEvent&);
	bool onKeyPressedEvent(gKeyPressedEvent&);
	bool onKeyReleasedEvent(gKeyReleasedEvent&);
	bool onMouseMovedEvent(gMouseMovedEvent&);
	bool onMouseButtonPressedEvent(gMouseButtonPressedEvent&);
	bool onMouseButtonReleasedEvent(gMouseButtonReleasedEvent&);
	bool onWindowMouseEnterEvent(gWindowMouseEnterEvent&);
	bool onWindowMouseExitEvent(gWindowMouseExitEvent&);
	bool onMouseScrolledEvent(gMouseScrolledEvent&);
	bool onWindowFocusEvent(gWindowFocusEvent&);
	bool onWindowLoseFocusEvent(gWindowLoseFocusEvent&);
	bool onJoystickConnectEvent(gJoystickConnectEvent&);
	bool onJoystickDisconnectEvent(gJoystickDisconnectEvent&);


	EventHandlerFn eventhandler;
	std::string appname;
	gBaseWindow* window;
	gBaseApp* app;
	int windowmode;
	bool usewindow;
	int loopmode;
	bool loopalways;
	gCanvasManager* canvasmanager;
	gGUIManager* guimanager;
	bool ismouseentered;
	bool mousebuttonpressed[maxmousebuttonnum];
	int pressed;
	AppClockTimePoint starttime, endtime;
	AppClockDuration deltatime;
	AppClockDuration timestepnano;
	AppClockDuration lag;
	double sleeptime;
	uint64_t elapsedtime;
	int updates, draws;
	int framerate;
	int uci, ucj, upi, upj;
	double t_estimate, t_mean, t_m2;
	int64_t t_count;
	double t_observed, t_delta, t_stddev;
	bool canvasset;
	bool iswindowfocused;
	bool isjoystickenabled;
	bool joystickconnected[maxjoysticknum];
	bool joystickbuttonstate[maxjoysticknum][maxjoystickbuttonnum];
	bool gpbuttonstate;
	int joystickhatcount;
	int joystickaxecount;
};

#endif /* ENGINE_CORE_GAPPMANAGER_H_ */
