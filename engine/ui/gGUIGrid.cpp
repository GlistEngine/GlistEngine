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
	firstcellx = gridx + 1 + (gridboxw / 2);
	firstcelly = gridy + 1 + gridboxh;
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
//	gridx = left;
//	gridy = top;
//	setGrid(linenum, columnnum);
}

void gGUIGrid::setGrid(int rowNum, int columnNum) {
//	for(int i = 0; i < lineNum; i++) {
//		std::vector<gGUITextbox> tempvec;
//		for(int j = 0; j < columnNum; j++) {
//			gGUITextbox texttemp;
//			tempvec.push_back(texttemp);
//		}
//		textboxvec.push_back(tempvec);
//		for(int j = 0; j < columnNum; j++) gridsizer.setControl(i, j, &textboxvec[i][j]);
//	}
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

void gGUIGrid::showCell(int rowNo, int columnNo) {
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
}

void gGUIGrid::drawContent() {
	gColor oldcolor = renderer->getColor();
	drawCellBackground(); //cell's background
	if(isselected) drawSelectedBox(); // selected box
	drawCellContents(); // draws cell's contents
	drawTitleRowBackground(); // title line background
	drawRowContents(); // draws titles and lines of rows
	drawTitleColumnBackground(); // title line background
	drawColumnContents(); // draws titles and lines of columns
	drawTitleLines(); // draws title lines

	renderer->setColor(oldcolor);
}

void gGUIGrid::drawCellBackground() {
	renderer->setColor(*textbackgroundcolor);
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

	fillCell(0, 0, "Start");
	fillCell(1, 1, "hello");
	fillCell(2, 2, "world");
	fillCell(3, 3, "messageeeeeeeeeeeehehehehe");

	for(int i = 0; i < rownum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(!allcells.at(cellindexcounter).cellcontent.empty()) {
//				gLogi("GameCanvas") << "girdi: " << cellindexcounter;
//				gLogi("Filled cell") << " cellrowno: " << allcells.at(cellindexcounter).cellrowno << " cellcolumnno: " << allcells.at(cellindexcounter).cellcolumnno;
				font->drawText(allcells.at(cellindexcounter).cellcontent, allcells.at(cellindexcounter).cellx, allcells.at(cellindexcounter).celly + (gridboxh / 2) + (font->getStringHeight(allcells.at(cellindexcounter).cellcontent) / 2) - firsty);

			}
			if(cellindexcounter <= (rownum * columnnum)) cellindexcounter++;
		}
	}
}

void gGUIGrid::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);

//	gLogi("x") << x;
//	gLogi("y") << y;
//	gLogi("left") << left;
//	gLogi("top") << top;

	if(x - left >= gridx + (gridboxw / 2) && x - left <= gridx + gridw - (gridboxw / 2) && y - top >= gridy + gridboxh && y - top <= gridy + gridh) {
//		gLogi("GameCanvas") << "collided";
		int cellindexcounter = 0;
		isselected = true;
//		selectedbox[SELECTEDBOX_X] = x - left - (gridboxw / 2 + 6); // 6 piksel gridin dýþýndaki 6 piksellik kýsým yüzünden
//		selectedbox[SELECTEDBOX_Y] = y - top - (gridboxh);
		selectedbox[SELECTEDBOX_X] = ((int)((x - left - (gridboxw / 2)) / gridboxw)) * gridboxw + (gridboxw / 2);
		selectedbox[SELECTEDBOX_Y] = ((int)((y + firsty - top - gridboxh - 20) / gridboxh)) * gridboxh + gridboxh;
		gLogi("GameCanvas") << "selectedbox_x: " << selectedbox[SELECTEDBOX_X] << " selectedbox_y: " << selectedbox[SELECTEDBOX_Y];
		for(int i = 0; i < rownum; i++) {
			for(int j = 0; j < columnnum; j++) {
				if(allcells.at(cellindexcounter).cellx == selectedbox[SELECTEDBOX_X] && allcells.at(cellindexcounter).celly == selectedbox[SELECTEDBOX_Y]) {
					showCell(i, j);
				}
				cellindexcounter++;
			}
		}


	}
//	gLogi("GameCanvas") << "x: " << x << " y: " << y << " selectedbox: " << selectedbox[SELECTEDBOX_X]<< " " << selectedbox[SELECTEDBOX_Y];
}





