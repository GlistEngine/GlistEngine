/*
 * gGUITable.h
 *
 *  Created on: 14 Tem 2023
 *      Author: Admin
 */

#ifndef UI_GGUITABLE_H_
#define UI_GGUITABLE_H_

#include "gImage.h"
#include "gGUIControl.h"
#include "gFont.h"
#include "gGUISizer.h"

class gGUITable : public gGUIControl{
public:
	gGUITable();
	virtual ~gGUITable();

	void draw();
	//void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void addItem(gImage* image, std::string title = "New File");
	//gGUISizer tablesizer;

private:
	gFont tablefont;
	bool isfilepressed;
	int pressedfileid;
	static const int DOSYA_X = 0, DOSYA_Y = 1, DOSYA_W = 2, DOSYA_H = 3;
	std::deque<gImage*> dosyaresimleri;
	std::deque<std::string> dosyaisimleri;
	std::deque<int> dosyaparametreleri;
	int x, y, w, h;
	int selectedx, selectedy, selectedw, selectedh, selectedid;
	int screenwidth, screenheight;
	int maxcolumncount;

};

#endif /* UI_GGUITABLE_H_ */
