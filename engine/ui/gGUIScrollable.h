/*
 * gGUIScrollable.h
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#ifndef UI_GGUISCROLLABLE_H_
#define UI_GGUISCROLLABLE_H_

#include "gGUIControl.h"
#include "gFbo.h"


class gGUIScrollable: public gGUIControl {
public:
	gGUIScrollable();
	virtual ~gGUIScrollable();

	void enableScrollbars(bool isVerticalEnabled, bool isHorizontalEnabled);

	void draw();
	virtual void drawContent();
	virtual void drawScrollbars();

	virtual void mouseMoved(int x, int y);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseDragged(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void mouseScrolled(int x, int y);


protected:
	void setDimensions(int width, int height);
	int boxw, boxh;
	int totalh;
	int firstx, firsty;
	int vsbx, vsby, vsbw, vsbh;
	int hsbx, hsby, hsbw, hsbh;

private:
	gFbo* boxfbo;

	bool vsbenabled, hsbenabled, sbenabled;
	bool vsbactive, hsbactive;
	bool iscursoronvsb, iscursoronhsb;
	gColor sbbgcolor, sbfgcolor, sbdragcolor;
	float vsbalpha, hsbalpha;
	float sbalphaboxlimit, sbalphasblimit;
	bool alphablending;
	int vrx, vry, vrw, vrh;
	int hrx, hry, hrw, hrh;
	int scrolldiff;
	int vsbmy;
};

#endif /* UI_GGUISCROLLABLE_H_ */
