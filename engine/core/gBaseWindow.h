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
#include "gEvent.h"

class gAppManager;

/**
 * gBaseWindow, looks at events for the game window, sets and gets the properties of the game window.
 * It is a base class that's why If who want create window class, have to use this class as parent.
 *
 * Use parent as gObject.
 */

class gBaseWindow : public gObject {
public:

	static const int WINDOWMODE_NONE = -1, WINDOWMODE_GAME = 0, WINDOWMODE_FULLSCREEN = 1, WINDOWMODE_APP = 2, WINDOWMODE_FULLSCREENGUIAPP = 3, WINDOWMODE_GUIAPP = 4;
	static const int CURSOR_ARROW = 0, CURSOR_IBEAM = 1, CURSOR_CROSSHAIR = 2, CURSOR_HAND = 3, CURSOR_HRESIZE = 4, CURSOR_VRESIZE = 5;


	gBaseWindow();
	virtual ~gBaseWindow();

	/**
	 * Sets game window's width, height sizes and window mode.
	 *
	 * @param width Size of game screen width.
	 * @param height Size of game screen height.
	 * @param windowMode Determines which mode the window is in. Example: Window Mode,Game Mode, Fullscreen Mode.
	 */
	virtual void initialize(int width, int height, int windowMode, bool isResizable);
	virtual bool getShouldClose();

	bool isRendering() const;

	/**
	 * Performs the said operations at the specified frame rate.
	 */
	virtual void update();

	/**
	 * Destroys all remaining windows and cursors, restores any modified gamma ramps and frees any other allocated resources.
	 */
	virtual void close();

	bool isVsyncEnabled();
	virtual void setVsync(bool vsync);

	virtual void setCursor(int cursorNo);
	virtual void setCursorMode(int cursorMode);

	virtual void setClipboardString(std::string text);
	virtual std::string getClipboardString() const;

	/**
	 * Sets game window size.
	 *
	 * @param width Size of game screen width.
	 * @param height Size of game screen height.
	 */
	void setSize(int width, int height);
	virtual void setWindowSize(int width, int height);

	/**
	 * Get game window width size.
	 *
	 * Game window's width.
	 */
	int getWidth();

	/**
	 * Get game window height size.
	 *
	 * @return Game window's height.
	 */
	int getHeight();

	/**
	 * The title at the top of the window is changed with the entered value.
	 *
	 * @param windowTitle The text to be written in the title of the window.
	 */
	void setTitle(const std::string& windowTitle);
	void setTitle(std::string&& windowTitle);

	/**
	 * Gets the title at the top of the window.
	 *
	 * @return Returns the title at the top of the window.
	 */
	const std::string& getTitle() const;

	virtual void setWindowResizable(bool isResizable);
	virtual void setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight);

	void setEventHandler(EventHandlerFn handler);

	virtual bool isJoystickPresent(int joystickId);
    virtual bool isJoystickButtonPressed(int joystickId, int buttonId);
	virtual const float* getJoystickAxes(int joystickId, int* axisCountPtr);

public:
	bool vsync;
	void callEvent(gEvent& event);

protected:

	int width, height;
	int scalex, scaley;
	int windowmode;
	std::string title;
	bool isfocused;
	EventHandlerFn eventhandler;
	static void sighandler(int signum);
	static std::string signalname[32];
};

#endif /* ENGINE_BASE_GBASEWINDOW_H_ */
