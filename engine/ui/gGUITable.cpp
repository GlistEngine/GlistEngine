/*
 * gGUITable.cpp
 *
 *  Created on: 28 Tem 2023
 *      Author: Nazim Gultekin
 */

#include "gGUITable.h"

gGUITable::gGUITable() {
	// TODO Auto-generated constructor stub
	x = 16;
	y = 20;
	filew = 90;
	fileh = 160;
	maxcolumncount = screenwidth / (filew * 2);
	sonucw = 0;
	sonuch = 0;
	cornerspace = 10;
	minh = 4;
	minw = 1;
	isfilepressed = false;
	isselected = false;
	ismoved = false;
	screenwidth = 0;
	screenheight = getScreenWidth();
	movedfileno = getScreenHeight();
	selectedfileno = -1;
	mousepressedonlist = false;
}

gGUITable::~gGUITable() {

}

void gGUITable::set(gBaseApp *root, gBaseGUIObject *topParentGUIObject,
		gBaseGUIObject *parentGUIObject, int parentSlotLineNo,
		int parentSlotColumnNo, int x, int y, int w, int h) {
	totalw = screenwidth;
	totalh = screenheight;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	setDimensions(totalw, totalh);
}

void gGUITable::update() {
	screenwidth = getScreenWidth();
	screenheight = getScreenHeight();
	maxcolumncount = screenwidth / (filew + cornerspace);
	if(maxcolumncount == 0) return;

		for (int i = 0; i < imagelist.size(); i++) {
			imagew.push_back(imagelist[i]->getWidth());
			imageh.push_back(imagelist[i]->getHeight());

				if(filew < fileh) {
					if(imagew[i] >= imageh[i]) {
						//horizontal image
						sonucw = imagew[i] / filew;
						imagew[i] /= sonucw + minw;
						imageh[i] /= sonucw + minw;
					}

					else if(imageh[i] > imagew[i]) {
						//vertical image
						sonuch = imageh[i] / fileh;
						if(!(imageh[i] <= fileh)){
							imagew[i] /= sonuch + minh;
							imageh[i] /= sonuch + minh;
						}
					}
			}
		}

}

void gGUITable::draw() {
	if(maxcolumncount == 0) return;
	gColor oldcolor = renderer->getColor();
	renderer->setColor(backgroundcolor);
	gDrawRectangle(0, 0, screenwidth, screenheight, true);
	renderer->setColor(oldcolor);
	//draw selected rectangle
	if(isselected) {
		gColor oldcolor = renderer->getColor();
		gColor selected = gColor(0.8f, 0.7f, 0.5f);
		renderer->setColor(selected);
		gDrawRectangle(filex[selectedfileno], filey[selectedfileno], filew, fileh, true);
		renderer->setColor(oldcolor);
	}
	//draw moved rectangle
	if(ismoved) {
		gColor oldcolor = renderer->getColor();
		gColor moved = gColor(0.8f, 0.7f, 0.5f);
		renderer->setColor(moved);
		gDrawRectangle(filex[movedfileno], filey[movedfileno], filew, fileh, false);
		renderer->setColor(oldcolor);
	}

	for (int index = 0; index < imagelist.size(); ++index) {
		filex.push_back(x + 100 * (index % maxcolumncount));
		filey.push_back(y + 200 * (index / maxcolumncount));
		filex[index] = x + 100 * (index % maxcolumncount);
		filey[index] = y + 200 * (index / maxcolumncount);
		//draw file
		imagelist[index]->drawSub(filex[index] + (filew - imagew[index]) / 2,
				filey[index] + (fileh - (imageh[index] + cornerspace)),
				imagew[index],
				imageh[index],
				0, 0, imagelist[index]->getWidth(), imagelist[index]->getHeight());
		//draw file name
		gColor oldcolor = *renderer->getColor();
		renderer->setColor(&fcolor);
		setFontColor(&fcolor);
		font->drawText(imagetextlist[index],
				x + filew / 2 - (font->getStringWidth(imagetextlist[index]) / 2) + (index % maxcolumncount) * 100,
				 y + fileh + 10 + (index / maxcolumncount) * 200);
		renderer->setColor(&oldcolor);
	}
}

void gGUITable::addItem(gTexture* image, std::string title) {
	imagelist.push_back(image);
	imagetextlist.push_back(title);
}

void gGUITable::setFontColor(gColor color) {
	fcolor = color;
}

gColor* gGUITable::getFontColor() {
	return &fcolor;
}

void gGUITable::setTableColor(gColor color) {
	backgroundcolor = color;
}

int gGUITable::setSelectedFileNo(int selectedFileNo) {
	selectedfileno = selectedFileNo;
	return selectedfileno;
}

int gGUITable::getSelectedFileNo() {
	return selectedfileno;
}

gColor* gGUITable::getTableColor() {
	return &backgroundcolor;
}

void gGUITable::mousePressed(int x, int y, int button) {
	for(int index = 0; index < imagelist.size(); index++) {
		if(x >= filex[index] && x <= filex[index] + filew && y >= filey[index] && y <= filey[index] + fileh) {
			isselected = true;
			selectedfileno = index;
			break;
		}
		else {
			isselected = false;
		}
	}
}

void gGUITable::mouseMoved(int x, int y) {
		for(int index = 0; index < imagelist.size(); index++) {
		if(x >= filex[index] && x <= filex[index] + filew && y >= filey[index] && y <= filey[index] + fileh) {
			ismoved = true;
			movedfileno = index;
			break;
			}
		else ismoved = false;
		}
}
