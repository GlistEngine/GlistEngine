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
 * This class should be extended whenever you are developing
 * a new plugin for your project.
 *
 * The functions in this class should never be called manually,
 * as that's handled by the engine, and doing so might raise
 * errors that are unknown.
 *
 * Example:
 * void setup() {
 *     update();
 * }
 *
 * This usage is prone to errors, and should be avoided.
 */
class gBasePlugin : public gRenderObject {
public:
	gBasePlugin();
	virtual ~gBasePlugin();

	/**
	 * Starting point for each plugin that extends this class.
	 *
	 * This function gets called once when this class is instantiated, and
	 * is called after the constructor. Almost every initialization that a
	 * plugin has to do, should be made in this function.
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
	 * Exceptions of not using only functions in update should be conditions, that
	 * allows/disallows functions to be called.
	 */
	virtual void update();

	/**
	 * Gets called each time a key is pressed on the keyboard.
	 *
	 * Events that are related to a key being pressed, should be handled in this
	 * function. As this function is called synchronous to the other keyPressed()
	 * functions, events should be handled consciously with that knowledge as mapping
	 * the same keys to different functionalities will cause problems.
	 *
	 * @param key The keycode of the key that has been pressed.
	 */
	virtual void keyPressed(int key);

	/**
	 * Gets called each time a key is released on the keyboard.
	 *
	 * Events that are related to a key being released, should be handled in this
	 * function. As this function is called synchronous to the other keyReleased()
	 * functions, events should be handled consciously with that knowledge as mapping
	 * the same keys to different functionalities will cause problems.
	 *
	 * @param key The keycode of the key that has been released.
	 */
	virtual void keyReleased(int key);

	/**
	 * Gets called every time the mouse moves on the current canvas.
	 *
	 * This function DOES NOT get triggered when the mouse is moved on the current
	 * plugin, but instead, it gets triggered when the mouse is moved on the current
	 * canvas.
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
	 * the mouseReleased function. If an immediate response is needed for an action,
	 * use this function instead.
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
	 * deque container.
	 */
	static std::deque<gBasePlugin*> usedplugins;

private:
};

#endif /* ENGINE_BASE_GBASEPLUGIN_H_ */
