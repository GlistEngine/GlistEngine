/*
 * gGUITable.cpp
 *
 *  Created on: 28 Tem 2023
 *      Author: Nazim Gultekin
 */

#include "gGUITable.h"

gGUITable::gGUITable() {
	// TODO Auto-generated constructor stub
	tablefont.loadFont("FreeSansBold.ttf", 12);
	x = 10;
	y = 10;
	w = 90;
	h = 160;
	screenwidth = 1280;
	screenheight = 720;
	maxcolumncount = screenwidth / (w * 2);
	pressedfileid = 0;
	isfilepressed = false;
	selectedx = 0;
	selectedy = 0;
	selectedw = 0;
	selectedh = 0;
	selectedid = 0;
}

gGUITable::~gGUITable() {

}

void gGUITable::draw() {
	//topparent->width;
	for (int index = 0; index < imagelist.size(); ++index) {
		//draw file
		imagelist[index]->drawSub( x + 150 * (index % maxcolumncount),
				y + (index / maxcolumncount) * 200,
				w,
				h,
				0, 0, imagelist[index]->getWidth(), imagelist[index]->getHeight());
		//draw file name
		tablefont.drawText(imagetextlist[index],
				x + w / 2 - (tablefont.getStringWidth(imagetextlist[index]) / 2) + (index % maxcolumncount) * 150,
				 y + h + 10 + (index / maxcolumncount) * 200);
	}
	//if(true) {
	//	gColor* oldcolor;
	//	oldcolor->set(renderer->getColor());
	//	renderer->setColor(225, 225, 255, 150);
	//	gDrawRectangle(x, y, w, h, true);
	//	renderer->setColor(oldcolor);
	//}
}

void gGUITable::addItem(gImage* image, std::string title) {
	imagelist.push_back(image);
	imagetextlist.push_back(title);
}
