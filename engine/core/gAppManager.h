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

#ifndef GAPPMANAGER_H
#define GAPPMANAGER_H

#include "gBaseWindow.h"
#include "gEvent.h"
#include "gWindowEvents.h"
#include "gFont.h"
#include "gObject.h"
class gGUIAppThread;

#include <chrono>
#include <iostream>
#include <mutex>
#include "gGUIManager.h"

class gCanvasManager;
class gBaseCanvas;
class gBaseApp;
class gAppManager;
class gGUIFrame;

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
 * update() function should be called manually.
 */
void gStartEngine(gBaseApp* baseApp, const std::string& appName, int loopMode);

extern gAppManager* appmanager;

class gAppManager : public gObject {
public:
    gAppManager(const std::string& appName, gBaseApp *baseApp, int width, int height, int windowMode, int unitWidth, int unitHeight, int screenScaling, bool isResizable, int loopMode);
    ~gAppManager();

	// Main application
	void runApp();

    void initialize();
	void setup();
    void loop();
    void stop();

	/**
     * Submits a function to run on the main loop.
     * Queue is executed after every tick.
     *
     * @param fn Function to submit
     */
	void submitToMainThread(std::function<void()> fn);

	/**
	 * Returns app's name
	 *
	 * @return App name
	 */
	std::string getAppName();

	/**
	 * Getter for the Canvas Manager
	 *
	 * @return Pointer of the Canvas Manager
	 */
	gCanvasManager* getCanvasManager();

	/**
	 * Getter for the GUI Manager
	 *
	 * @return Pointer of the GUI Manager
	 */
	gGUIManager* getGUIManager();

	/**
	 * Returns the loop mode of the engine. The returning value can be
	 * one of G_LOOPMODE_NORMAL(0) or G_LOOPMODE_NONE(1)
	 *
	 * @return Loop mode
	 */
	int getLoopMode();

	/**
	 * Returns if the current window is focused by operating system
	 *
	 * @return Is current window focused
	 */
	bool isWindowFocused();

	/**
	 * Publishes the event to canvas, app and gui manager. Returns if true the event was handled by any of them.
	 *
	 * @param event Event to publish
	 * @return isHandled
	 */
	bool callEvent(gEvent& event);

	/**
	 * Returns a pointer of the current canvas
	 *
	 * @return Current canvas.
	 */
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

	/**
	 * @return Current gGUIFrame.
	 */
	gGUIFrame* getCurrentGUIFrame();

	/**
	 * Function to save a string into the system clipboard. Clipboard string
	 * should be arranged by the app developer.
	 *
	 * @param String to save
	 */
	void setClipboardString(const std::string& clipboard);

	/**
	 * Reads and returns the string saved to clipboard.
	 */
    std::string getClipboardString();

	/**
	 * Possible values are
	 * - WINDOWMODE_GAME,
	 * - WINDOWMODE_FULLSCREEN,
	 * - WINDOWMODE_APP,
	 * - WINDOWMODE_FULLSCREENGUIAPP,
	 * - WINDOWMODE_GUIAPP
	 *
	 * @return Window mode of the application.
	 */
    int getWindowMode() const { return windowmode; }

	gBaseWindow* getWindow() const { return window; }

	void setWindowSize(int width, int height);
	void setWindowResizable(bool isResizable);
	void setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight);

	/**
     * Sets screen size by given width and height. GlistEngine scales the process.
     *
     * @param width new width value of screen.
     *
     * @param height new height value of screen.
     */
	void setScreenSize(int width, int height);

	/**
	 * Completely replace the current gBaseCanvas with the specified gBaseCanvas.
	 *
	 * @param baseCanvas new gBaseCanvas to replace.
	 */
	void setCurrentCanvas(gBaseCanvas* canvas);

	void enableVsync();
	void disableVsync();

	/**
	 * Completely replace the current gGUIFrame with the specified gGUIFrame.
	 *
	 * @param guiFrame new gGUIFrame to replace.
	 */
	void setCurrentGUIFrame(gGUIFrame* guiFrame);

	gGUIAppThread* getGUIAppThread();

    void setCursor(int cursorId);
    void setCursorMode(int cursorMode);

    void setWindowIcon(std::string pngFullpath);
    void setWindowIcon(unsigned char* imageData, int w, int h);

    void setWindowTitle(const std::string& windowTitle);

    bool isJoystickConnected(int joystickId);
    int getJoystickAxesCount(int joystickId);
    const float* getJoystickAxes(int joystickId);
    bool isGamepadEnabled() { return isjoystickenabled; }
    bool isGamepadButtonPressed(int joystickId, int buttonId);
    int getMaxJoystickNum() { return maxjoysticknum; }
    int getMaxJoystickButtonNum() { return maxjoystickbuttonnum; }

    std::string getANSILocale();

#ifdef ANDROID
	DeviceOrientation getDeviceOrientation() { return deviceorientation; }
	void setDeviceOrientation(DeviceOrientation orientation);
#endif
    
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    void iosLoop();
#endif // TARGET_OS_IPHONE || TARGET_OS_SIMULATOR

private:
    static const int maxjoysticknum = 16;
    static const int maxjoystickbuttonnum = 15;
    static const int maxmousebuttonnum = 3;

    using AppClock = std::chrono::steady_clock;
    using AppClockDuration = AppClock::duration;
    using AppClockTimePoint = AppClock::time_point;

    gCanvasManager* canvasmanager;
    gGUIManager* guimanager;

    EventHandlerFn eventhandler;
    gBaseWindow* window;
	bool usewindow;
    bool isrunning;
    bool isrendering;
	bool setupcomplete;
    std::string appname;
    gBaseApp* app;
    int width;
    int height;
    int windowmode;
    int unitwidth;
    int unitheight;
    int screenscaling;
    bool isresizable;
    int loopmode;
    bool initialized;
    bool initializedbefore;
    bool iswindowfocused;
    std::string ansilocale;

    bool ismouseentered;
    bool mousebuttonpressed[maxmousebuttonnum];
    int mousebuttonstate;

    AppClockTimePoint starttime, endtime;
    AppClockDuration deltatime;
    AppClockDuration targettimestep;
    uint64_t totaltime;

    int framerate;
    int totalupdates;
    int totaldraws;
    int targetframerate;
    bool iscanvasset;

    bool isjoystickenabled;
    bool joystickconnected[maxjoysticknum];
    bool joystickbuttonstate[maxjoysticknum][maxjoystickbuttonnum];
    int joystickaxecount;

#if defined(GLIST_MOBILE)
    DeviceOrientation deviceorientation;
    DeviceOrientation olddeviceorientation;
    bool delayedresize;
#endif

    std::vector<std::function<void()>> mainthreadqueue;
    std::mutex mainthreadqueuemutex;

private:

    void tick();
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
#if defined(GLIST_MOBILE)
    bool onAppPauseEvent(gAppPauseEvent&);
    bool onAppResumeEvent(gAppResumeEvent&);
    bool onDeviceOrientationChangedEvent(gDeviceOrientationChangedEvent&);
    bool onTouchEvent(gTouchEvent&);
#endif

    void updateTime();
    void executeQueue();

	EventHandlerFn getEventHandler() { return eventhandler; }

    static void preciseSleep(double seconds);

    gGUIAppThread* guiappthread;
    bool isguiapp;
};

#endif //GAPPMANAGER_H
