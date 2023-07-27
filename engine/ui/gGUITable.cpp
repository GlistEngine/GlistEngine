/*
 * gGUITable.cpp
 *
 *  Created on: 14 Tem 2023
 *      Author: Admin
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
	gLogi("GUITABLE: ") << "boyut: " << dosyaresimleri.size();

	for (int index = 0; index < dosyaresimleri.size(); ++index) {
		gLogi("GUITABLE: ") << "ugradi3: " << index;
		//resim
		dosyaresimleri[index]->drawSub( x + 150 * (index % maxcolumncount),
				y + (index / maxcolumncount) * 200,
				w,
				h,
				0, 0, dosyaresimleri[index]->getWidth(), dosyaresimleri[index]->getHeight());
		gLogi("GUITABLE: ") << "ugradi4: ";
		//yazý
		tablefont.drawText(dosyaisimleri[index],
				x + w / 2 - (tablefont.getStringWidth(dosyaisimleri[index]) / 2) + (index % maxcolumncount) * 150,
				 y + h + 10 + (index / maxcolumncount) * 200);
		gLogi("GUITABLE: ") << "ugradi5: ";
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
	gLogi("GUITABLE: ") << "ugradi0";
	dosyaresimleri.push_back(image);
	gLogi("GUITABLE: ") << "ugradi1";
	dosyaisimleri.push_back(title);
	gLogi("GUITABLE: ") << "ugradi2";
}
