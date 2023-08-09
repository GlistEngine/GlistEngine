/*
 * gGUIGrid.cpp
 *
 *  Created on: 5 Aug 2022
 *      Author: murat
 */

#include "gGUIGrid.h"
#include "gBaseApp.h"
#include <array>

gGUIGrid::gGUIGrid() {
	selectedframecolor = gColor(0.1f, 0.45f, 0.87f, 1.0f);
	selectedareacolor = gColor(0.85f, 0.85f, 0.9f, 1.0f);
	selectedareadarkcolor = gColor(0.2f, 0.2f, 0.22f, 1.0f);
	isselected = false;
	isrowselected = false;
	iscolumnselected = false;
	isdoubleclicked = false;
	shiftpressed = false;
	ctrlpressed = false;
	ctrlcpressed = false;
	ctrlvpressed = false;
	ctrlzpressed = false;
	ctrlypressed = false;
	selectedbox = 0;
	selectedtitle = 0;
	rownum = 50;
	columnnum = 10;
	rowtitle = 1;
	columntitle = 65; // 'A' char in ASCII
	cursor = gGUIForm::CURSOR_ARROW;
	currentrow = 0;
	currentcolumn = 0;
	firstcursorposx = 0;
	firstcursorposy = 0;
	firstselectedcell = -1;
	lastselectedcell = -1;
	gridboxw = 80.0f;
	gridboxh = 30.0f;
	gridx = 0.0f;
	gridy = 0.0f;
	gridw = gridboxw * columnnum;
	gridh = gridboxh * rownum;
	clicktimediff = 250;
	titlediff = 20;
	clicktime = gGetSystemTimeMillis();
	previousclicktime = clicktime - 2 * clicktimediff;
	firstclicktime = previousclicktime - 2 * clicktimediff;
	strflag = "";
	totalw = columnnum * gridboxw;
	totalh = rownum * gridboxh;
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
	if(rowNum > maxrownum) rownum = maxrownum;
	else rownum = rowNum;
	if(columnNum > maxcolumnnum) columnnum = maxcolumnnum;
	else columnnum = columnNum;
	gridw = gridboxw * columnnum;
	gridh = gridboxh * rownum;
}

void gGUIGrid::setRowNum(int rowNum) {
	rownum = rowNum;
}

void gGUIGrid::setColumnNum(int columnNum) {
	columnnum = columnNum;
}

void gGUIGrid::update() {
	textbox.update();
}

void gGUIGrid::setCellFont(int fontNum) {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		addUndoStack(PROCESS_FONT);
		for(int i = 0; i < selectedcells.size(); i++) {
			allcells[selectedcells[i]].fontnum = fontNum;
			allcells[selectedcells[i]].fontstate = gGUIManager::FONTTYPE_REGULAR;
			if(allcells[selectedcells[i]].isbold) allcells[selectedcells[i]].fontstate += gGUIManager::FONTTYPE_BOLD;
			if(allcells[selectedcells[i]].isitalic) allcells[selectedcells[i]].fontstate += gGUIManager::FONTTYPE_ITALIC;
		}
	}
	else {
		if(!ctrlypressed) {
			allcells[undocellstack.top()[0]].fontnum = fontNum;
			allcells[undocellstack.top()[0]].fontstate = gGUIManager::FONTTYPE_REGULAR;
			if(allcells[undocellstack.top()[0]].isbold) allcells[undocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_BOLD;
			if(allcells[undocellstack.top()[0]].isitalic) allcells[undocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_ITALIC;
		}
		else {
			allcells[redocellstack.top()[0]].fontnum = fontNum;
			allcells[redocellstack.top()[0]].fontstate = gGUIManager::FONTTYPE_REGULAR;
			if(allcells[redocellstack.top()[0]].isbold) allcells[redocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_BOLD;
			if(allcells[redocellstack.top()[0]].isitalic) allcells[redocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_ITALIC;
		}
	}
	textbox.setTextFont(manager->getFont(allcells[selectedbox].fontnum, allcells[selectedbox].fontstate));
}

void gGUIGrid::setCellFontBold() {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		addUndoStack(PROCESS_FONTSTATE);
		for(int i = 0; i < selectedcells.size(); i++) {
			if(allcells[selectedcells[i]].isbold) {
				allcells[selectedcells[i]].isbold = false;
				allcells[selectedcells[i]].fontstate -= gGUIManager::FONTTYPE_BOLD;
			}
			else {
				allcells[selectedcells[i]].isbold = true;
				allcells[selectedcells[i]].fontstate += gGUIManager::FONTTYPE_BOLD;
			}
		}
	}
	else {
		if(!ctrlypressed) {
			if(allcells[undocellstack.top()[0]].isbold) {
				allcells[undocellstack.top()[0]].isbold = false;
				allcells[undocellstack.top()[0]].fontstate -= gGUIManager::FONTTYPE_BOLD;
			}
			else {
				allcells[undocellstack.top()[0]].isbold = true;
				allcells[undocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_BOLD;
			}
		}
		else {
			if(allcells[redocellstack.top()[0]].isbold) {
				allcells[redocellstack.top()[0]].isbold = false;
				allcells[redocellstack.top()[0]].fontstate -= gGUIManager::FONTTYPE_BOLD;
			}
			else {
				allcells[redocellstack.top()[0]].isbold = true;
				allcells[redocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_BOLD;
			}
		}
	}
	textbox.setTextFont(manager->getFont(allcells[selectedbox].fontnum, allcells[selectedbox].fontstate));
}

void gGUIGrid::setCellFontItalic() {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		addUndoStack(PROCESS_FONTSTATE);
		for(int i = 0; i < selectedcells.size(); i++) {
			if(allcells[selectedcells[i]].isitalic) {
				allcells[selectedcells[i]].isitalic = false;
				allcells[selectedcells[i]].fontstate -= gGUIManager::FONTTYPE_ITALIC;
			}
			else {
				allcells[selectedcells[i]].isitalic = true;
				allcells[selectedcells[i]].fontstate += gGUIManager::FONTTYPE_ITALIC;
			}
		}
	}
	else {
		if(!ctrlypressed) {
			if(allcells[undocellstack.top()[0]].isitalic) {
				allcells[undocellstack.top()[0]].isitalic = false;
				allcells[undocellstack.top()[0]].fontstate -= gGUIManager::FONTTYPE_ITALIC;
			}
			else {
				allcells[undocellstack.top()[0]].isitalic = true;
				allcells[undocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_ITALIC;
			}
		}
		else {
			if(allcells[redocellstack.top()[0]].isitalic) {
				allcells[redocellstack.top()[0]].isitalic = false;
				allcells[redocellstack.top()[0]].fontstate -= gGUIManager::FONTTYPE_ITALIC;
			}
			else {
				allcells[redocellstack.top()[0]].isitalic = true;
				allcells[redocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_ITALIC;
			}
		}
	}
}

void gGUIGrid::setCellAlignment(int cellAlignment, bool clicked) {
	if(allcells.empty()) return;
	if(clicked) {
		addUndoStack(PROCESS_ALIGNMENT);
		for(int i = 0; i < selectedcells.size(); i++) {
			if(clicked && !allcells[selectedcells[i]].iscellaligned) allcells[selectedcells[i]].iscellaligned = true;
			else if(clicked && allcells[selectedcells[i]].iscellaligned && allcells[selectedcells[i]].celltype == Cell::TYPE_DIGIT) {
				allcells[selectedcells[i]].iscellaligned = false;
				cellAlignment = gBaseGUIObject::TEXTALIGNMENT_RIGHT;
			}
			else if(clicked && allcells[selectedcells[i]].cellalignment == cellAlignment) {
				allcells[selectedcells[i]].iscellaligned = false;
				cellAlignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
			}
			allcells[selectedcells[i]].cellalignment = cellAlignment;
			allcells[selectedcells[i]].textmoveamount = 0.5f * cellAlignment;
		}
	}
	else {
		if(!ctrlzpressed && !ctrlypressed) {
			allcells[selectedbox].cellalignment = cellAlignment;
			allcells[selectedbox].textmoveamount = 0.5f * cellAlignment;
		}
		else {
			if(!ctrlypressed) {
				allcells[undocellstack.top()[0]].cellalignment = cellAlignment;
				allcells[undocellstack.top()[0]].textmoveamount = 0.5f * cellAlignment;
			}
			else {
				allcells[redocellstack.top()[0]].cellalignment = cellAlignment;
				allcells[redocellstack.top()[0]].textmoveamount = 0.5f * cellAlignment;
			}
		}
	}
	textbox.setTextAlignment(cellAlignment, allcells[selectedbox].cellw, textbox.getInitX());
}

void gGUIGrid::setCellFontColor(gColor *fontColor) {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		addUndoStack(PROCESS_COLOR);
		for(int i = 0; i < selectedcells.size(); i++) {
			allcells[selectedcells[i]].cellfontcolor = fontColor;
			if(fontColor != fontcolor) allcells[selectedcells[i]].iscolorchanged = true;
			else allcells[selectedcells[i]].iscolorchanged = false;
		}
	}
	else {
		if(!ctrlypressed) {
			allcells[undocellstack.top()[0]].cellfontcolor = fontColor;
			if(fontColor != fontcolor) allcells[undocellstack.top()[0]].iscolorchanged = true;
			else allcells[undocellstack.top()[0]].iscolorchanged = false;
		}
		else {
			allcells[redocellstack.top()[0]].cellfontcolor = fontColor;
			if(fontColor != fontcolor) allcells[redocellstack.top()[0]].iscolorchanged = true;
			else allcells[redocellstack.top()[0]].iscolorchanged = false;
		}
	}
	textbox.setTextColor(fontColor);
}

void gGUIGrid::setCellLine(int lineNo, bool clicked) {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		addUndoStack(PROCESS_LINE);
		for(int i = 0; i < selectedcells.size(); i++) {
			if(clicked && lineNo == allcells[selectedcells[i]].lineno) allcells[selectedcells[i]].lineno = TEXTLINE_NONE;
			else allcells[selectedcells[i]].lineno = lineNo;
		}
	}
	else {
		if(!ctrlypressed) allcells[undocellstack.top()[0]].lineno = lineNo;
		else allcells[redocellstack.top()[0]].lineno = lineNo;
	}
}

void gGUIGrid::setSelectedFrameColor(gColor* selectedFrameColor) {
	selectedframecolor = *selectedFrameColor;
}

void gGUIGrid::setSelectedAreaColor(gColor* selectedAreaColor) {
	selectedareacolor = *selectedAreaColor;
}

void gGUIGrid::setCellContent(gGUIGrid::Cell* cell, std::string cellContent) {
	cell->cellcontent = cellContent;
	int cellno = -1;
	for(int i = 0; i < allcells.size(); i++) {
		if(allcells[i].cellrowno == cell->cellrowno && allcells[i].cellcolumnno == cell->cellcolumnno) {
			cellno = i;
			break;
		}
	}
	changeCell(cellno);
}

void gGUIGrid::setCellContent(std::string cell, std::string cellContent) {
	Cell* c = getCell(cell);
	setCellContent(c, cellContent);
}

void gGUIGrid::setCellAlignment(Cell* cell, int cellAlignment) {
	cell->cellalignment = cellAlignment;
	cell->textmoveamount = 0.5f * cellAlignment;
}
void gGUIGrid::setCellAlignment(std::string cell, int cellAlignment) {
	Cell* c = getCell(cell);
	setCellAlignment(c, cellAlignment);
}

gGUIGrid::Cell* gGUIGrid::getCell(std::string cellID) {
	std::transform(cellID.begin(), cellID.end(), cellID.begin(), ::toupper);
	std::string column = getTextColumn(cellID);
	std::string row = cellID.substr(column.size(), cellID.size() - column.size());
	int columnindex = int(column[column.size() - 1]) % 65;
	for(int i = 0; i < column.size() - 1; i++) columnindex += int((column[i] % 65 + 1) * (26 * ((column.size() - 1) - i)));
	int index = getCellNo(std::stoi(row) - 1, columnindex);
	if(index == -1) {
		createCell(std::stoi(row) - 1, columnindex);
		index = allcells.size() - 1;
	}
	return &allcells[index];
}

gColor* gGUIGrid::getSelectedFrameColor() {
	return &selectedframecolor;
}

gColor* gGUIGrid::getSelectedAreaColor() {
	return &selectedareacolor;
}

std::string gGUIGrid::fixTextFunction(std::string text, int index) {
	if(text == "") return text;
	std::string tempstr = text;
	bool function = (tempstr[0] == '=');
	if(int(tempstr[0]) == 39) tempstr.erase(0, 1);
	else if(function) {
		tempstr.erase(0, function);
		bool isnegative = false;
		std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::toupper);
		tempstr = fixNumeric(tempstr);
		isnegative = (tempstr[0] == '-');
		tempstr.erase(0, isnegative);
		bool hasdigit = isNumeric(tempstr);

		if(!isdigit(tempstr[0])) {
			int parentheses1 = tempstr.find('(');
			int parentheses2 = tempstr.find(')');
			if(parentheses1 != std::string::npos && parentheses2 != std::string::npos) {
				std::string functionstr = tempstr.substr(0, parentheses1);
				std::string parenthesesstr = tempstr.substr(parentheses1 + 1, parentheses2 - parentheses1 - 1);
				parenthesesstr = fixNumeric(parenthesesstr);
				bool isparenthesesnegative = (parenthesesstr[0] == '-');
				parenthesesstr.erase(0, isparenthesesnegative);
				if(functionstr == "SUM") {
					int doubledat = parenthesesstr.find(':');
					if(doubledat == std::string::npos) {
						if(!isNumeric(parenthesesstr)) {
							std::string column = getTextColumn(parenthesesstr);
							std::string row = parenthesesstr.substr(column.size(), parenthesesstr.size() - column.size());
							int columnindex = int(column[column.size() - 1]) % 65;
							for(int i = 0; i < column.size() - 1; i++) columnindex += int((column[i] % 65 + 1) * (26 * ((column.size() - 1) - i)));
							int copiedindex = getCellNo(std::stoi(row), columnindex);
							if(copiedindex == -1) {
								createCell(std::stoi(row) - 1, columnindex);
								copiedindex = allcells.size() - 1;
							}
							tempstr = allcells[copiedindex].showncontent;
							functionindexes.push_back(index);
							if(text != strflag) addFunction(FUNCTION_COPY, copiedindex);
						}
					}
					else {
						std::string str1 = parenthesesstr.substr(0, doubledat);
						std::string str2 = parenthesesstr.substr(str1.size() + 1, parenthesesstr.size());
						int columnno1 = int(str1[0]) % 65;
						int columnno2 = int(str2[0]) % 65;
						int rowno1 = std::stoi(str1.substr(1, str1.size())) - 1;
						int rowno2 = std::stoi(str2.substr(1, str2.size())) - 1;

						tempstr = std::to_string(makeSum(columnno1, rowno1, columnno2, rowno2));
						while(tempstr[tempstr.size() - 1] == '0') tempstr.erase(tempstr.size() - 1, 1);
						if(tempstr[tempstr.size() - 1] == '.') tempstr.erase(tempstr.size() - 1, 1);
						if(text != strflag) addFunction(FUNCTION_SUM, index);
					}
					if((isnegative && !isparenthesesnegative) || (!isnegative && isparenthesesnegative)) tempstr = "-" + tempstr;
				}
			}
			else if(int(tempstr[0]) >= 65 && int(tempstr[0]) < 91) {
				if(tempstr.size() > 1) {
					std::string tstr = tempstr.substr(1, tempstr.size());
					int found = tstr.find_first_not_of("0123456789");
					if(found == std::string::npos) {
						int copiedindex = getCellNo(std::stoi(tstr) - 1, int(tempstr[0]) % 65);
						if(copiedindex == -1) {
							createCell(std::stoi(tstr) - 1, int(tempstr[0]) % 65);
							copiedindex = allcells.size() - 1;
						}
						tempstr = allcells[copiedindex].showncontent;
						if(isnegative) tempstr = "-" + tempstr;
						functionindexes.push_back(index);
						if(text != strflag) addFunction(FUNCTION_COPY, copiedindex);
					}
				}
			}
		}
	}
	return tempstr;
}

std::string gGUIGrid::fixNumeric(std::string text) {
	std::string tempstr = text;
	std::stack<int> unnecessaryindexes;
	if(tempstr[0] == '+') {
		unnecessaryindexes.push(0);
		for(int i = 1; i < tempstr.size(); i++) {
			if(tempstr[i] != '+' && tempstr[i] != '-') break;
			if(tempstr[i] == '+') unnecessaryindexes.push(i);
			else if(tempstr[i] != '-') continue;
		}
		while(!unnecessaryindexes.empty()) {
			tempstr.erase(unnecessaryindexes.top(), 1);
			unnecessaryindexes.pop();
		}
	}
	if(tempstr[0] == '-') {
		int minuscount = 1;
		for(int i = 1; i < tempstr.size(); i++) {
			if(tempstr[i] == '-') minuscount++;
			else if(tempstr[i] == '+') unnecessaryindexes.push(i);
			else break;
		}
		while(!unnecessaryindexes.empty()) {
			tempstr.erase(unnecessaryindexes.top(), 1);
			unnecessaryindexes.pop();
		}
		if(minuscount % 2 == 1) tempstr.erase(0, minuscount - 1);
		else tempstr.erase(0, minuscount);
	}
	return tempstr;
}

std::string gGUIGrid::fixOverflowText(Cell& thisCell, Cell& otherCell) {
	std::string tmpstr = "";
	int diff = 0;
	int charindex = 0;
	bool middlealignment = (thisCell.cellalignment == gBaseGUIObject::TEXTALIGNMENT_MIDDLE);

	if(otherCell.cellx > thisCell.cellx)
		diff = otherCell.cellx - (thisCell.cellx + (middlealignment * thisCell.cellw / 2) + (!middlealignment * (textbox.getInitX() + 2) * 2));
	else
		diff = (thisCell.cellx + thisCell.cellw - (middlealignment * thisCell.cellw / 2)) - (otherCell.cellx + otherCell.cellw + (!middlealignment * (textbox.getInitX() + 2) * 2));

	while(manager->getFont(thisCell.fontnum)->getStringWidth(tmpstr) < diff) {
		tmpstr += thisCell.showncontent[charindex];
		charindex++;
	}
	return tmpstr;
}

std::string gGUIGrid::getTextColumn(std::string text) {
    int found = text.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (found != std::string::npos && std::isalpha(text[0])) return text.substr(0, found);
    return text;
}

int gGUIGrid::getCellNo(int rowNo, int columnNo) {
	for(int i = 0; i < allcells.size(); i++)
		if(allcells[i].cellrowno == rowNo && allcells[i].cellcolumnno == columnNo) return i;
	return -1;
}

int gGUIGrid::getNearestFilledCell(int index) {
	int nearestindex = -1;
	for(int i = 0; i < allcells.size(); i++) {
		if(i != index && allcells[i].cellrowno == allcells[index].cellrowno && !allcells[i].showncontent.empty()) {
			if(allcells[index].cellalignment != gBaseGUIObject::TEXTALIGNMENT_RIGHT && allcells[index].cellx + font->getStringWidth(allcells[index].showncontent) > allcells[i].cellx && allcells[index].cellcolumnno < allcells[i].cellcolumnno) {
				if(nearestindex != -1 && allcells[nearestindex].cellcolumnno > allcells[i].cellcolumnno) nearestindex = i;
				else if(nearestindex == -1) nearestindex = i;
			}
			else if(allcells[index].cellalignment == gBaseGUIObject::TEXTALIGNMENT_RIGHT && allcells[index].cellx - (font->getStringWidth(allcells[index].showncontent) - allcells[index].cellw) < allcells[i].cellx + allcells[i].cellw && allcells[index].cellcolumnno > allcells[i].cellcolumnno) {
				if(nearestindex != -1 && allcells[nearestindex].cellcolumnno < allcells[i].cellcolumnno) nearestindex = i;
				else if(nearestindex == -1) nearestindex = i;
			}
		}
	}
	return nearestindex;
}

float gGUIGrid::getColumnWidth(int columnNo) {
	float w = gridboxw;
	for(int i = 0; i < gridboxesw.size(); i++) {
		if(gridboxesw[i][0] == columnNo) {
			w = gridboxesw[i][1];
			break;
		}
	}
	return w;
}
float gGUIGrid::getRowHeight(int rowNo) {
	float h = gridboxh;
	for(int i = 0; i < gridboxesh.size(); i++) {
		if(gridboxesh[i][0] == rowNo) {
			h = gridboxesh[i][1];
			break;
		}
	}
	return h;
}

void gGUIGrid::fillCell(int cellNo, std::string tempstr) { //when rowNo = 1, columnNO = 4; tempstr = "happyyyy";
	if(cellNo > rownum * columnnum - 1) return;
	int cellindex = cellNo;
	allcells[cellindex].cellcontent = tempstr;
	allcells[cellindex].showncontent = fixTextFunction(tempstr, cellindex);

	checkCellType(cellindex);

	int nearestindex = -1;
	for(int i = 0; i < allcells.size(); i++) {
		if(allcells[i].cellrowno == allcells[cellindex].cellrowno) {
			nearestindex = getNearestFilledCell(i);
			if(nearestindex != -1) allcells[i].overflowcontent = fixOverflowText(allcells[i], allcells[nearestindex]);
			else allcells[i].overflowcontent = "";
		}
	}
	if(allcells[cellindex].cellw < font->getStringWidth(allcells[cellindex].showncontent)) {
		nearestindex = getNearestFilledCell(cellindex);
		if(nearestindex != -1) allcells[cellindex].overflowcontent = fixOverflowText(allcells[cellindex], allcells[nearestindex]);
		else allcells[cellindex].overflowcontent = "";
	}
}

float gGUIGrid::makeSum(int c1, int r1, int c2, int r2) {
	if(c1 > c2) {
		int flag = c1;
		c1 = c2;
		c2 = flag;
	}
	if(r1 > r2) {
		int flag = r1;
		r1 = r2;
		r2 = flag;
	}
	float result = 0;
	for(int row = r1; row <= r2; row++) {
		for(int column = c1; column <= c2; column++) {
			int currentindex = getCellNo(row, column);
			if(currentindex == -1) {
				createCell(row, column);
				currentindex = allcells.size() - 1;
			}
			functionindexes.push_back(currentindex);
			if(allcells[currentindex].celltype == Cell::TYPE_DIGIT) {
				result += std::stof(allcells[currentindex].showncontent);
			}
		}
	}
	return result;
}

float gGUIGrid::calculateCurrentX(int columnNum) {
	float currentx = gridx + gridboxw / 2 - firstx;
	for(int column = 0; column < columnNum; column++) currentx += getColumnWidth(column);
	return currentx;
}

float gGUIGrid::calculateCurrentY(int rowNum) {
	float currenty = gridy + gridboxh - firsty;
	for(int row = 0; row < rowNum; row++) currenty += getRowHeight(row);
	return currenty;
}

bool gGUIGrid::isNumeric(std::string text) {
	bool hasdigit = true;
	for(int i = 0; i < text.size(); i++) {
		if(!isdigit(text[i])) {
			hasdigit = false;
			break;
		}
	}
	return hasdigit;
}

void gGUIGrid::addFunction(int functionType, int functionSender) {
	int same = -1;
	functionindexes.insert(functionindexes.begin(), functionSender);
	functionindexes.insert(functionindexes.begin(), functionType);
	for(int i = 0; i < functions.size(); i++) {
		if(functionSender == functions[i][FUNCTION_SENDER] && functions[i][FUNCTION_TYPE] != FUNCTION_COPY) {
			same = i;
			break;
		}
	}
	if(same != -1) functions[same] = functionindexes;
	else functions.push_back(functionindexes);
	functionindexes.clear();
	operateFunction(functions.size() - 1);
}

void gGUIGrid::addUndoStack(int process) {
	while(!undovaluestack.empty()) undovaluestack.pop();
	undoprocessstack.push(process);
	std::deque<int> tmpcell;
	if(ctrlypressed) {
		undocellstack.push(redocellstack.top());
		for(int i = 0; i < undocellstack.top().size(); i++) {
			switch(undoprocessstack.top()) {
			case PROCESS_TEXT:
				undovaluestack.push(allcells[undocellstack.top()[i]].cellcontent);
				break;
			case PROCESS_FONT:
				undovaluestack.push(std::to_string(allcells[undocellstack.top()[i]].fontnum));
				break;
			case PROCESS_FONTSTATE:
				undovaluestack.push(std::to_string(allcells[undocellstack.top()[i]].fontstate));
				break;
			case PROCESS_ALIGNMENT:
				undovaluestack.push(std::to_string(allcells[undocellstack.top()[i]].cellalignment));
				break;
			case PROCESS_COLOR:
				undovaluestack.push(std::to_string(allcells[undocellstack.top()[i]].cellfontcolor.r) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellfontcolor.g) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellfontcolor.b) + ":" + std::to_string(allcells[undocellstack.top()[i]].iscolorchanged));
				break;
			case PROCESS_LINE:
				undovaluestack.push(std::to_string(allcells[undocellstack.top()[i]].lineno));
				break;
			case PROCESS_ALL:
				undovaluestack.push(allcells[undocellstack.top()[i]].cellcontent + ":" + std::to_string(allcells[undocellstack.top()[i]].fontnum) + ":" + std::to_string(allcells[undocellstack.top()[i]].fontstate) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellalignment) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellfontcolor.r) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellfontcolor.g) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellfontcolor.b) + ":" + std::to_string(allcells[undocellstack.top()[i]].lineno));
				break;
			}
		}
		undostack.push(undovaluestack);
	}
	else {
		switch(undoprocessstack.top()) {
		case PROCESS_TEXT:
			tmpcell.push_back(selectedbox);
			undocellstack.push(tmpcell);
			undovaluestack.push(strflag);
			break;
		case PROCESS_FONT:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				undovaluestack.push(std::to_string(allcells[selectedcells[i]].fontnum));
			}
			break;
		case PROCESS_FONTSTATE:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				undovaluestack.push(std::to_string(allcells[selectedcells[i]].fontstate));
			}
			break;
		case PROCESS_ALIGNMENT:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				undovaluestack.push(std::to_string(allcells[selectedcells[i]].cellalignment));
			}
			break;
		case PROCESS_COLOR:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				std::string tmpstr = std::to_string(allcells[selectedcells[i]].cellfontcolor.r) + ":" + std::to_string(allcells[selectedcells[i]].cellfontcolor.g) + ":" + std::to_string(allcells[selectedcells[i]].cellfontcolor.b) + ":" + std::to_string(allcells[selectedcells[i]].iscolorchanged);
				undovaluestack.push(tmpstr);
			}
			break;
		case PROCESS_LINE:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				undovaluestack.push(std::to_string(allcells[selectedcells[i]].lineno));;
			}
			break;
		case PROCESS_ALL:
			if(!ctrlvpressed) {
				undocellstack.push(selectedcells);
				std::reverse(undocellstack.top().begin(), undocellstack.top().end());
				for(int i = 0; i < selectedcells.size(); i++) {
					int amount = selectedcells.at(i) - selectedbox;
					std::string tmpstr = allcells[selectedbox + amount].cellcontent + ":" + std::to_string(allcells[selectedbox + amount].fontnum) + ":" + std::to_string(allcells[selectedbox + amount].fontstate) + ":" + std::to_string(allcells[selectedbox + amount].cellalignment) + ":" + std::to_string(allcells[selectedbox + amount].cellfontcolor.r) + ":" + std::to_string(allcells[selectedbox + amount].cellfontcolor.g) + ":" + std::to_string(allcells[selectedbox + amount].cellfontcolor.b) + ":" + std::to_string(allcells[selectedbox + amount].iscolorchanged) + ":" + std::to_string(allcells[selectedbox + amount].lineno);
					undovaluestack.push(tmpstr);
				}
			}
			else {
				int index = selectedbox;
				int row = (allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno) / columnnum;
				int column = (allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno) % columnnum;
				int colon = copiedcellvalues.at(copiedcellvalues.size() - 1).find(':');
				int maxcolumn = column + std::stoi(copiedcellvalues.at(copiedcellvalues.size() - 1).substr(0, colon));
				colon = copiedcellvalues.at(0).find(':');
				maxcolumn -= std::stoi(copiedcellvalues.at(0).substr(0, colon));
				std::deque<int> indexes;
				for(int i = 0; i < copiedcellvalues.size(); i++) {
					if(column > maxcolumn) {
						column = (allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno) % columnnum;
						row++;
					}
					index = getCellNo(row, column);
					if(index == -1) {
						createCell(row, column);
						index = allcells.size() - 1;
					}
					indexes.push_back(index);
					column++;
				}
				undocellstack.push(indexes);
				std::reverse(undocellstack.top().begin(), undocellstack.top().end());
				for(int i = 0; i < indexes.size(); i++) {
					int amount = indexes.at(i) - selectedbox;
					std::string tmpstr = allcells[selectedbox + amount].cellcontent + ":" + std::to_string(allcells[selectedbox + amount].fontnum) + ":" + std::to_string(allcells[selectedbox + amount].fontstate) + ":" + std::to_string(allcells[selectedbox + amount].cellalignment) + ":" + std::to_string(allcells[selectedbox + amount].cellfontcolor.r) + ":" + std::to_string(allcells[selectedbox + amount].cellfontcolor.g) + ":" + std::to_string(allcells[selectedbox + amount].cellfontcolor.b) + ":" + std::to_string(allcells[selectedbox + amount].iscolorchanged) + ":" + std::to_string(allcells[selectedbox + amount].lineno);
					undovaluestack.push(tmpstr);
				}
			}
			break;
		}
		undostack.push(undovaluestack);
	}
}

void gGUIGrid::addRedoStack() {
	while(!redovaluestack.empty()) redovaluestack.pop();
	redoprocessstack.push(undoprocessstack.top());
	redocellstack.push(undocellstack.top());
	for(int i = 0; i < redocellstack.top().size(); i++) {
		switch(redoprocessstack.top()) {
		case PROCESS_TEXT:
			redovaluestack.push(allcells[redocellstack.top()[i]].cellcontent);
			break;
		case PROCESS_FONT:
			redovaluestack.push(std::to_string(allcells[redocellstack.top()[i]].fontnum));
			break;
		case PROCESS_FONTSTATE:
			redovaluestack.push(std::to_string(allcells[redocellstack.top()[i]].fontstate));
			break;
		case PROCESS_ALIGNMENT:
			redovaluestack.push(std::to_string(allcells[redocellstack.top()[i]].cellalignment));
			break;
		case PROCESS_COLOR:
			redovaluestack.push(std::to_string(allcells[redocellstack.top()[i]].cellfontcolor.r) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellfontcolor.g) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellfontcolor.b) + ":" + std::to_string(allcells[redocellstack.top()[i]].iscolorchanged));
			break;
		case PROCESS_LINE:
			redovaluestack.push(std::to_string(allcells[redocellstack.top()[i]].lineno));
			break;
		case PROCESS_ALL:
			redovaluestack.push(allcells[redocellstack.top()[i]].cellcontent + ":" + std::to_string(allcells[redocellstack.top()[i]].fontnum) + ":" + std::to_string(allcells[redocellstack.top()[i]].fontstate) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellalignment) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellfontcolor.r) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellfontcolor.g) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellfontcolor.b) + ":" + std::to_string(allcells[redocellstack.top()[i]].iscolorchanged) + ":" + std::to_string(allcells[redocellstack.top()[i]].lineno));
			break;
		}
	}
	redostack.push(redovaluestack);
}

void gGUIGrid::addOrChangeColumnWidth(int columnNo, float w) {
	int windex = -1;
	for(int i = 0; i < gridboxesw.size(); i++) {
		if(columnNo == gridboxesw[i][0]) {
			windex = i;
			break;
		}
	}
	if(windex == -1) gridboxesw.push_back({(float)columnNo, w});
	else gridboxesw[windex][1] = w;
}
void gGUIGrid::addOrChangeRowHeight(int rowNo, float h) {
	int hindex = -1;
	for(int i = 0; i < gridboxesh.size(); i++) {
		if(rowNo == gridboxesh[i][0]) {
			hindex = i;
			break;
		}
	}
	if(hindex == -1) gridboxesh.push_back({(float)rowNo, h});
	else gridboxesh[hindex][1] = h;
}

void gGUIGrid::removeFunction(int functionNum) {
	functionindexes.clear();
	functions.erase(functions.begin() + functionNum);
}

void gGUIGrid::operateFunction(int functionNum) {
	switch(functions[functionNum][FUNCTION_TYPE]) {
	case FUNCTION_COPY:
		allcells[functions[functionNum][FUNCTION_FIRSTINDEX]].showncontent = allcells[functions[functionNum][FUNCTION_SENDER]].showncontent;
		break;
	case FUNCTION_SUM:
		std::string result = std::to_string(makeSum(functions[functionNum][FUNCTION_FIRSTINDEX] % columnnum, functions[functionNum][FUNCTION_FIRSTINDEX] / columnnum, functions[functionNum][functions[functionNum].size() - 1] % columnnum, functions[functionNum][functions[functionNum].size() - 1] / columnnum));
		while(result[result.size() - 1] == '0') result.erase(result.size() - 1, 1);
		if(result[result.size() - 1] == '.') result.erase(result.size() - 1, 1);
		allcells[functions[functionNum][FUNCTION_SENDER]].showncontent = result;
		break;
	}
}

void gGUIGrid::makeDefaultCell() {
	for(int i = 0; i < selectedcells.size(); i++) {
		allcells[selectedcells[i]].iscellselected = false;
		allcells[selectedcells[i]].iscellaligned = false;
		allcells[selectedcells[i]].isbold = false;
		allcells[selectedcells[i]].isitalic = false;
		allcells[selectedcells[i]].iscolorchanged = false;
		allcells[selectedcells[i]].fontnum = gGUIManager::FONT_FREESANS;
		allcells[selectedcells[i]].fontstate = gGUIManager::FONTTYPE_REGULAR;
		allcells[selectedcells[i]].cellalignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
		allcells[selectedcells[i]].lineno = TEXTLINE_NONE;
		allcells[selectedcells[i]].celltype = Cell::TYPE_STRING;
		allcells[selectedcells[i]].textmoveamount = 0;
		allcells[selectedcells[i]].cellcontent = "";
		allcells[selectedcells[i]].showncontent = "";
		allcells[selectedcells[i]].overflowcontent = "";
		allcells[selectedcells[i]].cellfontcolor = fontcolor;
		for(int i = 0; i < functions.size(); i++) {
			if(selectedcells.at(i) == functions[i][FUNCTION_SENDER]) {
				removeFunction(selectedcells.at(i));
				break;
			}
		}
	}
}

void gGUIGrid::changeAllAffectedCellsXW(float diff) {
	for(int i = 0; i < allcells.size(); i++) {
		if(allcells[i].cellcolumnno > currentcolumn) allcells[i].cellx += diff;
		else if(allcells[i].cellcolumnno == currentcolumn) allcells[i].cellw += diff;
	}
	gridw += diff;
}
void gGUIGrid::changeAllAffectedCellsYH(float diff) {
	for(int i = 0; i < allcells.size(); i++) {
		if(allcells[i].cellrowno > currentrow) allcells[i].celly += diff;
		else if(allcells[i].cellrowno == currentrow) allcells[i].cellh += diff;
	}
	gridh += diff;
}

void gGUIGrid::changeSelectedCell(int amount) {
	if(isselected) {
		if(allcells[selectedbox].cellcolumnno == firstselectedcell % columnnum) {
			if(amount == 1 && lastselectedcell % columnnum + 1 < columnnum) lastselectedcell += amount;
			else if(amount == -1 && lastselectedcell % columnnum > firstselectedcell % columnnum) lastselectedcell += amount;
			else if(amount == -1) firstselectedcell += amount;
			else if(amount == columnnum && int(firstselectedcell / columnnum) == allcells[selectedbox].cellrowno && lastselectedcell + columnnum < rownum * columnnum) lastselectedcell += amount;
			else if(amount == columnnum && firstselectedcell + columnnum <= lastselectedcell && lastselectedcell + columnnum < rownum * columnnum) firstselectedcell += amount;
			else if(amount == -columnnum && int(lastselectedcell / columnnum) != allcells[selectedbox].cellrowno && lastselectedcell / columnnum > firstselectedcell / columnnum) lastselectedcell += amount;
			else if(amount == -columnnum && firstselectedcell - columnnum >= firstselectedcell % columnnum) firstselectedcell += amount;
		}
		else if(allcells[selectedbox].cellcolumnno == lastselectedcell % columnnum) {
			if(amount == 1 && lastselectedcell % columnnum + 1 < columnnum) {
				if(firstselectedcell % columnnum < lastselectedcell % columnnum) firstselectedcell += amount;
				else lastselectedcell += amount;
			}
			else if(amount == -1 && firstselectedcell % columnnum > 0) firstselectedcell += amount;
			else if(amount == columnnum && int(firstselectedcell / columnnum) == allcells[selectedbox].cellrowno && lastselectedcell + columnnum < rownum * columnnum) lastselectedcell += amount;
			else if(amount == columnnum && firstselectedcell + columnnum <= lastselectedcell && lastselectedcell + columnnum < rownum * columnnum) firstselectedcell += amount;
			else if(amount == -columnnum && int(lastselectedcell / columnnum) != allcells[selectedbox].cellrowno && lastselectedcell / columnnum > firstselectedcell / columnnum) lastselectedcell += amount;
			else if(amount == -columnnum && firstselectedcell - columnnum >= firstselectedcell % columnnum) firstselectedcell += amount;
		}
	}
	else if(iscolumnselected) {
		if(amount == 1 && lastselectedcell % columnnum + 1 < columnnum) {
			if(allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno == firstselectedcell) lastselectedcell += amount;
			else firstselectedcell += amount;
		}
		else if(amount == 1 && allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno != firstselectedcell) firstselectedcell += amount;
		else if(amount == -1 && lastselectedcell % columnnum > firstselectedcell && allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno == firstselectedcell) lastselectedcell += amount;
		else if(amount == -1 && firstselectedcell > 0) firstselectedcell += amount;
		else if(amount == columnnum && lastselectedcell + columnnum < rownum * columnnum) lastselectedcell += amount;
		else if(amount == -columnnum && lastselectedcell > lastselectedcell % columnnum + firstselectedcell) lastselectedcell += amount;
	}
	else if(isrowselected) {
		if(amount == 1 && lastselectedcell % columnnum + 1 < columnnum) lastselectedcell += amount;
		else if(amount == -1 && lastselectedcell % columnnum > 0) lastselectedcell += amount;
		else if(amount == columnnum && lastselectedcell + columnnum < rownum * columnnum) {
			if(allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno == firstselectedcell) lastselectedcell += amount;
			else firstselectedcell += amount;
		}
		else if(amount == columnnum && allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno != firstselectedcell) firstselectedcell += amount;
		else if(amount == -columnnum && lastselectedcell / columnnum > firstselectedcell / columnnum && allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno == firstselectedcell) lastselectedcell += amount;
		else if(amount == -columnnum && firstselectedcell > 0) firstselectedcell += amount;
	}

	setSelectedCells();
}

void gGUIGrid::changeCell(int cellNo) {
	std::stack<int> removefunctionindexstack;
	for(int function = 0; function < functions.size(); function++) {
		if(cellNo == functions[function][FUNCTION_SENDER] && functions[function][FUNCTION_TYPE] == FUNCTION_SUM) {
			removefunctionindexstack.push(function);
			continue;
		}
		else if(cellNo == functions[function][FUNCTION_FIRSTINDEX] && functions[function][FUNCTION_TYPE] == FUNCTION_COPY) {
			removefunctionindexstack.push(function);
			continue;
		}
	}
	while(!removefunctionindexstack.empty()) {
		removeFunction(removefunctionindexstack.top());
		removefunctionindexstack.pop();
	}
	if(istextboxactive) {
		fillCell(cellNo, textbox.getText());
		textbox.cleanText();
		istextboxactive = false;
	}
	else {
		if(!ctrlzpressed && !ctrlypressed) fillCell(cellNo, allcells[cellNo].cellcontent);
		ctrlvpressed = false;
	}

	for(int function = 0; function < functions.size(); function++) {
		if(selectedbox == functions[function][FUNCTION_SENDER] && functions[function][FUNCTION_TYPE] == FUNCTION_COPY) {
			operateFunction(function);
			continue;
		}
		for(int functionindex = FUNCTION_FIRSTINDEX; functionindex < functions[function].size(); functionindex++) {
			if(selectedbox == functions[function][functionindex]) {
				operateFunction(function);
				break;
			}
		}
	}
}

void gGUIGrid::setSelectedCells() {
	selectedcells.clear();
	int c1 = firstselectedcell % columnnum;
	int c2 = lastselectedcell % columnnum;
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
	int r1 = firstselectedcell / columnnum;
	int r2 = lastselectedcell / columnnum;
	if(r1 > r2) {
		r1 = r1 ^ r2;
		r2 = r1 ^ r2;
		r1 = r1 ^ r2;
	}

	for(int column = c1; column <= c2; column++) {
		for(int row = r1; row <= r2; row++) {
			int index = getCellNo(row, column);
			if(index == -1) {
				createCell(row, column);
				index = allcells.size() - 1;
			}
			selectedcells.push_back(index);
		}
	}
}

void gGUIGrid::resetSelectedIndexes() {
	firstselectedcell = allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno;
	lastselectedcell = firstselectedcell;
	selectedcells.clear();
	selectedcells.push_back(selectedbox);
}

void gGUIGrid::copyCells() {
	copiedcellvalues.clear();
	for(int i = 0; i < selectedcells.size(); i++) {
		std::string tmpstr = std::to_string(allcells[selectedcells[i]].cellcolumnno) + ":" + allcells[selectedcells[i]].cellcontent + ":" + std::to_string(allcells[selectedcells[i]].fontnum) + ":" + std::to_string(allcells[selectedcells[i]].fontstate) + ":" + std::to_string(allcells[selectedcells[i]].cellalignment) + ":" + std::to_string(allcells[selectedcells[i]].cellfontcolor.r) + ":" + std::to_string(allcells[selectedcells[i]].cellfontcolor.g) + ":" + std::to_string(allcells[selectedcells[i]].cellfontcolor.b) + ":" + std::to_string(allcells[selectedcells[i]].iscolorchanged) + ":" + std::to_string(allcells[selectedcells[i]].lineno);
		copiedcellvalues.push_back(tmpstr);
	}
}

void gGUIGrid::pasteCells() {
	int index = selectedbox;
	int row = (allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno) / columnnum;
	int column = (allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno) % columnnum;
	int colon = copiedcellvalues.at(copiedcellvalues.size() - 1).find(':');
	int maxcolumn = column + std::stoi(copiedcellvalues.at(copiedcellvalues.size() - 1).substr(0, colon));
	colon = copiedcellvalues.at(0).find(':');
	maxcolumn -= std::stoi(copiedcellvalues.at(0).substr(0, colon));
	std::vector<std::string> tmpval;

	for(int i = 0; i < copiedcellvalues.size(); i++) {
		tmpval.push_back(copiedcellvalues.at(i));
		tmpval.at(i).erase(0, colon + 1);
		if(copiedcellvalues.size() > i + 1) colon = copiedcellvalues.at(i + 1).find(':');
	}
	for(int i = 0; i < tmpval.size(); i++) {
		int process = PROCESS_TEXT;
		std::string tmpstr = tmpval.at(i);
		while(tmpstr != "") {
			colon = tmpstr.find(':');
			if(colon == std::string::npos) colon = tmpstr.size();
			switch(process) {
			case PROCESS_TEXT:
				allcells[index].cellcontent = tmpstr.substr(0, colon);
				break;
			case PROCESS_FONT:
				allcells[index].fontnum = std::stoi(tmpstr.substr(0, colon));
				break;
			case PROCESS_FONTSTATE:
				allcells[index].fontstate = std::stoi(tmpstr.substr(0, colon));
				break;
			case PROCESS_ALIGNMENT:
				allcells[index].cellalignment = std::stoi(tmpstr.substr(0, colon));
				allcells[index].textmoveamount = 0.5f * std::stoi(tmpstr.substr(0, colon));
				break;
			case PROCESS_LINE:
				allcells[index].lineno = std::stoi(tmpstr.substr(0, colon));
				break;
			case PROCESS_COLOR:
				float r = std::stof(tmpstr.substr(0, colon));
				tmpstr.erase(0, colon + 1);
				colon = tmpstr.find(':');
				float g = std::stof(tmpstr.substr(0, colon));
				tmpstr.erase(0, colon + 1);
				colon = tmpstr.find(':');
				float b = std::stof(tmpstr.substr(0, colon));
				tmpstr.erase(0, colon + 1);
				colon = tmpstr.find(':');
				bool colorchanged = std::stoi(tmpstr.substr(0, colon));
				if(colorchanged) {
					allcells[index].iscolorchanged = true;
					allcells[index].cellfontcolor = gColor(r, g, b);
				}
				else {
					allcells[index].iscolorchanged = false;
					allcells[index].cellfontcolor = fontcolor;
				}
				break;
			}
			tmpstr.erase(0, colon + 1);
			process++;
		}

		fillCell(index, allcells[index].cellcontent);
		column++;
		if(column > maxcolumn) {
			column = (allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno) % columnnum;
			row++;
		}
		index = getCellNo(row, column);
		if(index == -1) {
			createCell(row, column);
			index = allcells.size() - 1;
		}
	}
}

void gGUIGrid::makeUndo() {
	addRedoStack();
	while(!undocellstack.top().empty()) {
		int colon = undostack.top().top().find(':');
		if(undoprocessstack.top() == PROCESS_TEXT) {
			fillCell(undocellstack.top().at(0), undostack.top().top().substr(0, colon));
		}
		else if(undoprocessstack.top() == PROCESS_FONT) {
			setCellFont(std::stoi(undostack.top().top().substr(0, colon)));
		}
		else if(undoprocessstack.top() == PROCESS_FONTSTATE) {
			if(std::stoi(undostack.top().top().substr(0, colon)) - allcells[undocellstack.top()[0]].fontstate == 1 || std::stoi(undostack.top().top().substr(0, colon)) - allcells[undocellstack.top()[0]].fontstate == -1)  {
				setCellFontBold();
			}
			else setCellFontItalic();
		}
		else if(undoprocessstack.top() == PROCESS_ALIGNMENT) {
			setCellAlignment(std::stoi(undostack.top().top().substr(0, colon)), false);
		}
		else if(undoprocessstack.top() == PROCESS_COLOR) {
			float r = std::stof(undostack.top().top().substr(0, colon));
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			float g = std::stof(undostack.top().top().substr(0, colon));
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			float b = std::stof(undostack.top().top().substr(0, colon));
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			bool colorchanged = std::stoi(undostack.top().top().substr(0, colon));
			gColor c = gColor(r, g, b);
			if(colorchanged) setCellFontColor(&c);
			else setCellFontColor();
		}
		else if(undoprocessstack.top() == PROCESS_LINE) {
			setCellLine(std::stoi(undostack.top().top().substr(0, colon)), false);
		}
		else if(undoprocessstack.top() == PROCESS_ALL) {
			fillCell(undocellstack.top().at(0), undostack.top().top().substr(0, colon));
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			setCellFont(std::stoi(undostack.top().top().substr(0, colon)));
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			if(std::stoi(undostack.top().top().substr(0, colon)) - allcells[undocellstack.top()[0]].fontstate == 1 || std::stoi(undostack.top().top().substr(0, colon)) - allcells[undocellstack.top()[0]].fontstate == -1) setCellFontBold();
			else if(std::stoi(undostack.top().top().substr(0, colon)) - allcells[undocellstack.top()[0]].fontstate == 2 || std::stoi(undostack.top().top().substr(0, colon)) - allcells[undocellstack.top()[0]].fontstate == -2) setCellFontItalic();
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			setCellAlignment(std::stoi(undostack.top().top().substr(0, colon)), false);
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			float r = std::stof(undostack.top().top().substr(0, colon));
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			float g = std::stof(undostack.top().top().substr(0, colon));
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			float b = std::stof(undostack.top().top().substr(0, colon));
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			bool colorchanged = std::stoi(undostack.top().top().substr(0, colon));
			gColor c = gColor(r, g, b);
			if(colorchanged) setCellFontColor(&c);
			else setCellFontColor();
			undostack.top().top().erase(0, colon + 1);
			colon = undostack.top().top().find(':');
			setCellLine(std::stoi(undostack.top().top().substr(0, colon)), false);
		}
		changeCell(undocellstack.top().at(0));
		undocellstack.top().pop_front();
		undostack.top().pop();
	}

	undostack.pop();
	undoprocessstack.pop();
	undocellstack.pop();
	ctrlzpressed = false;
}

void gGUIGrid::makeRedo() {
	addUndoStack(redoprocessstack.top());
	while(!redocellstack.top().empty()) {
		int colon = redostack.top().top().find(':');
		if(redoprocessstack.top() == PROCESS_TEXT) {
			fillCell(redocellstack.top().at(0), redostack.top().top().substr(0, colon));
		}
		else if(redoprocessstack.top() == PROCESS_FONT) {
			setCellFont(std::stoi(redostack.top().top().substr(0, colon)));
		}
		else if(redoprocessstack.top() == PROCESS_FONTSTATE) {
			if(std::stoi(redostack.top().top().substr(0, colon)) - allcells[redocellstack.top()[0]].fontstate == 1 || std::stoi(redostack.top().top().substr(0, colon)) - allcells[redocellstack.top()[0]].fontstate == -1)  {
				setCellFontBold();
			}
			else setCellFontItalic();
		}
		else if(redoprocessstack.top() == PROCESS_ALIGNMENT) {
			setCellAlignment(std::stoi(redostack.top().top().substr(0, colon)), false);
		}
		else if(redoprocessstack.top() == PROCESS_COLOR) {
			float r = std::stof(redostack.top().top().substr(0, colon));
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			float g = std::stof(redostack.top().top().substr(0, colon));
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			float b = std::stof(redostack.top().top().substr(0, colon));
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			bool colorchanged = std::stoi(redostack.top().top().substr(0, colon));
			gColor c = gColor(r, g, b);
			if(colorchanged) setCellFontColor(&c);
			else setCellFontColor();
		}
		else if(redoprocessstack.top() == PROCESS_LINE) {
			setCellLine(std::stoi(redostack.top().top().substr(0, colon)), false);
		}
		else if(redoprocessstack.top() == PROCESS_ALL) {
			fillCell(redocellstack.top().at(0), redostack.top().top().substr(0, colon));
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			setCellFont(std::stoi(redostack.top().top().substr(0, colon)));
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			if(std::stoi(redostack.top().top().substr(0, colon)) - allcells[redocellstack.top()[0]].fontstate == 1 || std::stoi(redostack.top().top().substr(0, colon)) - allcells[redocellstack.top()[0]].fontstate == -1) setCellFontBold();
			else if(std::stoi(redostack.top().top().substr(0, colon)) - allcells[redocellstack.top()[0]].fontstate == 2 || std::stoi(redostack.top().top().substr(0, colon)) - allcells[redocellstack.top()[0]].fontstate == -2) setCellFontItalic();
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			setCellAlignment(std::stoi(redostack.top().top().substr(0, colon)), false);
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			float r = std::stof(redostack.top().top().substr(0, colon));
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			float g = std::stof(redostack.top().top().substr(0, colon));
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			float b = std::stof(redostack.top().top().substr(0, colon));
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			bool colorchanged = std::stoi(redostack.top().top().substr(0, colon));
			gColor c = gColor(r, g, b);
			if(colorchanged) setCellFontColor(&c);
			else setCellFontColor();
			redostack.top().top().erase(0, colon + 1);
			colon = redostack.top().top().find(':');
			setCellLine(std::stoi(redostack.top().top().substr(0, colon)), false);
		}
		changeCell(redocellstack.top().at(0));
		redocellstack.top().pop_front();
		redostack.top().pop();
	}

	redostack.pop();
	redoprocessstack.pop();
	redocellstack.pop();
	ctrlypressed = false;
}

void gGUIGrid::createCell(int rowNo, int columnNo) {
	Cell tempcell;
	tempcell.cellx = calculateCurrentX(columnNo) + firstx;
	tempcell.celly = calculateCurrentY(rowNo) + firsty;
	tempcell.cellw = getColumnWidth(columnNo);
	tempcell.cellh = getRowHeight(rowNo);
	tempcell.cellrowno = rowNo;
	tempcell.cellcolumnno = columnNo;
	allcells.push_back(tempcell);
}

void gGUIGrid::createTextBox() {
	//allcells[selectedbox].cellx + 1
	//allcells.at(cellindexcounter).cellx + (allcells.at(cellindexcounter).cellw - font->getStringWidth(allcells.at(cellindexcounter).showncontent)) * textbox.getTextMoveAmount() - textbox.getInitX() * textbox.getTextAlignment() - firstx
	textbox.set(root, this, this, 0, 0, allcells[selectedbox].cellx + 1, allcells[selectedbox].celly - 2, allcells[selectedbox].cellw - 10, allcells[selectedbox].cellh - 2);
	textbox.setTextFont(manager->getFont(allcells[selectedbox].fontnum, allcells[selectedbox].fontstate));
	textbox.setTextAlignment(allcells[selectedbox].cellalignment, allcells[selectedbox].cellw, textbox.getInitX());
	textbox.setTextColor(&allcells[selectedbox].cellfontcolor);
	if(allcells[selectedbox].cellcontent != "") {
		textbox.setText(allcells[selectedbox].cellcontent);
		int length = allcells[selectedbox].cellcontent.length();
		if(allcells[selectedbox].cellalignment == gBaseGUIObject::TEXTALIGNMENT_LEFT || allcells[selectedbox].cellalignment == gBaseGUIObject::TEXTALIGNMENT_RIGHT)
			textbox.setCursorPosX(font->getStringWidth(allcells[selectedbox].cellcontent), length);
		else {
			std::string mid;
			int middle;
			if(length % 2 == 0) middle = length / 2;
			else middle = length / 2 + 1;
			mid = allcells[selectedbox].cellcontent.substr(0, middle);
			textbox.setCursorPosX(font->getStringWidth(mid), middle);
		}
		allcells[selectedbox].showncontent = "";
	}
}

void gGUIGrid::checkCellType(int cellIndex) {
	if(allcells[cellIndex].showncontent == "") return;
	bool isnegative = false;
	bool isfractional = false;
	bool doubledot = false;
	bool isdate = true;
	if(allcells[cellIndex].showncontent.at(0) == '-') isnegative = true;
	for(int i = 1 + isnegative; i < allcells[cellIndex].showncontent.length(); i++) {
		if(allcells[cellIndex].showncontent.at(i) == '.') {
			for(int j = i + 1; j < allcells[cellIndex].showncontent.length(); j++) {
				if(!isdigit(allcells[cellIndex].showncontent.at(j))) {
					isfractional = false;
					doubledot = true;
					break;
				}
				else isfractional = true;
			}
			if(doubledot) break;
		}
	}
	if(allcells[cellIndex].showncontent.length() == 10) {
	    for(int i = 0; i < 10; i++) {
	        if(i == 2 || i == 5) {
	            if(allcells[cellIndex].showncontent[i] != '/' && allcells[cellIndex].showncontent[i] != '.' && allcells[cellIndex].showncontent[i] != '-') {
	            	isdate = false;
	            	break;
	            }
	        }
	        else if(!isdigit(allcells[cellIndex].showncontent[i])) {
	        	isdate = false;
	        	break;
	        }
	    }
	    if(isdate) {
		    int day = stoi(allcells[cellIndex].showncontent.substr(0, 2));
		    int month = stoi(allcells[cellIndex].showncontent.substr(3, 2));
		    int year = stoi(allcells[cellIndex].showncontent.substr(6, 4));

		    if(day < 1 || day > 31 || month < 1 || month > 12 || year < 0) isdate = false;
	    }
	}
	else isdate = false;
	for(int i = 0 + isnegative; i < allcells[cellIndex].showncontent.length(); i++) {
		if(!isdigit(allcells[cellIndex].showncontent.at(i)) && !(isfractional && allcells[cellIndex].showncontent.at(i) == '.') && !isdate) {
			if(allcells[cellIndex].celltype == Cell::TYPE_DIGIT && !allcells[cellIndex].iscellaligned) setCellAlignment(&allcells[cellIndex], gBaseGUIObject::TEXTALIGNMENT_LEFT);
			allcells[cellIndex].celltype = Cell::TYPE_STRING;
			break;
		}
		else allcells[cellIndex].celltype = Cell::TYPE_DIGIT;
	}
	if(allcells[cellIndex].celltype == Cell::TYPE_DIGIT && !allcells[cellIndex].iscellaligned) setCellAlignment(&allcells[cellIndex], gBaseGUIObject::TEXTALIGNMENT_RIGHT);
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
//	gLogi("Grid") << "cellx: "<< allcells[cellindex].cellx
//		<< " celly: " << allcells[cellindex].celly
//		<< " cellrowno: " << allcells[cellindex].cellrowno
//		<< " cellcolumnno: " << allcells[cellindex].cellcolumnno
//		<< " cellcontent: " << allcells[cellindex].cellcontent
//		<< " celltype: " << allcells[cellindex].celltype;
}

void gGUIGrid::drawContent() {
	gColor oldcolor = renderer->getColor();
	drawCellBackground();
	if(isselected || isrowselected || iscolumnselected) drawSelectedArea();
	drawCellContents();
	if(istextboxactive) {
		textbox.setFirstX(firstx);
		textbox.setFirstY(firsty);
		textbox.draw();
	}
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
	for(int i = 1; i <= rownum; i++) {
		if(calculateCurrentY(i) < gridy || calculateCurrentY(i) > boxh) continue;
		if(calculateCurrentY(i) >= gridy && calculateCurrentY(i) <= boxh) {
			std::string rowtitlestring = std::to_string(i);
			renderer->setColor(*fontcolor);
			font->drawText(rowtitlestring, gridx + gridboxw / 4 - font->getStringWidth(rowtitlestring) / 2, calculateCurrentY(i) - getRowHeight(i - 1) / 2 + font->getStringHeight(rowtitlestring) / 2);
			renderer->setColor(*pressedbuttoncolor);
			gDrawLine(gridx - firstx, calculateCurrentY(i), gridx + gridw + gridboxw / 2 - firstx, calculateCurrentY(i));
		}
		temprow++;
	}
}

void gGUIGrid::drawColumnContents() {
	std::string columntitlestring = "";
	for(int i = 0; i < columnnum; i++) {
		if(calculateCurrentX(i + 1) < gridx || calculateCurrentX(i + 1) > boxw) continue;
		if(calculateCurrentX(i + 1) >= gridx && calculateCurrentX(i + 1) <= boxw) {
			if(i / 26 > 26) {
				columntitlestring = (char)(columntitle + i / (26 * 26) - 1);
				columntitlestring += (char)(columntitle + (i / 26 - 1) % 26);
				columntitlestring += (char)(columntitle + i % 26);
			}
			else if(i / 26 > 0) {
				columntitlestring = (char)(columntitle + i / 26 - 1);
				columntitlestring += (char)(columntitle + i % 26);
			}
			else columntitlestring = (char)(columntitle + i);
			renderer->setColor(*fontcolor);
			font->drawText(columntitlestring, calculateCurrentX(i + 1) - getColumnWidth(i) / 2 - (font->getStringWidth(columntitlestring) / 2), gridy + (gridboxh / 2) + (font->getStringHeight(columntitlestring) / 2));
			renderer->setColor(*pressedbuttoncolor);
			gDrawLine(calculateCurrentX(i + 1), gridy - firsty, calculateCurrentX(i + 1), gridy + gridboxh + gridh - firsty);
		}
	}
	gDrawRectangle(gridx, gridy, gridboxw / 2, gridboxh, true);
}

void gGUIGrid::drawTitleLines() {
	renderer->setColor(*backgroundcolor);
	gDrawLine(gridx + (gridboxw / 2) + 1 - firstx, gridy - firsty, gridx + (gridboxw / 2) + 1 - firstx, gridy + gridboxh + gridh - firsty);
	gDrawLine(gridx - firstx, gridy + (gridboxh) - firsty, gridx + gridboxw / 2 + gridw - firstx, gridy + (gridboxh) - firsty);
}

void gGUIGrid::drawCellContents() {
	for(int i = 0; i < allcells.size(); i++) {
		if(allcells[i].cellx + manager->getFont(allcells[i].fontnum, allcells[i].fontstate)->getStringWidth(allcells[i].showncontent) * allcells[i].textmoveamount - textbox.getInitX() * allcells[i].cellalignment < gridx + firstx || allcells[i].cellx + manager->getFont(allcells[i].fontnum, allcells[i].fontstate)->getStringWidth(allcells[i].showncontent) * allcells[i].textmoveamount - textbox.getInitX() * allcells[i].cellalignment > boxw + firstx || allcells[i].celly < gridy + firsty || allcells[i].celly > boxh + firsty) continue;
		if(allcells[i].cellx + manager->getFont(allcells[i].fontnum, allcells[i].fontstate)->getStringWidth(allcells[i].showncontent) * allcells[i].textmoveamount - textbox.getInitX() * allcells[i].cellalignment >= gridx + firstx && allcells[i].cellx + manager->getFont(allcells[i].fontnum, allcells[i].fontstate)->getStringWidth(allcells[i].showncontent) * allcells[i].textmoveamount - textbox.getInitX() * allcells[i].cellalignment <= boxw + firstx && allcells[i].celly >= gridy + firsty && allcells[i].celly <= boxh + firsty) {
			Cell& currentcell = allcells[i];
			if(!currentcell.iscolorchanged) currentcell.cellfontcolor = fontcolor;
			float currentstringwidth;
			float currentstringheight;
			if(currentcell.overflowcontent.empty()) {
				currentstringwidth = manager->getFont(currentcell.fontnum, currentcell.fontstate)->getStringWidth(currentcell.showncontent);
				currentstringheight = manager->getFont(currentcell.fontnum, currentcell.fontstate)->getStringHeight(currentcell.showncontent);
			}
			else {
				currentstringwidth = manager->getFont(currentcell.fontnum, currentcell.fontstate)->getStringWidth(currentcell.overflowcontent);
				currentstringheight = manager->getFont(currentcell.fontnum, currentcell.fontstate)->getStringHeight(currentcell.overflowcontent);
			}
			renderer->setColor(currentcell.cellfontcolor);
			if(currentcell.overflowcontent.empty()) manager->getFont(currentcell.fontnum, currentcell.fontstate)->drawText(currentcell.showncontent, currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - firstx, currentcell.celly + (gridboxh / 2) + (currentstringheight / 2) - firsty);
			else manager->getFont(currentcell.fontnum, currentcell.fontstate)->drawText(currentcell.overflowcontent, currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - firstx, currentcell.celly + (gridboxh / 2) + (currentstringheight / 2) - firsty);
			switch(currentcell.lineno) {
			case TEXTLINE_UNDER:
				gDrawLine(currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - firstx, currentcell.celly + (gridboxh / 2) + currentstringheight - firsty,
						currentcell.cellx + currentstringwidth + ((currentcell.cellw - currentstringwidth) * currentcell.textmoveamount) - firstx, currentcell.celly + (gridboxh / 2) + currentstringheight - firsty);
				break;
			case TEXTLINE_DOUBLEUNDER:
				gDrawLine(currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - firstx, currentcell.celly + (gridboxh / 2) + currentstringheight - firsty,
						currentcell.cellx + currentstringwidth + ((currentcell.cellw - currentstringwidth) * currentcell.textmoveamount) - firstx, currentcell.celly + (gridboxh / 2) + currentstringheight - firsty);
				gDrawLine(currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - firstx, currentcell.celly + (gridboxh / 2) + currentstringheight - firsty + 2,
						currentcell.cellx + currentstringwidth + ((currentcell.cellw - currentstringwidth) * currentcell.textmoveamount) - firstx, currentcell.celly + (gridboxh / 2) + currentstringheight - firsty + 2);
				break;
			case TEXTLINE_STRIKE:
				gDrawLine(currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - firstx, currentcell.celly + currentcell.cellh / 2 + textbox.getInitX() - firsty,
						currentcell.cellx + currentstringwidth + ((currentcell.cellw - currentstringwidth) * currentcell.textmoveamount) - firstx, currentcell.celly + currentcell.cellh / 2 + textbox.getInitX() - firsty);
				break;
			default:
				break;
			}
		}
	}
}

void gGUIGrid::drawSelectedArea() {
	int sx = calculateCurrentX(firstselectedcell % columnnum);
	int sy = calculateCurrentY(int(firstselectedcell / columnnum));
	int sw = calculateCurrentX(lastselectedcell % columnnum) - sx + getColumnWidth(lastselectedcell % columnnum);
	int sh = calculateCurrentY(int(lastselectedcell / columnnum)) - sy + getRowHeight(int(lastselectedcell / columnnum));
	if(appmanager->getGUIManager()->getTheme() == gGUIManager::GUITHEME_DARK) renderer->setColor(selectedareadarkcolor);
	else renderer->setColor(selectedareacolor);
	gDrawRectangle(sx, sy, sw, sh, true);
	renderer->setColor(*textbackgroundcolor);
	gDrawRectangle(allcells[selectedbox].cellx - firstx, allcells[selectedbox].celly - firsty, allcells[selectedbox].cellw, allcells[selectedbox].cellh, true);
	renderer->setColor(selectedframecolor);
	gDrawRectangle(sx + 1, sy + 1, sw - 2, sh - 2, false);
	gDrawRectangle(sx + sw - 2 - 6, sy + sh - 2 - 4, 6, 6, true); // FLAG
}

void gGUIGrid::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	int pressedx = x - left + firstx;
	int pressedy = y - top + firsty - titledy;
	if(cursor == gGUIForm::CURSOR_VRESIZE || cursor == gGUIForm::CURSOR_HRESIZE) {
		firstcursorposx = x;
		firstcursorposy = y;

		previousclicktime = clicktime;
		clicktime = gGetSystemTimeMillis();
		if(clicktime - previousclicktime <= clicktimediff) {
			if(cursor == gGUIForm::CURSOR_VRESIZE) {
				int row = 0;
				while(calculateCurrentY(row) + getRowHeight(row) < pressedy - mousetolerance - firsty) row++;
				int index = getCellNo(row, 0);
				if(index == -1) {
					createCell(row, 0);
					index = allcells.size() - 1;
				}
				int diff = gridboxh - allcells[index].cellh;
				currentrow = row;
				addOrChangeRowHeight(currentrow, gridboxh);
				changeAllAffectedCellsYH(diff);
			}
			else if(cursor == gGUIForm::CURSOR_HRESIZE) {
				int column = 0;
				while(calculateCurrentX(column) + getColumnWidth(column) < pressedx - mousetolerance - firstx) column++;
				int index = getCellNo(0, column);
				if(index == -1) {
					createCell(0, column);
					index = allcells.size() - 1;
				}
				for(int i = 0; i < allcells.size(); i++)
					if(allcells[i].cellcolumnno == column && manager->getFont(allcells[i].fontnum, allcells[i].fontstate)->getStringWidth(allcells[i].showncontent) > manager->getFont(allcells[index].fontnum, allcells[index].fontstate)->getStringWidth(allcells[index].showncontent)) index = i;
				float neww = manager->getFont(allcells[index].fontnum, allcells[index].fontstate)->getStringWidth(allcells[index].showncontent) + textbox.getInitX() + 1;
				if(neww < font->getSize() * 1.8f) neww = font->getSize() * 1.8f;
				int diff = neww - allcells[index].cellw;
				currentcolumn = column;
				addOrChangeColumnWidth(currentcolumn, neww);
				changeAllAffectedCellsXW(diff);
			}
		}
	}
	else if(!(pressedy < gridy + gridboxh + firsty && pressedx < gridx + gridboxw / 2 + firstx) && pressedx >= gridx + firstx && pressedx <= gridx + gridboxw / 2 + gridw + firstx && pressedy >= gridy + firsty && pressedy <= gridy + gridboxh + gridh + firsty) {
		if(pressedx >= gridx + gridboxw / 2 + firstx && pressedx <= gridx + gridboxw / 2 + gridw + firstx && pressedy >= gridy + gridboxh + firsty && pressedy <= gridy + gridboxh + gridh + firsty) {
			isselected = true;
			isrowselected = false;
			iscolumnselected = false;
			int row = 0;
			int column = 0;
			while(calculateCurrentY(row) + getRowHeight(row) < pressedy - firsty) row++;
			while(calculateCurrentX(column) + getColumnWidth(column) < pressedx - firstx) column++;
			int index = getCellNo(row, column);
			if(index == -1) {
				createCell(row, column);
				index = allcells.size() - 1;
			}
			if(istextboxactive) changeCell(selectedbox);
			allcells[selectedbox].iscellselected = false;
			allcells[index].iscellselected = true;
			selectedbox = index;
			resetSelectedIndexes();
		}
		else if(pressedx >= gridx + firstx && pressedx < gridx + (gridboxw / 2) + firstx && pressedy >= gridy + gridboxh + firsty && pressedy <= gridy + gridboxh + gridh + firsty) {
			int row = 0;
			while(calculateCurrentY(row) + getRowHeight(row) < pressedy - firsty) row++;
			int index = getCellNo(row, 0);
			if(index == -1) {
				createCell(row, 0);
				index = allcells.size() - 1;
			}
			if(istextboxactive) changeCell(selectedbox);
			allcells[selectedbox].iscellselected = false;
			allcells[index].iscellselected = true;
			selectedtitle = index;
			selectedbox = index;
			isselected = false;
			isrowselected = true;
			iscolumnselected = false;
			firstselectedcell = row * columnnum;
			lastselectedcell = row * columnnum + columnnum - 1;
			selectedcells.clear();
			setSelectedCells();
		}
		else if(pressedx >= gridx + (gridboxw / 2) + firstx && pressedx < gridx + (gridboxw / 2) + gridw + firstx && pressedy >= gridy + firsty && pressedy <= gridy + gridboxh + firsty) {
			int column = 0;
			while(calculateCurrentX(column) + getColumnWidth(column) < pressedx - firstx) column++;
			int index = getCellNo(0, column);
			if(index == -1) {
				createCell(0, column);
				index = allcells.size() - 1;
			}
			if(istextboxactive) changeCell(selectedbox);
			allcells[selectedbox].iscellselected = false;
			allcells[index].iscellselected = true;
			selectedtitle = index;
			selectedbox = index;
			isselected = false;
			isrowselected = false;
			iscolumnselected = true;
			firstselectedcell = column;
			lastselectedcell = (rownum - 1) * columnnum + column;
			selectedcells.clear();
			setSelectedCells();
		}

		previousclicktime = clicktime;
		clicktime = gGetSystemTimeMillis();
		if(clicktime - previousclicktime <= clicktimediff) {
			isdoubleclicked = true;
//			gLogi("Grid") << "doubleclicked.";
		} else isdoubleclicked = false;
		if(isdoubleclicked) {
			strflag = allcells[selectedbox].cellcontent;
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
	if(cursor == gGUIForm::CURSOR_VRESIZE) {
		int diff = y - firstcursorposy;
		firstcursorposy = y;
		int h = getRowHeight(currentrow);
		if(diff > 0 || h > gridboxh) {
			if(h + diff >= gridboxh) addOrChangeRowHeight(currentrow, h + diff);
			else {
				diff = gridboxh - h;
				addOrChangeRowHeight(currentrow, h + diff);
			}
			changeAllAffectedCellsYH(diff);
		}
	}
	else if(cursor == gGUIForm::CURSOR_HRESIZE) {
		int diff = x - firstcursorposx;
		firstcursorposx = x;
		int w = getColumnWidth(currentcolumn);
		if(diff > 0 || w > gridboxw) {
			if(w + diff >= gridboxw) addOrChangeColumnWidth(currentcolumn, w + diff);
			else {
				diff = gridboxw - w;
				addOrChangeColumnWidth(currentcolumn, w + diff);
			}
			changeAllAffectedCellsXW(diff);
		}
	}
	else if(isselected ||isrowselected || iscolumnselected) {
		int row = 0;
		int column = 0;
		if(!iscolumnselected) while(calculateCurrentY(row) + getRowHeight(row) < y - top - titledy) row++;
		if(!isrowselected) while(calculateCurrentX(column) + getColumnWidth(currentcolumn) < x - left) column++;
		int draggedcell = row * columnnum + ((rownum - 1) * columnnum * iscolumnselected) + column + columnnum * isrowselected;
		int amount;
		if(lastdraggedcell == -1) lastdraggedcell = lastselectedcell;
		amount = draggedcell - lastdraggedcell;
		changeSelectedCell(amount);
		lastdraggedcell = draggedcell;
	}
}

void gGUIGrid::keyPressed(int key){
	if(istextboxactive) textbox.keyPressed(key);
	else if(key == G_KEY_C && ctrlpressed) {
		ctrlcpressed = true;
		copyCells();
		appmanager->setClipboardString(allcells[selectedbox].cellcontent);
	}
	else if(key == G_KEY_V && ctrlpressed && appmanager->getClipboardString() != "") {
		ctrlvpressed = true;
		addUndoStack(PROCESS_ALL);
		pasteCells();
	}
	else if(key == G_KEY_X && ctrlpressed) {
		ctrlcpressed = false;
		addUndoStack(PROCESS_ALL);
		copyCells();
		appmanager->setClipboardString(allcells[selectedbox].cellcontent);
		textbox.cleanText();
		makeDefaultCell();
	}
	else if(key == G_KEY_Z && ctrlpressed) {
		if(!undostack.empty()) {
			ctrlzpressed = true;
			makeUndo();
		}
	}
	else if(key == G_KEY_Y && ctrlpressed) {
		if(!redostack.empty()) {
			ctrlypressed = true;
			makeRedo();
		}
	}
	else if(key == G_KEY_B && ctrlpressed) setCellFontBold();
	else if(key == G_KEY_I && ctrlpressed) setCellFontItalic();
	else if(shiftpressed) {
		if(key == G_KEY_DOWN) changeSelectedCell(columnnum);
		else if(key == G_KEY_UP) changeSelectedCell(-columnnum);
		else if(key == G_KEY_LEFT) changeSelectedCell(-1);
		else if(key == G_KEY_RIGHT) changeSelectedCell(1);
	}
	else if((isselected || isrowselected || iscolumnselected) && (key == G_KEY_LEFT_CONTROL || key == G_KEY_RIGHT_CONTROL)) ctrlpressed = true;
	else if((isselected || isrowselected || iscolumnselected) && (key == G_KEY_LEFT_SHIFT || key == G_KEY_RIGHT_SHIFT)) {
		shiftpressed = true;
		if(isselected && firstselectedcell == -1) {
			firstselectedcell = allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno;
			lastselectedcell = firstselectedcell;
		}
		else if(isrowselected && firstselectedcell == -1) {
			firstselectedcell = allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno;
			lastselectedcell = allcells[selectedbox].cellrowno * columnnum + columnnum - 1;
		}
		else if(iscolumnselected && firstselectedcell == -1) {
			firstselectedcell = allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno;
			lastselectedcell = (rownum - 1) * columnnum + allcells[selectedbox].cellcolumnno;
		}
	}
	else if((isselected || isrowselected || iscolumnselected) && !ctrlpressed && key != G_KEY_ENTER && key != G_KEY_UP && key != G_KEY_DOWN && key != G_KEY_RIGHT && key != G_KEY_LEFT && key != G_KEY_ESC && key != G_KEY_F2) {
		textbox.cleanText();
		strflag = allcells[selectedbox].cellcontent;
		allcells[selectedbox].cellcontent = "";
		allcells[selectedbox].showncontent = "";
		createTextBox();
		textbox.mousePressed(allcells[selectedbox].cellx + textbox.getInitX(), allcells[selectedbox].celly + textbox.getInitX(), 0);
		textbox.keyPressed(key);
		istextboxactive = true;
		isdoubleclicked = true;
	}
}

void gGUIGrid::keyReleased(int key) {
	if(istextboxactive) textbox.keyReleased(key);
	if(key == G_KEY_ENTER && isdoubleclicked && (strflag != allcells[selectedbox].cellcontent || (strflag == "" && textbox.getText() != ""))) addUndoStack(PROCESS_TEXT);
	if((key == G_KEY_ENTER && firstselectedcell != lastselectedcell)) {
		changeCell(selectedbox);
		istextboxactive = false;
		textbox.setEditable(false);
		bool godown = (allcells[selectedbox].cellrowno + 1 <= int(lastselectedcell / columnnum));
		int newrow;
		int newcolumn;
		if(!godown) {
			newrow = int(firstselectedcell / columnnum);
			if(allcells[selectedbox].cellcolumnno < lastselectedcell % columnnum) newcolumn = allcells[selectedbox].cellcolumnno + 1;
			else newcolumn = firstselectedcell % columnnum;
		}
		else {
			newrow = allcells[selectedbox].cellrowno + 1;
			newcolumn = allcells[selectedbox].cellcolumnno;
		}
		int index = getCellNo(newrow, newcolumn);
		if(index == -1) {
			createCell(newrow, newcolumn);
			index = allcells.size() - 1;
		}
		selectedbox = index;
	}
	else if ((key == G_KEY_ENTER || key == G_KEY_DOWN) && !shiftpressed) {
		changeCell(selectedbox);
		istextboxactive = false;
		textbox.setEditable(false);
		isrowselected = false;
		isselected = true;
		bool godown = (allcells[selectedbox].cellrowno + 1 < rownum);
		int newrow;
		int newcolumn;
		if(!godown) {
			newrow = 0;
			if(allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno + 1 < rownum * columnnum) newcolumn = allcells[selectedbox].cellcolumnno + 1;
			else newcolumn = 0;
		}
		else {
			newrow = allcells[selectedbox].cellrowno + 1;
			newcolumn = allcells[selectedbox].cellcolumnno;
		}
		int index = getCellNo(newrow, newcolumn);
		if(index == -1) {
			createCell(newrow, newcolumn);
			index = allcells.size() - 1;
		}
		selectedbox = index;
		resetSelectedIndexes();
	}
	else if(key == G_KEY_UP && !istextboxactive) {
		if(!shiftpressed && allcells[selectedbox].cellrowno - 1 >= 0) {
			isrowselected = false;
			isselected = true;
			int index = getCellNo(allcells[selectedbox].cellrowno - 1, allcells[selectedbox].cellcolumnno);
			if(index == -1) {
				createCell(allcells[selectedbox].cellrowno - 1, allcells[selectedbox].cellcolumnno);
				index = allcells.size() - 1;
			}
			selectedbox = index;
			resetSelectedIndexes();
		}
	}
	else if(key == G_KEY_RIGHT && !istextboxactive) {
		if(!shiftpressed && allcells[selectedbox].cellcolumnno + 1 < columnnum) {
			iscolumnselected = false;
			isselected = true;
			int index = getCellNo(allcells[selectedbox].cellrowno, allcells[selectedbox].cellcolumnno + 1);
			if(index == -1) {
				createCell(allcells[selectedbox].cellrowno, allcells[selectedbox].cellcolumnno + 1);
				index = allcells.size() - 1;
			}
			selectedbox = index;
			resetSelectedIndexes();
		}
	}
	else if(key == G_KEY_LEFT && !istextboxactive) {
		if(!shiftpressed && allcells[selectedbox].cellcolumnno - 1 >= 0) {
			iscolumnselected = false;
			isselected = true;
			int index = getCellNo(allcells[selectedbox].cellrowno, allcells[selectedbox].cellcolumnno - 1);
			if(index == -1) {
				createCell(allcells[selectedbox].cellrowno, allcells[selectedbox].cellcolumnno - 1);
				index = allcells.size() - 1;
			}
			selectedbox = index;
			resetSelectedIndexes();
		}
	}
	else if(key == G_KEY_F2) {
		strflag = allcells[selectedbox].cellcontent;
		textbox.cleanText();
		createTextBox();
		istextboxactive = true;
		textbox.setEditable(true);
	}
	else if(key == G_KEY_LEFT_CONTROL || key == G_KEY_RIGHT_CONTROL) ctrlpressed = false;
	else if(key == G_KEY_LEFT_SHIFT || key == G_KEY_RIGHT_SHIFT) shiftpressed = false;
}

void gGUIGrid::charPressed(unsigned int codepoint) {
	if(istextboxactive) textbox.charPressed(codepoint);
}

void gGUIGrid::mouseScrolled(int x, int y) {
	gGUIScrollable::mouseScrolled(x, y);
}

int gGUIGrid::getCursor(int x, int y) {
	int currentx = x - left + firstx;
	int currenty = y - top + firsty - titledy;
	cursor = gGUIForm::CURSOR_ARROW;
	currentrow = 0;
	currentcolumn = 0;
	if(currentx >= gridx + firstx && currentx < gridx + (gridboxw / 2) + firstx && currenty >= gridy + gridboxh + getRowHeight(currentrow) - mousetolerance - firsty && currenty <= gridy + gridboxh + gridh + mousetolerance - firsty) {
		int currenth = gridy + gridboxh + getRowHeight(currentrow) - mousetolerance;
		while(currentrow + 1 < rownum && currenth + getRowHeight(currentrow + 1) <= currenty) {
			currenth += getRowHeight(currentrow + 1);
			currentrow++;
		}
		if(currenth <= currenty && currenth + mousetolerance * 2 >= currenty) cursor = gGUIForm::CURSOR_VRESIZE;
	}
	else if(currentx >= gridx + (gridboxw / 2) + getColumnWidth(currentcolumn) - mousetolerance - firstx && currentx < gridx + (gridboxw / 2) + gridw + mousetolerance - firstx && currenty >= gridy + firsty && currenty <= gridy + gridboxh + firsty) {
		int currentw = gridx + (gridboxw / 2) + getColumnWidth(currentcolumn) - mousetolerance;
		while(currentcolumn + 1 < columnnum && currentw + getColumnWidth(currentcolumn + 1) <= currentx) {
			currentw += getColumnWidth(currentcolumn + 1);
			currentcolumn++;
		}
		if(currentw <= currentx && currentw + mousetolerance * 2 >= currentx) cursor = gGUIForm::CURSOR_HRESIZE;
	}
	return cursor;
}
