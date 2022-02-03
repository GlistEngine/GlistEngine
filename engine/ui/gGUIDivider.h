/*
 * gGUIDivider.h
 *
 *  Created on: Sep 29, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIDIVIDER_H_
#define UI_GGUIDIVIDER_H_

#include "gGUIControl.h"


class gGUIDivider: public gGUIControl {
public:
	enum {
		DIR_HORIZONTAL,
		DIR_VERTICAL
	};

	gGUIDivider();
	virtual ~gGUIDivider();

	void setDirection(int direction);
	int getDirection();

private:
	int direction;
};

#endif /* UI_GGUIDIVIDER_H_ */
