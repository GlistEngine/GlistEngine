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
#include "gGUIManager.h"
#include "gFont.h"
class gBaseWindow;
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

	/*
	 * Gets called each time a key is pressed on the keyboard.
	 *
	 * Receives which key is pressed and converts the key value to unicode
	 * represantation.
	 *
	 * Graphics Library Framework(GLFW) handles communication with keyboard and OS
	 * input.
	 * @param key the pressed key value.

	 */
	void onCharEvent(unsigned int key);

	/**
	 * Receives either the keyboard key is pressed, or released. Saves the input of the
	 * user. Then later sends this value to keyReleased or keyPressed methods.
	 *
	 * Later on developers can manipulate these methods and process the input of user.
	 *
	 * Graphics Library Framework(GLFW) handles communication with keyboard and OS
	 * input.
	 *
	 * Note that this method is implemented to return an integer of the pressed key,
	 * developers are encouraged to use onCharEvent method if they want a unicode value
	 * return type.
	 *
	 * @param key the value of user's interacted key.
	 *
	 * @param action info of user's action. either the keyboard key is pressed or
	 * released.
	 */
	void onKeyEvent(int key, int action);

	/**
	 * Receives x and y coordinate of the application window's clicked positon by user.
	 * Then sends x and y coordinates to mouseMoved method. And in addition sends
	 * clicked button value to mouseDragged.
	 *
	 * Later on developers can manipulate these methods and process the input of user.
	 *
	 * Due to the shortcomings of only getting x and y coordinates of mouse,
	 * developers are encouraged to use onMouseButtonEvent for getting button info
	 * addition to the coordinates.
	 *
	 * Users should remember that x and y values are returned in window coordinates.
	 *
	 * Graphics Library Framework(GLFW) handles communication with mouse and OS input.
	 *
	 * @param xpos x coordinate of the application window's clicked position.
	 *
	 * @param ypos y coordinate of the application window's clicked position.
	 */
	void onMouseMoveEvent(double xpos, double ypos);

	/**
	 * Receives x and y coordinate of the application window's clicked position by user.
	 * Either the mouse button is pressed, or released. Saves the input of the user. Then
	 * later sends this value to mouseReleased or mousePressed methods.
	 *
	 * Later on developers can manipulate these methods and process the input of user.
	 *
	 * Users should remember that x and y values are returned in window coordinates.
	 *
	 * Graphics Library Framework(GLFW) handles communication with mouse and OS input.
	 *
	 * @param button which button is being interacted.
	 *
	 * @param action info of user's action. either mouse button is pressed or released.
	 *
	 * @param  xpos x coordinate of the application window's clicked position.
	 *
	 * @param ypos y coordinate of the application window's clicked position.
	 */
	void onMouseButtonEvent(int button, int action, double xpos, double ypos);

	/**
	 * Gets the info of user's cursor. Checks if user is currently on the application
	 * window or not.
	 *
	 * Then sends this info to mouseEntered and mouseExited methods.
	 * Developers can manipulate these methods and process current state. This can be
	 * helpful to understand if user is currently on the application tab or borders,
	 * continuing the game when user is on the tab, and pausing when is not.
	 *
	 * Graphics Library Framework(GLFW) handles communication with mouse and OS input.
	 *
	 * @param info of user's cursor is either in the boundaries of application window
	 * or outside the boundaries.
	 */
	void onMouseEnterEvent(int entered);

	/**
	 * Gets the info of user's cursor. Checks if user is currently on the application
	 * window or not. Then receives offset values x and y, x for horizontal axis and y
	 * for vertical axis. Then sends this values to mouseScrolled method. Developers
	 * can manipulate this method and process current state.
	 *
	 * If x value is greater than 0, mouse is being scrolled to the right. If
	 * x value is less than 0, mouse is being scrolled to the left.
	 *
	 * If y value is greater than 0, mouse is being scrolled up. If y value is less
	 * than 0, mouse is being scrolled down.
	 *
	 * This can be helpful for aim precision, view changing, and smooth controls.
	 *
	 * @param xoffset offset value of x(horizontal) axis.
	 *
	 * @param yoffset offset value of y(vertical) axis.
	 */
	void onMouseScrollEvent(double xoffset, double yoffset);

	void onWindowFocus(bool isFocused);
	void onJoystickConnected(int jid, bool isGamepad, bool isConnected);

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
	bool isJoystickConnected(int jId);
	int getJoystickAxesCount(int jId);
	const float* getJoystickAxes(int jId);
	bool isGamepadEnabled();
	bool isGamepadButtonPressed(int gamepadId, int buttonId);
	int getMaxGamepadNum();
	int getGamepadButtonNum();

	void setWindowSize(int width, int height);
	void setWindowResizable(bool isResizable);
	void setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight);

private:
	static const int maxgamepadnum = 4;
	static const int gamepadbuttonnum = 15;

	using AppClock = std::chrono::steady_clock;
	using AppClockDuration = AppClock::duration;
	using AppClockTimePoint = AppClock::time_point;

	void internalUpdate();
	void preciseSleep(double seconds);

	std::string appname;
	gBaseWindow* window;
	gBaseApp* app;
	int windowmode;
	bool usewindow;
	int loopmode;
	bool loopalways;
	gBaseCanvas* canvas;
	gCanvasManager* canvasmanager;
	gGUIManager* guimanager;
	bool ismouseentered;
	bool buttonpressed[3];
	int pressed;
	int myPow (int x, int p);
	int mpi, mpj;
	AppClockTimePoint starttime, endtime;
	AppClockDuration deltatime;
	AppClockDuration timestepnano;
	AppClockDuration lag;
	double sleeptime;
	long long elapsedtime;
	int updates, draws;
	int framerate;
	int uci, ucj, upi, upj;
	double t_estimate, t_mean, t_m2;
	int64_t t_count;
	double t_observed, t_delta, t_stddev;
	bool canvasset;
	bool iswindowfocused;
	bool isgamepadenabled;
	bool gamepadconnected[maxgamepadnum];
	bool gamepadbuttonstate[maxgamepadnum][gamepadbuttonnum];
	bool gpbuttonstate;
	int joystickhatcount;
	int joystickaxecount;
};

#endif /* ENGINE_CORE_GAPPMANAGER_H_ */
