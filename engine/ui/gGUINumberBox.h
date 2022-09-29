/*
 * gGUINumberBox.h
 *
 *  Created on: 27 Jul 2022
 *      Author: Aliv
 */

#ifndef UI_GGUINUMBERBOX_H_
#define UI_GGUINUMBERBOX_H_

#include "gGUIContainer.h"
#include "gGUITextbox.h"

class gGUINumberBox: public gGUIContainer {
public:
	gGUINumberBox();
	virtual ~gGUINumberBox();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	virtual void set(int x, int y, int w, int h);

	void setText(const std::string& text);

	/*
	 * Sets type of number which will be increased or decreased
	 *
	 * @param texttype for switching between integer and float values
	 */
	bool setType(bool isInteger = true);
	void setSize(int width, int height);
	void showTitle(bool isShown);

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void update();
	void draw();

private:
	int boxwidth, boxheight, numboxwidth, numboxheight, smalboxwidth, smalboxheight;

	bool b1ispressed, b2ispressed;
	bool b1isdisabled, b2isdisabled;
	bool isinteger;

	int lineno, columno;
	int castcurrtexttoint;
	int incboxposx, incboxposy;
	int decboxposx, decboxposy;
	int inctriucorpx, inctriucorpy, inctrilcorpx, inctrilcorpy, inctrircorpx, inctrircorpy;
	int dectriucorpx, dectriucorpy, dectrilcorpx, dectrilcorpy, dectrircorpx, dectrircorpy;

	float castcurrtexttofloat;

	std::string defintvalue, deffloatvalue;
	std::string currenttext;

	gGUITextbox textbox;
	gGUISizer boxsizer;
	bool istitleshown;
	int boxtoph;
	int smalltriangleheight;

};
#endif /* UI_GGUINUMBERBOX_H_ */

