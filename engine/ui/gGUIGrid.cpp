/*
 * gGUIGrid.cpp
 *
 *  Created on: 5 Aðu 2022
 *      Author: murat
 */

#include "gGUIGrid.h"
const int gGUIGrid::SELECTEDBOX_X = 0;
const int gGUIGrid::SELECTEDBOX_Y = 1;

gGUIGrid::gGUIGrid() {
//	gridsizer.setSize(10,10);
//	gridsizer.enableBorders(true);
	gridx = 0.0f;
	gridy = 0.0f;
	gridboxw = 80.0f;
	gridboxh = 30.0f;
	rownum = 50;
	columnnum = 10;
	gridw = gridboxw * columnnum;
	gridh = gridboxh * rownum;
	selectedbox[0] = 0;
	selectedbox[1] = 0;
	isselected = false;
	totalh = rownum * gridboxh;
	rowtitle = 1;
	columntitle = 65; // 'A' char in ASCII

//	setSizer(&gridsizer);
//	gridsizer.setControl(1, 0, &slider);
	enableScrollbars(true, false);

}

gGUIGrid::~gGUIGrid() {

}

void gGUIGrid::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	totalh = rownum * gridboxh;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(w, h);
	textbox.set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, gridx + (gridboxw / 2), gridy + gridboxh, gridboxw, gridboxh);
	textbox.enableBackground(false);

//	gridx = left;
//	gridy = top;
//	setGrid(linenum, columnnum);
}

void gGUIGrid::setGrid(int rowNum, int columnNum) {
	rownum = rowNum;
	columnnum = columnNum;
	createCells();
}

void gGUIGrid::setRowNum(int rowNum) {
	rownum = rowNum;
}

void gGUIGrid::setcolumnNum(int columnNum) {
	columnnum = columnNum;
}

void gGUIGrid::createCells() {
	Cell tempcell;
	for(int i = 0; i < rownum; i++) {
		for(int j = 0; j < columnnum; j++) {
			tempcell.cellx = gridx + (gridboxw / 2) + (j * gridboxw);
			tempcell.celly = gridy + gridboxh + (i * gridboxh);
			tempcell.cellrowno = i;
			tempcell.cellcolumnno = j;
			tempcell.cellcontent = "";
//			gLogi("x") << tempcell.cellx;
//			gLogi("y") << tempcell.celly;
//			gLogi("cellrowno") << tempcell.cellrowno;
//			gLogi("cellcolumnno") << tempcell.cellcolumnno;
//			gLogi("content") << tempcell.cellcontent << "\n";

			allcells.push_back(tempcell);

		}
	}
//	showCells();
//	showCell(24, 8);
}

void gGUIGrid::createTextBox(int x, int y) {
//	textbox.set(root, topparent, parent, parentslotlineno, parentslotcolumnno, x, y, gridboxw, gridboxh);
//	gLogi("gamecanvas") << "textbox is on";
	textbox.addLeftMargin(x - left);
	textbox.addTopMargin(y - top);
}

void gGUIGrid::showCells() {
	int cellindexcounter = 0;
	for(int i = 0; i < rownum; i++) {
		for(int j = 0; j < columnnum; j++) {
			gLogi("Cell") << "cellx: "<< allcells.at(cellindexcounter).cellx
				<< " celly: " << allcells.at(cellindexcounter).celly
				<< " cellrowno: " << allcells.at(cellindexcounter).cellrowno
				<< " cellcolumnno: " << allcells.at(cellindexcounter).cellcolumnno
				<< " cellcontent: " << allcells.at(cellindexcounter).cellcontent
				<< "cellindexcounter: " << cellindexcounter  << "\n";
			cellindexcounter++;
		}
	}
}

void gGUIGrid::showCell(int rowNo , int columnNo) { // 2,4
	int cellindex = columnNo + (rowNo  * columnnum);
	gLogi("cellindex") << cellindex;
	gLogi("Cell") << "cellx: "<< allcells.at(cellindex).cellx
		<< " celly: " << allcells.at(cellindex).celly
		<< " cellrowno: " << allcells.at(cellindex).cellrowno
		<< " cellcolumnno: " << allcells.at(cellindex).cellcolumnno
		<< " cellcontent: " << allcells.at(cellindex).cellcontent;
}

void gGUIGrid::fillCell(int rowNo, int columnNo, std::string tempstr) {
	int cellindex = columnNo + (rowNo * columnnum);
	allcells.at(cellindex).cellcontent = tempstr;
	allcells.at(cellindex).showncontent = allcells.at(cellindex).cellcontent;
}

bool gGUIGrid::isRightCellFull(int cellIndex) {
	if(cellIndex % columnnum != columnnum - 1) {
		if(!allcells.at(cellIndex).cellcontent.empty() && !allcells.at(cellIndex + 1).cellcontent.empty()) {
			return true;
		}
	}
	else if(cellIndex % columnnum == columnnum - 1) {
		return true;
	}

	return false;
}

void gGUIGrid::drawContent() {
	gColor oldcolor = renderer->getColor();
	drawCellBackground();
	if(isselected) drawSelectedBox();
	drawCellContents();
	drawTitleRowBackground();
	drawRowContents();
	drawTitleColumnBackground();
	drawColumnContents();
	drawTitleLines();
//	textbox.draw();
//	gLogi("Textbox") << textbox.width;
	renderer->setColor(oldcolor);
}

void gGUIGrid::drawCellBackground() {
	renderer->setColor(*textbackgroundcolor);
//	renderer->setColor(*buttoncolor);
	gDrawRectangle(gridx + (gridboxw / 2), gridy + gridboxh, gridw - gridboxw, gridh - gridboxh, true);
}

void gGUIGrid::drawSelectedBox() {
	renderer->setColor(0.0f, 1.0f, 0.0f, 1.0f);
	gDrawRectangle(selectedbox[SELECTEDBOX_X] + 1, (selectedbox[SELECTEDBOX_Y] + 1) - firsty, gridboxw - 2, gridboxh - 2, false);
}

void gGUIGrid::drawTitleRowBackground() {
	renderer->setColor(*buttoncolor);
	gDrawRectangle(gridx, gridy - firsty, gridboxw / 2, gridh, true);
}

void gGUIGrid::drawTitleColumnBackground() {
	renderer->setColor(*buttoncolor);
	gDrawRectangle(gridx, gridy, gridw, gridboxh , true);
}

void gGUIGrid::drawRowContents() {
	int temprow = rowtitle;
	renderer->setColor(*pressedbuttoncolor);
	for(int i = 2; i <= rownum; i++) {
		std::string rowtitlestring = std::to_string(temprow);
		renderer->setColor(*fontcolor);
		font->drawText(rowtitlestring, gridx + (gridboxw / 4) - (font->getStringWidth(rowtitlestring) / 2), - firsty + gridy + (i * gridboxh) - (gridboxh / 2) + (font->getStringHeight(rowtitlestring) / 2));
		temprow++;
		renderer->setColor(*pressedbuttoncolor);
		gDrawLine(gridx, gridy + (i * gridboxh) - firsty, gridx + gridw, gridy + (i * gridboxh) - firsty);
	}
}

void gGUIGrid::drawColumnContents() {
	int tempcol = columntitle;
	for(int i = 2; i <= columnnum; i++) {
	    std::string columntitlestring(1, (char) tempcol);
	    renderer->setColor(*fontcolor);
		font->drawText(columntitlestring, gridx + ((i-1) * gridboxw) - (font->getStringWidth(columntitlestring) / 2), gridy + (gridboxh / 2) + (font->getStringHeight(columntitlestring) / 2));
		tempcol++;
		renderer->setColor(*pressedbuttoncolor);
		gDrawLine(gridx - (gridboxw / 2) + (i * gridboxw), gridy - firsty, gridx - (gridboxw / 2) + (i * gridboxw), gridy + gridh - firsty);
	}
}

void gGUIGrid::drawTitleLines() {
	//black row and column grid
	renderer->setColor(*backgroundcolor);
	gDrawLine(gridx + (gridboxw / 2) + 1, gridy, gridx + (gridboxw / 2) + 1, gridy + gridh); //vertical
	gDrawLine(gridx, gridy + (gridboxh), gridx + gridw, gridy + (gridboxh)); //horizontal
}

void gGUIGrid::drawCellContents() {
	renderer->setColor(*fontcolor);
	int cellindexcounter = 0;
	for(int i = 0; i < rownum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(!allcells.at(cellindexcounter).cellcontent.empty()) {
				if(isRightCellFull(cellindexcounter)) {
					std::string tempstr;
					int index = 0;
					if(font->getStringWidth(allcells.at(cellindexcounter).cellcontent) > gridboxw * (0.9f)){
						while (font->getStringWidth(tempstr) < gridboxw * (0.9f)) {
							tempstr += allcells.at(cellindexcounter).cellcontent[index];
							index++;
						}
						tempstr.pop_back();
						allcells.at(cellindexcounter).showncontent = tempstr;
					}
//					gLogi("GameCanvas") << "str: " << allcells.at(cellindexcounter).showncontent;
				}
				else {
					allcells.at(cellindexcounter).showncontent = allcells.at(cellindexcounter).cellcontent;
				}
//				gLogi("GameCanvas") << "girdi: " << cellindexcounter;
//				gLogi("Filled cell") << " cellrowno: " << allcells.at(cellindexcounter).cellrowno << " cellcolumnno: " << allcells.at(cellindexcounter).cellcolumnno;
				font->drawText(allcells.at(cellindexcounter).showncontent, allcells.at(cellindexcounter).cellx, allcells.at(cellindexcounter).celly + (gridboxh / 2) + (font->getStringHeight(allcells.at(cellindexcounter).showncontent) / 2) - firsty);

			}
			if(cellindexcounter < (rownum * columnnum)) cellindexcounter++;
		}
	}
}

void gGUIGrid::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	textbox.mousePressed(x, y, button);
	if(x - left >= gridx + (gridboxw / 2) && x - left <= gridx + gridw - (gridboxw / 2) && y - top >= gridy + gridboxh && y - top <= gridy + gridh) {
		int cellindexcounter = 0;
		isselected = true;
		selectedbox[SELECTEDBOX_X] = ((int)((x - left - (gridboxw / 2)) / gridboxw)) * gridboxw + (gridboxw / 2);
		selectedbox[SELECTEDBOX_Y] = ((int)((y + firsty - top - gridboxh - 20) / gridboxh)) * gridboxh + gridboxh;
//		gLogi("GameCanvas") << "selectedbox_x: " << selectedbox[SELECTEDBOX_X] << " selectedbox_y: " << selectedbox[SELECTEDBOX_Y];
		for(int i = 0; i < rownum; i++) {
			for(int j = 0; j < columnnum; j++) {
				if(allcells.at(cellindexcounter).cellx == selectedbox[SELECTEDBOX_X] && allcells.at(cellindexcounter).celly == selectedbox[SELECTEDBOX_Y]) {
					showCell(i, j);
				}
				cellindexcounter++;
			}
		}
		createTextBox(x, y);
	}
}





