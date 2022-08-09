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
	void draw();

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

private:
	int buttoncoverw, buttoncoverh, buttoncoverx, buttoncovery;
	int timeboxw, timeboxh;
	int trianglea, triangleb, trianglec; //These are x axis for triangle button what use gDrawTriangle() function
	int textboxw, textboxh;
	int buttontype;

	bool isdisabled;
	bool ispressedb1, ispressedb2;
	bool ispressednow;
	bool istoggle;

	gGUISizer timeboxsizer;
	gGUIText textcolon;
	gColor bluecolor;
	gColor pressedbluecolor;
	gColor boxcolor;

};

#endif /* UI_GGUITIMEBOX_H_ */
