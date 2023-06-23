/*
 * gGUIGrid.cpp
 *
 *  Created on: 5 Aug 2022
 *      Author: murat
 */

#include "gGUIGrid.h"
//const int gGUIGrid::SELECTEDBOX_X = 0;
//const int gGUIGrid::SELECTEDBOX_Y = 1;

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
	selectedbox = 0;
	isselected = false;
	totalh = rownum * gridboxh;
	rowtitle = 1;
	columntitle = 65; // 'A' char in ASCII
	clicktimediff = 250;
	clicktime = gGetSystemTimeMillis();
	previousclicktime = clicktime - 2 * clicktimediff;
	firstclicktime = previousclicktime - 2 * clicktimediff;
	isdoubleclicked = false;
	enableScrollbars(true, false);
}

gGUIGrid::~gGUIGrid() {

}

void gGUIGrid::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	totalh = rownum * gridboxh;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	setDimensions(w, h);
	textbox.set(root, this, this, 0, 0, gridx + (gridboxw / 2) + 1, gridy + gridboxh - 5, gridboxw - 6, gridboxh);
	textbox.setSize(gridboxw - 6, gridboxh - 2);
	textbox.enableBackground(false);
//	gLogi("Grid") << "Textbox:" << textbox.left << " " << textbox.top << " " << textbox.right << " " << textbox.bottom;
}

void gGUIGrid::setGrid(int rowNum, int columnNum) {
	rownum = rowNum;
	columnnum = columnNum;
	gridw = gridboxw * columnnum;
	gridh = gridboxh * rownum;
	createCells();
}

void gGUIGrid::setRowNum(int rowNum) {
	rownum = rowNum;
}

void gGUIGrid::setcolumnNum(int columnNum) {
	columnnum = columnNum;
}

void gGUIGrid::update() {
	textbox.update();
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
			allcells.push_back(tempcell);
		}
	}
}

void gGUIGrid::createTextBox() {
	textbox.set(root, this, this, 0, 0, allcells.at(selectedbox).cellx + 1, allcells.at(selectedbox).celly - 2, gridboxw - 10, gridboxh - 2);
	if(allcells.at(selectedbox).cellcontent != "") {
		textbox.setText(allcells.at(selectedbox).cellcontent);
		allcells.at(selectedbox).showncontent = "";
	}
}

void gGUIGrid::showCells() {
	int cellindexcounter = 0;
	for(int i = 0; i < rownum; i++) {
		for(int j = 0; j < columnnum; j++) {
//			gLogi("Grid") << "cellx: "<< allcells.at(cellindexcounter).cellx
//				<< " celly: " << allcells.at(cellindexcounter).celly
//				<< " cellrowno: " << allcells.at(cellindexcounter).cellrowno
//				<< " cellcolumnno: " << allcells.at(cellindexcounter).cellcolumnno
//				<< " cellcontent: " << allcells.at(cellindexcounter).cellcontent
//				<< " celltype:" << allcells.at(cellindexcounter).celltype;
			cellindexcounter++;
		}
	}
}

void gGUIGrid::showCell(int rowNo , int columnNo) {
	int cellindex = columnNo + (rowNo  * columnnum);
//	gLogi("Grid") << "cellindex:" << cellindex;
//	gLogi("Grid") << "cellx: "<< allcells.at(cellindex).cellx
//		<< " celly: " << allcells.at(cellindex).celly
//		<< " cellrowno: " << allcells.at(cellindex).cellrowno
//		<< " cellcolumnno: " << allcells.at(cellindex).cellcolumnno
//		<< " cellcontent: " << allcells.at(cellindex).cellcontent
//		<< " celltype: " << allcells.at(cellindex).celltype;
}

void gGUIGrid::checkCellType(int cellIndex) {
	for(int i = 0; i < allcells.at(cellIndex).cellcontent.length(); i++) {
		if(!isdigit(allcells.at(cellIndex).cellcontent.at(i))) {
			allcells.at(cellIndex).celltype = "string";
			break;
		}
		else allcells.at(cellIndex).celltype = "digit";
	}
}
void gGUIGrid::fillCell(int rowNo, int columnNo, std::string tempstr) { //when rowNo = 1, columnNO = 4; tempstr = "happyyyy";
	if(rowNo > rownum - 1 || columnNo > columnnum - 1) return;
	int cellindex = columnNo + (rowNo * columnnum);
	allcells.at(cellindex).cellcontent = tempstr;
	allcells.at(cellindex).showncontent = tempstr;
	bool isempty = (tempstr == "");
//	gLogi("Grid") << "isempty:" << isempty;

	checkCellType(cellindex);
	// Check left cell & update that cell's shown content if necessary
	int leftcolumnlimit = rowNo * columnnum;
	int leftcolumnindex = cellindex - 1;

	 // Check if input is empty and update left cells according to that
	if(isempty) {
		leftcolumnindex = cellindex;
		cellindex++;
	}

	while(leftcolumnindex >= leftcolumnlimit) {
		if(allcells.at(leftcolumnindex).cellcontent.empty()) {
			leftcolumnindex--;
			continue;
		}
		else {
			if(font->getStringWidth(allcells.at(leftcolumnindex).cellcontent) > (cellindex - leftcolumnindex) * gridboxw) {
				std::string tempstr;
				int index = 0;
				while (font->getStringWidth(tempstr) < (cellindex - leftcolumnindex - 1) * gridboxw + gridboxw * (0.9f)) {
					tempstr += allcells.at(leftcolumnindex).cellcontent[index];
					index++;
				}
				tempstr.pop_back();
				allcells.at(leftcolumnindex).showncontent = tempstr;
				break;
			} break;
		}
	}

	// Check right cell for shown content
	if(font->getStringWidth(tempstr) >= gridboxw){
		int rightcolumnlimit = (int)(font->getStringWidth(tempstr) / gridboxw) + cellindex;
		if(rightcolumnlimit >= (rowNo + 1) * columnnum) rightcolumnlimit = (rowNo + 1) * columnnum - 1;
		if(cellindex == rightcolumnlimit) {
			std::string tempstr;
			int index = 0;
			while (font->getStringWidth(tempstr) < gridboxw * (0.9f)) {
				tempstr += allcells.at(cellindex).cellcontent[index];
				index++;
			}
			tempstr.pop_back();
			allcells.at(cellindex).showncontent = tempstr;
			return;
		}
		int rightcolumnindex = cellindex + 1;

		while(rightcolumnindex <= rightcolumnlimit) {
			if(allcells.at(rightcolumnindex).cellcontent.empty()) {
				if(rightcolumnindex == rightcolumnlimit) {
					std::string tempstr;
					int index = 0;
					while (font->getStringWidth(tempstr) < (rightcolumnindex - cellindex) * gridboxw + gridboxw * (0.9f)) {
						if(index > allcells.at(cellindex).cellcontent.length()) break;
						tempstr += allcells.at(cellindex).cellcontent[index];
						index++;
					}
					tempstr.pop_back();
					allcells.at(cellindex).showncontent = tempstr;
					break;
				}
				rightcolumnindex++;
				continue;
			}
			else {
				std::string tempstr;
				int index = 0;
				while (font->getStringWidth(tempstr) < (rightcolumnindex - cellindex - 1) * gridboxw + gridboxw * (0.9f)) {
					if(index > allcells.at(cellindex).cellcontent.length()) break;
					tempstr += allcells.at(cellindex).cellcontent[index];
					index++;
				}
				tempstr.pop_back();
				allcells.at(cellindex).showncontent = tempstr;
				break;
			} break;

		}

		if(allcells.at(cellindex).showncontent == "") {
//			gLogi("Grid");
		}
	}
}

void gGUIGrid::changeCell() {
	std::string tmpstr = textbox.getText();
	fillCell((allcells.at(selectedbox).celly - gridboxh) / gridboxh, (allcells.at(selectedbox).cellx - (gridboxw / 2)) / gridboxw, tmpstr);
	textbox.cleanText();
	istextboxactive = false;
}

void gGUIGrid::drawContent() {
	gColor oldcolor = renderer->getColor();
	drawCellBackground();
	if(isselected) drawSelectedBox();
	drawCellContents();
	if(istextboxactive)textbox.draw();
	drawTitleRowBackground();
	drawRowContents();
	drawTitleColumnBackground();
	drawColumnContents();
	drawTitleLines();
	renderer->setColor(oldcolor);
}

void gGUIGrid::drawCellBackground() {
	renderer->setColor(*textbackgroundcolor);
	gDrawRectangle(gridx + (gridboxw / 2), gridy + gridboxh, gridw, gridh, true);
}

void gGUIGrid::drawSelectedBox() {
	renderer->setColor(0.0f, 1.0f, 0.0f, 1.0f);
	gDrawRectangle(allcells.at(selectedbox).cellx + 1, (allcells.at(selectedbox).celly + 1) - firsty, gridboxw - 2, gridboxh - 2, false);
	gDrawRectangle(allcells.at(selectedbox).cellx + (gridboxw - 2) - 6, allcells.at(selectedbox).celly + (gridboxh - 2) - 4 - firsty, 6, 6, true); // FLAG
}

void gGUIGrid::drawTitleRowBackground() {
	renderer->setColor(*buttoncolor);
	gDrawRectangle(gridx, gridy - firsty, gridboxw / 2, gridh + gridboxh, true);
}

void gGUIGrid::drawTitleColumnBackground() {
	renderer->setColor(*buttoncolor);
	gDrawRectangle(gridx, gridy, gridw + (gridboxw / 2), gridboxh , true);
}

void gGUIGrid::drawRowContents() {
	int temprow = rowtitle;
	renderer->setColor(*pressedbuttoncolor);
	for(int i = 1; i <= rownum; i++) {
		std::string rowtitlestring = std::to_string(temprow);
		renderer->setColor(*fontcolor);
		font->drawText(rowtitlestring, gridx + (gridboxw / 4) - (font->getStringWidth(rowtitlestring) / 2), gridy + (gridboxh / 2) + (i * gridboxh)  + (font->getStringHeight(rowtitlestring) / 2) - firsty);
		temprow++;
		renderer->setColor(*pressedbuttoncolor);
		gDrawLine(gridx, gridy + ((i + 1) * gridboxh) - firsty, gridx + gridw + (gridboxw / 2), gridy + ((i + 1) * gridboxh) - firsty);
	}
}

void gGUIGrid::drawColumnContents() {
	int tempcol = columntitle;
	for(int i = 1; i <= columnnum; i++) {
	    std::string columntitlestring(1, (char) tempcol);
	    renderer->setColor(*fontcolor);
		font->drawText(columntitlestring, gridx + (i * gridboxw) - (font->getStringWidth(columntitlestring) / 2), gridy + (gridboxh / 2) + (font->getStringHeight(columntitlestring) / 2));
		tempcol++;
		renderer->setColor(*pressedbuttoncolor);
		gDrawLine(gridx - (gridboxw / 2) + ((i + 1) * gridboxw), gridy - firsty, gridx - (gridboxw / 2) + ((i + 1) * gridboxw), gridy + gridboxh +gridh - firsty);
	}
}

void gGUIGrid::drawTitleLines() {
	renderer->setColor(*backgroundcolor);
	gDrawLine(gridx + (gridboxw / 2) + 1, gridy, gridx + (gridboxw / 2) + 1, gridy + gridboxh + gridh);
	gDrawLine(gridx, gridy + (gridboxh), gridx + gridboxw / 2 + gridw, gridy + (gridboxh));
}

void gGUIGrid::drawCellContents() {
	renderer->setColor(*fontcolor);
	int cellindexcounter = 0;
	for(int i = 0; i < rownum; i++) {
		for(int j = 0; j < columnnum; j++) {
			font->drawText(allcells.at(cellindexcounter).showncontent, allcells.at(cellindexcounter).cellx, allcells.at(cellindexcounter).celly + (gridboxh / 2) + (font->getStringHeight(allcells.at(cellindexcounter).showncontent) / 2) - firsty);
			cellindexcounter++;
		}
	}
}

void gGUIGrid::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	int pressedx = x - left;
	int pressedy = y - top - firsty - titledy;
	if(pressedx >= gridx + (gridboxw / 2) && pressedx <= gridx + (gridboxw / 2) + gridw && pressedy >= gridy + gridboxh && pressedy <= gridy + gridboxh + gridh) {
		previousclicktime = clicktime;
		clicktime = gGetSystemTimeMillis();
		if(clicktime - previousclicktime <= clicktimediff) {
			isdoubleclicked = true;
//			gLogi("Grid") << "doubleclicked.";
		} else isdoubleclicked = false;
		if(isdoubleclicked) {
			textbox.cleanText();
			createTextBox();
			textbox.mousePressed(pressedx, pressedy, button);
			istextboxactive = true;
		} else istextboxactive = false;
		isselected = true;
		int newcellindex = ((int)((x - left - (gridboxw / 2)) / gridboxw)) + ((int)((y + firsty - top - gridboxh - 20 + ((font->getSize() * 1.8f) * !istitleon)) / gridboxh))  * columnnum ; // * gridboxw + (gridboxw / 2);
		if(newcellindex != selectedbox) {
			if(istextboxactive) changeCell();
			selectedbox = newcellindex;
		}
	}

}

void gGUIGrid::mouseReleased(int x, int y, int button) {
	if(istextboxactive) textbox.mouseReleased((x - left), (y - top - firsty - titledy), button);
}

void gGUIGrid::mouseDragged(int x, int y, int button) {
//	gLogi("Grid") << x << " " << y;
	if(istextboxactive) textbox.mouseDragged((x - left), (y - top - firsty), button);
}

void gGUIGrid::keyPressed(int key){
	if(istextboxactive) textbox.keyPressed(key);
}

void gGUIGrid::keyReleased(int key) {
	if(istextboxactive) textbox.keyReleased(key);
	if(key == G_KEY_ENTER && istextboxactive) {
		changeCell();
		istextboxactive = false;
		textbox.setEditable(false);
	}
	else if ((key == G_KEY_ENTER || key == G_KEY_DOWN) && !istextboxactive) {
		if(selectedbox + columnnum < rownum * columnnum) selectedbox += columnnum;
	}
	else if(key == G_KEY_UP && !istextboxactive) {
		if(selectedbox - columnnum > 0) selectedbox -= columnnum;
	}
	else if(key == G_KEY_RIGHT && !istextboxactive) {
		if(selectedbox % columnnum != (columnnum - 1)) selectedbox++;
	}
	else if(key == G_KEY_LEFT && !istextboxactive) {
		if(selectedbox % columnnum != 0) selectedbox--;
	}
	else if(key == G_KEY_F2) {
		textbox.cleanText();
		createTextBox();
		istextboxactive = true;
		istextboxactive = true;
		textbox.setEditable(true);
	}
}

void gGUIGrid::charPressed(unsigned int codepoint) {
	if(istextboxactive) textbox.charPressed(codepoint);
}

void gGUIGrid::mouseScrolled(int x, int y) {
	gGUIScrollable::mouseScrolled(x, y);
}
