/*
 * gGUITopContainer.h
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIFORM_H_
#define UI_GGUIFORM_H_

#include "gBaseGUIObject.h"
class gGUISizer;


class gGUIForm: public gBaseGUIObject {
public:
	gGUIForm();
	virtual ~gGUIForm();

	void setSizer(gGUISizer* guiSizer);
	gGUISizer* getSizer();

	virtual int getCursor(int x, int y);

	void show();
	void hide();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int key);
	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

protected:
	gGUISizer* guisizer;
	bool isshown;
};

#endif /* UI_GGUIFORM_H_ */
