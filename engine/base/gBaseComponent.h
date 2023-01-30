/*
 * gBaseComponent.h
 *
 *  Created on: Jan 28, 2023
 *      Author: noyan
 */

#ifndef BASE_GBASECOMPONENT_H_
#define BASE_GBASECOMPONENT_H_

#include "gAppObject.h"
#include <deque>


class gBaseComponent: public gAppObject {
public:
	gBaseComponent();
	virtual ~gBaseComponent();

	virtual void setup();
	virtual void update();

	static std::deque<gBaseComponent*> usedcomponents;
};

#endif /* BASE_GBASECOMPONENT_H_ */
