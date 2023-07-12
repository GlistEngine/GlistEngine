/*
 * gGUIContainer.h
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#ifndef UI_GGUICONTAINER_H_
#define UI_GGUICONTAINER_H_

#include "gGUIScrollable.h"
#include "gGUISizer.h"


class gGUIContainer: public gGUIScrollable {
public:
	gGUIContainer();
	virtual ~gGUIContainer();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	virtual void set(int x, int y, int w, int h);

	void setSizer(gGUISizer* guiSizer);
	gGUISizer* getSizer();

	virtual int getCursor(int x, int y);
	virtual void keyPressed(int key);
	virtual void keyReleased(int key);
	virtual void charPressed(unsigned int codepoint);
	virtual void mouseMoved(int x, int y);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseDragged(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void mouseScrolled(int x, int y);
	virtual void windowResized(int w, int h);

	void update();
	void draw();

	void setCursorOn(bool isOn);

protected:
	gGUISizer* guisizer;
	int topbarh;
	bool sizerrescaling;
};

#endif /* UI_GGUICONTAINER_H_ */
