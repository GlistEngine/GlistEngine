/*
 * gGUIGrid.cpp
 *
 *  Created on: 5 Aug 2022
 *      Author: murat
 */

#include "gGUIGrid.h"
//const int gGUIGrid::SELECTEDBOX_X = 0;
//const int gGUIGrid::SELECTEDBOX_Y = 1;
#include "gBaseApp.h"

gGUIGrid::gGUIGrid() {
//	gridsizer.setSize(10,10);
//	gridsizer.enableBorders(true);
	gridx = 0.0f;
	gridy = 0.0f;
	gridboxw = 80.0f;
	gridboxh = 30.0f;
	newgridboxw = gridboxw;
	rownum = 50;
	columnnum = 10;
	gridw = gridboxw * columnnum;
	gridh = gridboxh * rownum;
	selectedbox = 0;
	isselected = false;
	isrowselected = false;
	iscolumnselected = false;
	totalw = columnnum * gridboxw;
	totalh = rownum * gridboxh;
	rowtitle = 1;
	columntitle = 65; // 'A' char in ASCII
	clicktimediff = 250;
	titlediff = 20;
	clicktime = gGetSystemTimeMillis();
	previousclicktime = clicktime - 2 * clicktimediff;
	firstclicktime = previousclicktime - 2 * clicktimediff;
	isdoubleclicked = false;
	selectedtitle = 0;
	enableScrollbars(true, false);
	Cell tempcell;
	setMargin(tempcell.cellw / 2, tempcell.cellh);
}

gGUIGrid::~gGUIGrid() {

}

void gGUIGrid::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalw = columnnum * gridboxw;
	totalh = h;
	totalh = rownum * gridboxh;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	setDimensions(w, h);
	textbox.set(root, this, this, 0, 0, gridx + (gridboxw / 2) + 1, gridy + gridboxh - 5, gridboxw - 6, gridboxh);
	textbox.setSize(gridboxw - 6, gridboxh - 2);
	textbox.enableBackground(false);
	manager = root->getAppManager()->getGUIManager();

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
			tempcell.fontnum = gGUIManager::FONT_REGULAR;
			tempcell.cellalignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
			tempcell.textmoveamount = 0;
			tempcell.cellfontcolor = fontcolor;
			tempcell.iscellaligned = false;
			allcells.push_back(tempcell);
		}
	}
}

void gGUIGrid::createTextBox() {
	//allcells.at(selectedbox).cellx + 1
	//allcells.at(cellindexcounter).cellx + (allcells.at(cellindexcounter).cellw - font->getStringWidth(allcells.at(cellindexcounter).showncontent)) * textbox.getTextMoveAmount() - textbox.getInitX() * textbox.getTextAlignment() - firstx
	int newwamount = font->getStringWidth(allcells.at(selectedbox).cellcontent) / gridboxw + 1;
	newgridboxw = gridboxw * newwamount;
	if(allcells.at(selectedbox).cellalignment == gBaseGUIObject::TEXTALIGNMENT_LEFT) {
		while(newgridboxw > gridw - allcells.at(selectedbox).cellx + allcells.at(selectedbox).cellw / 2)
			newgridboxw -= gridboxw;
			//Should go to a new line and the width of the textbox should be "gridw - allcells.at(selectedbox).cellx + allcells.at(selectedbox).cellw / 2"
	}
	else if(allcells.at(selectedbox).cellalignment == gBaseGUIObject::TEXTALIGNMENT_RIGHT) {
		while(newgridboxw > allcells.at(selectedbox).cellx - allcells.at(selectedbox).cellw / 2 + gridboxw)
			newgridboxw -= gridboxw;
			//Should go to a new line and the width of the textbox should be "allcells.at(selectedbox).cellx - allcells.at(selectedbox).cellw / 2 + gridboxw"
	}
	textbox.set(root, this, this, 0, 0, allcells.at(selectedbox).cellx + 1, allcells.at(selectedbox).celly - 2, newgridboxw - 10, gridboxh - 2);
	textbox.setTextFont(manager->getFont(allcells.at(selectedbox).fontnum));
	textbox.setTextAlignment(allcells.at(selectedbox).cellalignment, allcells.at(selectedbox).cellw, textbox.getInitX());
	textbox.setTextColor(&allcells.at(selectedbox).cellfontcolor);
	if(allcells.at(selectedbox).cellcontent != "") {
		textbox.setText(allcells.at(selectedbox).cellcontent);
		int length = allcells.at(selectedbox).cellcontent.length();
		if(allcells.at(selectedbox).cellalignment == gBaseGUIObject::TEXTALIGNMENT_LEFT || allcells.at(selectedbox).cellalignment == gBaseGUIObject::TEXTALIGNMENT_RIGHT)
			textbox.setCursorPosX(font->getStringWidth(allcells.at(selectedbox).cellcontent), length);
		else {
			std::string mid;
			int middle;
			if(length % 2 == 0) middle = length / 2;
			else middle = length / 2 + 1;
			mid = allcells.at(selectedbox).cellcontent.substr(0, middle);
			textbox.setCursorPosX(font->getStringWidth(mid), middle);
		}
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
			if(allcells.at(cellIndex).celltype == "digit" && !allcells.at(cellIndex).iscellaligned) changeCellAlignment(gBaseGUIObject::TEXTALIGNMENT_LEFT, false);
			allcells.at(cellIndex).celltype = "string";
			break;
		}
		else allcells.at(cellIndex).celltype = "digit";
	}
	if(allcells.at(cellIndex).celltype == "digit" && !allcells.at(cellIndex).iscellaligned) changeCellAlignment(gBaseGUIObject::TEXTALIGNMENT_RIGHT, false);
}

void gGUIGrid::changeCellFont(int fontNum) {
	if(allcells.at(selectedbox).iscellselected) {
		allcells.at(selectedbox).fontnum = fontNum;
		allcells.at(selectedbox).textmoveamount = 0.5f * fontNum;
		textbox.setTextFont(manager->getFont(fontNum));
	}
}

void gGUIGrid::changeCellAlignment(int cellAlignment, bool clicked) {
	if(clicked && !allcells.at(selectedbox).iscellaligned) allcells.at(selectedbox).iscellaligned = true;
	else if(clicked && allcells.at(selectedbox).iscellaligned && allcells.at(selectedbox).celltype == "digit") {
		allcells.at(selectedbox).iscellaligned = false;
		cellAlignment = gBaseGUIObject::TEXTALIGNMENT_RIGHT;
	}
	else if(clicked && allcells.at(selectedbox).cellalignment == cellAlignment) {
		allcells.at(selectedbox).iscellaligned = false;
		cellAlignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
	}
	allcells.at(selectedbox).cellalignment = cellAlignment;
	allcells.at(selectedbox).textmoveamount = 0.5f * cellAlignment;
	textbox.setTextAlignment(cellAlignment, allcells.at(selectedbox).cellw, textbox.getInitX());
}

void gGUIGrid::changeCellFontColor(gColor *fontColor) {
	if(allcells.at(selectedbox).iscellselected) {
		allcells.at(selectedbox).cellfontcolor = fontColor;
		textbox.setTextColor(fontColor);
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
	else if(isrowselected) drawSelectedRow();
	else if(iscolumnselected) drawSelectedColumn();
	drawCellContents();
	textbox.setFirstX(firstx);
	textbox.setFirstY(firsty);
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
	gDrawRectangle(gridx + (gridboxw / 2) - firstx, gridy + gridboxh - firsty, gridw, gridh, true);
}

void gGUIGrid::drawSelectedBox() {
	renderer->setColor(0.0f, 1.0f, 0.0f, 1.0f);
	gDrawRectangle(allcells.at(selectedbox).cellx + 1 - firstx, (allcells.at(selectedbox).celly + 1) - firsty, gridboxw - 2, gridboxh - 2, false);
	gDrawRectangle(allcells.at(selectedbox).cellx + (gridboxw - 2) - 6 - firstx, allcells.at(selectedbox).celly + (gridboxh - 2) - 4 - firsty, 6, 6, true); // FLAG
}

void gGUIGrid::drawSelectedRow() {
	renderer->setColor(0.0f, 1.0f, 0.0f, 1.0f);
	gDrawRectangle(gridx + (gridboxw / 2) + 1 - firstx, gridy + gridboxh * selectedtitle + 1 - firsty, gridw - 2, gridboxh - 2, false);
	gDrawRectangle(gridx + (gridboxw / 2) + gridw - 2 - 6 - firstx, gridy + gridboxh * selectedtitle + (gridboxh - 2) - 4 - firsty, 6, 6, true); // FLAG
}

void gGUIGrid::drawSelectedColumn() {
	renderer->setColor(0.0f, 1.0f, 0.0f, 1.0f);
	gDrawRectangle(gridx + (gridboxw / 2) + 1 + (gridboxw * (selectedtitle - 1)) - firstx, gridy + gridboxh + 1 - firsty, gridboxw - 2, gridh - 2, false);
	gDrawRectangle(gridx + (gridboxw / 2) + (gridboxw * (selectedtitle - 1)) + gridboxw - 2 - 6 - firstx, gridy + gridboxh + gridh - 2 - 4 - firsty, 6, 6, true); // FLAG
}

void gGUIGrid::drawTitleRowBackground() {
	renderer->setColor(*buttoncolor);
	gDrawRectangle(gridx, gridy - firsty, gridboxw / 2, gridh + gridboxh, true);
}

void gGUIGrid::drawTitleColumnBackground() {
	renderer->setColor(*buttoncolor);
	gDrawRectangle(gridx - firstx, gridy, gridw + (gridboxw / 2), gridboxh , true);
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
		gDrawLine(gridx - firstx, gridy + ((i + 1) * gridboxh) - firsty, gridx + gridw + (gridboxw / 2) - firstx, gridy + ((i + 1) * gridboxh) - firsty);
	}
}

void gGUIGrid::drawColumnContents() {
	int tempcol = columntitle;
	for(int i = 1; i <= columnnum; i++) {
	    std::string columntitlestring(1, (char) tempcol);
	    renderer->setColor(*fontcolor);
		font->drawText(columntitlestring, gridx + (i * gridboxw) - (font->getStringWidth(columntitlestring) / 2) - firstx, gridy + (gridboxh / 2) + (font->getStringHeight(columntitlestring) / 2));
		tempcol++;
		renderer->setColor(*pressedbuttoncolor);
		gDrawLine(gridx - (gridboxw / 2) + ((i + 1) * gridboxw) - firstx, gridy - firsty, gridx - (gridboxw / 2) + ((i + 1) * gridboxw) - firstx, gridy + gridboxh +gridh - firsty);
		gDrawRectangle(gridx, gridy, allcells.at(0).cellw / 2, allcells.at(0).cellh, true);
	}
}

void gGUIGrid::drawTitleLines() {
	renderer->setColor(*backgroundcolor);
	gDrawLine(gridx + (gridboxw / 2) + 1 - firstx, gridy - firsty, gridx + (gridboxw / 2) + 1 - firstx, gridy + gridboxh + gridh - firsty);
	gDrawLine(gridx - firstx, gridy + (gridboxh) - firsty, gridx + gridboxw / 2 + gridw - firstx, gridy + (gridboxh) - firsty);
}

void gGUIGrid::drawCellContents() {
	int cellindexcounter = 0;
	for(int i = 0; i < rownum; i++) {
		for(int j = 0; j < columnnum; j++) {
			renderer->setColor(allcells.at(cellindexcounter).cellfontcolor);
			manager->getFont(allcells.at(cellindexcounter).fontnum)->drawText(allcells.at(cellindexcounter).showncontent, allcells.at(cellindexcounter).cellx + (allcells.at(cellindexcounter).cellw - manager->getFont(allcells.at(cellindexcounter).fontnum)->getStringWidth(allcells.at(cellindexcounter).showncontent)) * allcells.at(cellindexcounter).textmoveamount - textbox.getInitX() * allcells.at(cellindexcounter).cellalignment - firstx, allcells.at(cellindexcounter).celly + (gridboxh / 2) + (manager->getFont(allcells.at(cellindexcounter).fontnum)->getStringHeight(allcells.at(cellindexcounter).showncontent) / 2) - firsty);
			cellindexcounter++;
		}
	}
}

void gGUIGrid::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	int pressedx = x - left - firstx;
	int pressedy = y - top - firsty - titledy;
	if(!(pressedy < gridy + gridboxh - firsty && pressedx < gridx + (gridboxw / 2) - firstx) && pressedx >= gridx - firstx && pressedx <= gridx + (gridboxw / 2) + gridw - firstx && pressedy >= gridy - firsty && pressedy <= gridy + gridboxh + gridh - firsty) {
		if(pressedx >= gridx + (gridboxw / 2) - firstx && pressedx <= gridx + (gridboxw / 2) + gridw - firstx && pressedy >= gridy + gridboxh - firsty && pressedy <= gridy + gridboxh + gridh - firsty) {
			isselected = true;
			isrowselected = false;
			iscolumnselected = false;
			int newcellindex = ((int)((x + firstx - left - (gridboxw / 2)) / gridboxw)) + ((int)((y + firsty - top - gridboxh - titletopmargin + ((font->getSize() * 1.8f) * !istitleon)) / gridboxh))  * columnnum ; // * gridboxw + (gridboxw / 2);
			if(newcellindex != selectedbox) {
				if(istextboxactive) changeCell();
				if(newcellindex != selectedbox) {
					allcells.at(selectedbox).iscellselected = false;
					allcells.at(newcellindex).iscellselected = true;
				}
				selectedbox = newcellindex;
			}
		}
		else if(pressedx >= gridx - firstx && pressedx < gridx + (gridboxw / 2) - firstx && pressedy >= gridy + gridboxh - firsty && pressedy <= gridy + gridboxh + gridh - firsty) {
			selectedtitle = ceil((pressedy + (firsty * 2)) / gridboxh) - 1;
			selectedbox = columnnum * (selectedtitle - 1);
			isselected = false;
			isrowselected = true;
			iscolumnselected = false;
		}
		else if(pressedx >= gridx + (gridboxw / 2) - firstx && pressedx < gridx + (gridboxw / 2) + gridw - firstx && pressedy >= gridy - firsty && pressedy <= gridy + gridboxh - firsty) {
			selectedtitle = ceil((pressedx - gridboxw / 2 + (firstx * 2)) / gridboxw);
			selectedbox = selectedtitle - 1;
			isselected = false;
			isrowselected = false;
			iscolumnselected = true;
		}

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
	}

}

void gGUIGrid::mouseReleased(int x, int y, int button) {
	if(istextboxactive) textbox.mouseReleased((x - left), (y - top - firsty - titledy), button);
	gGUIScrollable::mouseReleased(x, y, button);
}

void gGUIGrid::mouseDragged(int x, int y, int button) {
//	gLogi("Grid") << x << " " << y;
	if(istextboxactive) textbox.mouseDragged((x - left), (y - top - firsty), button);
	gGUIScrollable::mouseDragged(x, y, button);
}

void gGUIGrid::keyPressed(int key){
	if(istextboxactive) textbox.keyPressed(key);
	else if(isselected || isrowselected || iscolumnselected) {
		textbox.cleanText();
		allcells.at(selectedbox).cellcontent = "";
		allcells.at(selectedbox).showncontent = "";
		createTextBox();
		textbox.mousePressed(allcells.at(selectedbox).cellx + textbox.getInitX(), allcells.at(selectedbox).celly + textbox.getInitX(), 0);
		textbox.keyPressed(key);
		istextboxactive = true;
	}
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
