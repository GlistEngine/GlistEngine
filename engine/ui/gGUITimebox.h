/*
 * gGUITimebox.h
 *
 *  Created on: 4 Aug 2022
 *      Author: Baris Karakaya
 */

#ifndef UI_GGUITIMEBOX_H_
#define UI_GGUITIMEBOX_H_

#include "gGUIContainer.h"
#include "gGUITextbox.h"
#include "gGUIText.h"

class gGUITimebox: public gGUIContainer {
public:
	gGUITimebox();
	virtual ~gGUITimebox();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo,int parentSlotColumnNo, int x, int y, int w, int h);
	void set(int x, int y, int w, int h);

	void draw();
	void update();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
    void mouseDragged(int x, int y, int button);

private:
	int buttoncoverw, buttoncoverh, buttoncoverx, buttoncovery;
	int timeboxw, timeboxh;
	//These are x axis for triangle button what use gDrawTriangle() function...
	int triangleax, trianglebx, trianglecx;
	//These are y axis for triangle button what use gDrawTriangle() function...
	int triangleay1, triangleby1, trianglecy1;
	int triangleay2, triangleby2, trianglecy2;
	int buttontype;
	int hour, minute, second;

	int lineno,columno;
	//Buttons control...
	bool isdisabled;
	bool ispressedb1, ispressedb2;
	bool ispressednow;
	bool istoggle;

	bool ishour, isminute, issecond;
	/*
	 * All private functions are increasing an decreasing numbers...
	 * */
	void setHour();
	void setMinute();
	void setSecond();

	gGUISizer timeboxsizer;
	gGUITextbox hourbox, minutebox, secondbox;
	gColor bluecolor;
	gColor whitecolor;

};

#endif /* UI_GGUITIMEBOX_H_ */
