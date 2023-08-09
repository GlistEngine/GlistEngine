/*
 * gGUITable.h
 *
 *  Created on: 28 Tem 2023
 *       Author: Nazim Gultekin
 */

#ifndef UI_GGUITABLE_H_
#define UI_GGUITABLE_H_

#include "gImage.h"
#include "gGUIControl.h"
#include "gFont.h"
#include "gGUISizer.h"
#include "gBaseGUIObject.h"
#include "gColor.h"
#include "gGUIFrame.h"
#include "gBaseCanvas.h"

class gGUITable : public gGUIControl{
public:
	gGUITable();
	virtual ~gGUITable();

	void setup();
	void update();
	void draw();
	//void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void addItem(gImage* image, std::string title = "New File");
	void setFontColor(gColor color);
	void setBackgroundTableColor(gColor color);
	gColor* getBackgroundTableColor();
	gColor* getFontColor();
	//gGUISizer tablesizer;

private:
	bool isfilepressed;
	int pressedfileid;
	static const int FILE_X = 0, FILE_Y = 1, FILE_W = 2, FILE_H = 3;
	std::deque<gImage*> imagelist;
	std::deque<std::string> imagetextlist;
	std::deque<int> imageparameterlist;
	int x, y, filew, fileh;
	int selectedx, selectedy, selectedw, selectedh, selectedid;
	int screenwidth, screenheight;
	int maxcolumncount;
//	int imagew[20], imageh[20];
//	int filex[20], filey[20];
//	std::vector<std::array<int, 1>> imagew;
	std::vector<int> imagew;
	std::vector<int> imageh;
	std::vector<int> filex;
	std::vector<int> filey;
	int cornerspace;
	int sonucw, sonuch;
	gColor fcolor, backgroundcolor;
	int minh, minw;
	gGUIFrame frame;
	gGUISizer framesizer;
};

#endif /* UI_GGUITABLE_H_ */
