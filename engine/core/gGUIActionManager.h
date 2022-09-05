/*
 * gGUIActionManager.h
 *
 *  Created on: 22 A�u 2022
 *      Author: sevval
 */

#ifndef CORE_GGUIACTIONMANAGER_H_
#define CORE_GGUIACTIONMANAGER_H_

#include <gObject.h>
class gBaseGUIObject;
#include <deque>

/*
 * This class send information to the target classes when an event happend in the
 * source class. It builded with a struct structure.
 *
 * gBaseGUIObject has a static variable that type is gGUIActionManager. When
 * developers derive a class from gBaseGUIObject, they must use this variable
 * to call this class'es functions.
 *
 * All the child classes makes their own operations in onGUIEvent() function which
 * overrided from gBaseGUIObject class.
 *
 */
class gGUIActionManager: public gObject {
public:
	struct Action{
		gBaseGUIObject* sourceControl;
		int sourceEvent;
		gBaseGUIObject* targetControl;
		int targetEvent;
	};

	gGUIActionManager();
	virtual ~gGUIActionManager();

	/*
	 * Adds events to a vector. This function creates a new struct object with
	 * given parameters. Then adds the struct object to the vector. When an event
	 * happened, this should be added to this vector.
	 *
	 * @param srcControl is the source resource class. An event must be happened
	 * in this class and it should effect other classes.
	 *
	 * @param srcEvent is the source classe's event number which defined at the
	 * gGUIEvents class.
	 *
	 * @param dstControl is the destination classes. If an event happened in the
	 * source class and if this event information should be known from other class,
	 * this other class is named by destination class.
	 *
	 * @param dstEvent is the destination classe's event number which defined at
	 * the gGUIEvents class.
	 *
	 * @param value is the variable which developers wants to send to one class to
	 * other class. If it is not given from developer, this parameter will be given
	 * as empty string.
	 *
	 */
	void addAction(gBaseGUIObject* srcControl, int srcEvent, gBaseGUIObject* dstControl, int dstEvent);

	/**
	 * Removes an already added action from the list
	 */
	void removeAction(gBaseGUIObject* srcControl, int srcEvent, gBaseGUIObject* dstControl, int dstEvent);

	/*
	 * Uses for send information from one class to another class. This function
	 * compares the actions vector datas with given parameters. If they match,
	 * then the information is sended.
	 */
	void onGUIEvent(int guiObjectId, int eventType, std::string value1 = "", std::string value2 = "");

private:
	std::deque<Action> actions;
	std::deque<Action> happened;
};

#endif /* CORE_GGUIACTIONMANAGER_H_ */
