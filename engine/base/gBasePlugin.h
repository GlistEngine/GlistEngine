/*
 * gBasePlugin.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef ENGINE_BASE_GBASEPLUGIN_H_
#define ENGINE_BASE_GBASEPLUGIN_H_

#include "gRenderObject.h"
#include "gBaseApp.h"
#include <deque>

/**
 * This class should be extended whenever you are developing a new plugin for
 * your project.
 *
 * Developers should add their plugins inside ../dev/glist/glistplugins folder.
 * If the folder does not exist, it should be added. The file structure inside
 * of the glistplugins should be like 'glistplugins/gipPlugin'. Every plugin
 * folder should have atleast a src folder with .cpp and .h files of that plugin
 * inside it.
 *
 * Plugins should be classes that help the project by reducing the amount of
 * code that's being repeated. For example, buttons are a way to run certain
 * functions at the click of a mouse on a certain area. In that case a plugin,
 * such as gipButton, can be used to reduce the amount of code that's being
 * written inside the mouse state functions for the current canvas.
 *
 * When naming the plugins, developers are encouraged to add the 'gip' prefix to
 * whichever plugin that's being created. Ex: gipButton, gipMenu, gipTitle.
 * It's important to keep this naming convention, as that makes reading the file
 * structure easier for the whole team if a developer has one. If not, it should
 * still be used for when publishing the code online.
 *
 * gBasePlugin is a class that gets updated by the engine and therefore gets updated
 * at the same time as other classes that has update function.
 *
 * The functions in this class should never be called manually,
 * as that's handled by the engine, and doing so might raise
 * errors that are unknown.
 */
class gBasePlugin : public gRenderObject {
public:
	gBasePlugin();
	virtual ~gBasePlugin();

	/**
	 * Starting point for each plugin that extends this class.
	 *
	 * This function gets called once when this class is instantiated, and
	 * is called after the constructor.
	 *
	 * Almost every initialization that a plugin has to do, should be made
	 * in this function. Certain class functions such as gImage's loadImage,
	 * should also be called in this function as it initializes the pixel data
	 * for the image.
	 *
	 * setup should not be used for re-initalization of variables. Meaning,
	 * this function should not be called anywhere in the code, as it might
	 * raise some unwanted errors that might be hard to track.
	 */
	virtual void setup();

	/**
	 * This function gets called each and every engine update tick. All logic,
	 * except the ones including mouse's states, that's related to
	 * updating the plugin should be written in this function and this function
	 * only.
	 *
	 * For the better usage of this function, every functionality that should be
	 * updated, should get seperated into different functions and should be called
	 * here to allow maintaining the code clean and find errors quicker. Meaning,
	 * this function should only hold (other than some exceptions) function calls.
	 *
	 * If the plugin should be drawn, it should be in a seperate draw function,
	 * then be called in the draw function that belongs to the current canvas.
	 *
	 * Exceptions of not using only functions in update should be conditions, that
	 * allows/disallows functions to be called.
	 */
	virtual void update();

	/**
	 * This function gets the unicode representation of the pressed key as it's
	 * parameter.
	 *
	 * Can be either used for printing or drawing the key that's been pressed, or
	 * for the input controls.
	 *
	 * @param key Unicode representation of the pressed key on the keyboard.
	 */
	virtual void charPressed(wchar_t key);

	/**
	 * Gets called each time a key is pressed on the keyboard.
	 *
	 * If the developer wants the plugin to do something when a key is being pressed,
	 * the functionality should be defined in this function.
	 *
	 * This function should only contain a code that updates a keystate, with some
	 * exceptions. Not doing so might lead to a spaghetti code.
	 *
	 * @param key The keycode of the key that has been pressed.
	 */
	virtual void keyPressed(int key);

	/**
	 * Gets called each time a key is released on the keyboard.
	 *
	 * If the developer wants the plugin to do something when a key is being released,
	 * the functionality should be defined in this function.
	 *
	 * This function should only contain a code that updates a keystate, with some
	 * exceptions. Not doing so might lead to a spaghetti code.
	 *
	 * @param key The keycode of the key that has been released.
	 */
	virtual void keyReleased(int key);

	/**
	 * Gets called every time the mouse moves on the current canvas.
	 *
	 * @param x Mouse's current x coordinate on the canvas space
	 * @param y Mouse's current y coordinate on the canvas space
	 */
	virtual void mouseMoved(int x, int y);

	/**
	 * Gets called every time the mouse is being dragged on the current canvas.
	 *
	 * This function is triggered when the mouse is being moved with a mouse button
	 * currently being pressed. For that, as long as a mouse button is being pressed
	 * while the mouse is being moved on the canvas, this function cancels the trigger
	 * on the mouseMoved() function.
	 *
	 * @param x Mouse's current x coordinate on the canvas space
	 * @param y Mouse's current y coordinate on the canvas space
	 * @param button The mouse button that's being pressed while moving the mouse
	 */
	virtual void mouseDragged(int x, int y, int button);

	/**
	 * Gets called when a mouse button has been pressed inside the current canvas.
	 *
	 * As this function immediately gets triggered when a mouse button is clicked on
	 * the current canvas, if not intended, most click events should be handled inside
	 * the mouseReleased function.
	 *
	 * If an immediate response is needed for an action, use this function instead.
	 *
	 * @param x Mouse's x coordinate when the button was pressed.
	 * @param y Mouse's y coordinate when the button was pressed.
	 * @param button The mouse button that has been pressed.
	 */
	virtual void mousePressed(int x, int y, int button);

	/**
	 * Gets called when a mouse button has been released inside the current canvas.
	 *
	 * This function should be used for the events that needs to be triggered after a
	 * mouse button click, hence when the button is released.
	 *
	 * @param x Mouse's x coordinate when the button was released.
	 * @param y Mouse's y coordinate when the button was released.
	 * @param button The mouse button that has been released.
	 */
	virtual void mouseReleased(int x, int y, int button);

	/**
	 * Gets called when the mouse enters the current canvas.
	 *
	 * This function gets called only once when the cursor enters the current canvas
	 * and does not get called repeatedly while inside the canvas. And for the cursor
	 * to enter the canvas, it has to exit the canvas once.
	 */
	virtual void mouseEntered();

	/**
	 * Gets called when the mouse exits the current canvas.
	 *
	 * This function also gets called once when the cursor exits the current canvas,
	 * and does not get called repeatedly while outside the canvas.
	 */
	virtual void mouseExited();

	/**
	 * All the plugins that extends this class should add themselves into this
	 * deque container in their constructor, as this is not a plugin-specific variable.
	 */
	static std::deque<gBasePlugin*> usedplugins;

private:
};

#endif /* ENGINE_BASE_GBASEPLUGIN_H_ */
