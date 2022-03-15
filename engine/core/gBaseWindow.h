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
	 * Takes the address of the gAppManager in RAM and assigns it to the appmanager in the class.
	 *
	 * @param *appManager Get from RAM address.
	 */
	void setAppManager(gAppManager *appManager);

	/**
	 * Sets game window's width, height sizes and window mode.
	 *
	 * @param width Size of game screen width.
	 * @param height Size of game screen height.
	 * @param windowMode Determines which mode the window is in. Example: Window Mode,Game Mode, Fullscreen Mode.
	 */
	virtual void initialize(int width, int height, int windowMode);
	virtual bool getShouldClose();

	/**
	 * Performs the said operations at the specified frame rate.
	 */
	virtual void update();

	/**
	 * Destroys all remaining windows and cursors, restores any modified gamma ramps and frees any other allocated resources.
	 */
	virtual void close();

	bool isVsyncEnabled();
	virtual void enableVsync(bool vsync);

	virtual void setCursor(int cursorNo);
	virtual void setCursorMode(int cursorMode);

	/**
	 * Sets game window size.
	 *
	 * @param width Size of game screen width.
	 * @param height Size of game screen height.
	 */
	void setSize(int width, int height);

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

	/*
	 * Gets called each time a key is pressed on the keyboard.
	 *
	 * Receives which key is pressed and converts the key value to unicode
	 * represantation.
	 *
	 * Graphics Library Framework(GLFW) handles communication with keyboard and OS
	 * input.
	 *
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
	 * @param action info of user's action. either mouse button is pressed or released.
	 * @param  xpos x coordinate of the application window's clicked position.
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
	 * @param yoffset offset value of y(vertical) axis.
	 */
	void onMouseScrollEvent(double xoffset, double yoffset);

	bool vsync;

protected:
	gAppManager *appmanager;
	int width, height;
	int windowmode;
	std::string title;

	static void sighandler(int signum);
	static std::string signalname[32];
};

#endif /* ENGINE_BASE_GBASEWINDOW_H_ */
