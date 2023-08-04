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
	selectedframecolor = gColor(0.1f, 0.45f, 0.87f, 1.0f);
	selectedareacolor = gColor(0.85f, 0.85f, 0.9f, 1.0f);
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
	if(columnNum > rowNum) {
		for(int row = 0; row < rownum; row++) {
			gridboxesh.push_back(gridboxh);
			gridboxesw.push_back(gridboxw);
		}
		for(int column = rownum; column < columnnum; column++) gridboxesw.push_back(gridboxw);
	}
	else {
		for(int column = 0; column < columnnum; column++) {
			gridboxesh.push_back(gridboxh);
			gridboxesw.push_back(gridboxw);
		}
		for(int row = columnnum; row < rownum; row++) gridboxesh.push_back(gridboxh);
	}
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

void gGUIGrid::createCell(int rowNo, int columnNo) {
	Cell tempcell;
	tempcell.cellx = calculateCurrentX(columnNo) + firstx;
	tempcell.celly = calculateCurrentY(rowNo) + firsty;
	tempcell.cellw = gridboxesw[columnNo];
	tempcell.cellh = gridboxesh[rowNo];
	tempcell.cellrowno = rowNo;
	tempcell.cellcolumnno = columnNo;
	allcells.push_back(tempcell);
}

void gGUIGrid::createTextBox() {
	//allcells.at(selectedbox).cellx + 1
	//allcells.at(cellindexcounter).cellx + (allcells.at(cellindexcounter).cellw - font->getStringWidth(allcells.at(cellindexcounter).showncontent)) * textbox.getTextMoveAmount() - textbox.getInitX() * textbox.getTextAlignment() - firstx
	textbox.set(root, this, this, 0, 0, allcells.at(selectedbox).cellx + 1, allcells.at(selectedbox).celly - 2, allcells.at(selectedbox).cellw - 10, allcells.at(selectedbox).cellh - 2);
	textbox.setTextFont(manager->getFont(allcells.at(selectedbox).fontnum, allcells.at(selectedbox).fontstate));
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
	if(allcells.at(cellIndex).showncontent == "") return;
	bool isnegative = false;
	bool isfractional = false;
	bool doubledot = false;
	if(allcells.at(cellIndex).showncontent.at(0) == '-') isnegative = true;
	for(int i = 1 + isnegative; i < allcells.at(cellIndex).showncontent.length(); i++) {
		if(allcells.at(cellIndex).showncontent.at(i) == '.') {
			for(int j = i + 1; j < allcells.at(cellIndex).showncontent.length(); j++) {
				if(!isdigit(allcells.at(cellIndex).showncontent.at(j))) {
					isfractional = false;
					doubledot = true;
					break;
				}
				else isfractional = true;
			}
			if(doubledot) break;
		}
	}
	for(int i = 0 + isnegative; i < allcells.at(cellIndex).showncontent.length(); i++) {
		if(!isdigit(allcells.at(cellIndex).showncontent.at(i)) && !(isfractional && allcells.at(cellIndex).showncontent.at(i) == '.')) {
			if(allcells.at(cellIndex).celltype == Cell::TYPE_DIGIT && !allcells.at(cellIndex).iscellaligned) setCellAlignment(gBaseGUIObject::TEXTALIGNMENT_LEFT, false);
			allcells.at(cellIndex).celltype = Cell::TYPE_STRING;
			break;
		}
		else allcells.at(cellIndex).celltype = Cell::TYPE_DIGIT;
	}
	if(allcells.at(cellIndex).celltype == Cell::TYPE_DIGIT && !allcells.at(cellIndex).iscellaligned) setCellAlignment(gBaseGUIObject::TEXTALIGNMENT_RIGHT, false);
}

void gGUIGrid::resetSelectedIndexes() {
	firstselectedcell = allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno;
	lastselectedcell = firstselectedcell;
	selectedcells.clear();
	selectedcells.push_back(selectedbox);
}

void gGUIGrid::setCellFont(int fontNum) {
	if(allcells.empty()) return;
	if(!ctrlzpressed) {
		addUndoStack(PROCESS_FONT);
		for(int i = 0; i < selectedcells.size(); i++) {
			allcells.at(selectedcells.at(i)).fontnum = fontNum;
			allcells.at(selectedcells.at(i)).fontstate = gGUIManager::FONTTYPE_REGULAR;
			if(allcells.at(selectedcells.at(i)).isbold) allcells.at(selectedcells.at(i)).fontstate += gGUIManager::FONTTYPE_BOLD;
			if(allcells.at(selectedcells.at(i)).isitalic) allcells.at(selectedcells.at(i)).fontstate += gGUIManager::FONTTYPE_ITALIC;
		}
	}
	else {
		if(!ctrlypressed) {
			allcells.at(undocellstack.top().at(0)).fontnum = fontNum;
			allcells.at(undocellstack.top().at(0)).fontstate = gGUIManager::FONTTYPE_REGULAR;
			if(allcells.at(undocellstack.top().at(0)).isbold) allcells.at(undocellstack.top().at(0)).fontstate += gGUIManager::FONTTYPE_BOLD;
			if(allcells.at(undocellstack.top().at(0)).isitalic) allcells.at(undocellstack.top().at(0)).fontstate += gGUIManager::FONTTYPE_ITALIC;
		}
		else {
			allcells.at(redocellstack.top().at(0)).fontnum = fontNum;
			allcells.at(redocellstack.top().at(0)).fontstate = gGUIManager::FONTTYPE_REGULAR;
			if(allcells.at(redocellstack.top().at(0)).isbold) allcells.at(redocellstack.top().at(0)).fontstate += gGUIManager::FONTTYPE_BOLD;
			if(allcells.at(redocellstack.top().at(0)).isitalic) allcells.at(redocellstack.top().at(0)).fontstate += gGUIManager::FONTTYPE_ITALIC;
		}
	}
	textbox.setTextFont(manager->getFont(allcells.at(selectedbox).fontnum, allcells.at(selectedbox).fontstate));
}

void gGUIGrid::setCellFontBold() {
	if(allcells.empty()) return;
	if(!ctrlzpressed) {
		addUndoStack(PROCESS_FONTSTATE);
		for(int i = 0; i < selectedcells.size(); i++) {
			if(allcells.at(selectedcells.at(i)).isbold) {
				allcells.at(selectedcells.at(i)).isbold = false;
				allcells.at(selectedcells.at(i)).fontstate -= gGUIManager::FONTTYPE_BOLD;
			}
			else {
				allcells.at(selectedcells.at(i)).isbold = true;
				allcells.at(selectedcells.at(i)).fontstate += gGUIManager::FONTTYPE_BOLD;
			}
		}
	}
	else {
		if(!ctrlypressed) {
			if(allcells.at(undocellstack.top().at(0)).isbold) {
				allcells.at(undocellstack.top().at(0)).isbold = false;
				allcells.at(undocellstack.top().at(0)).fontstate -= gGUIManager::FONTTYPE_BOLD;
			}
			else {
				allcells.at(undocellstack.top().at(0)).isbold = true;
				allcells.at(undocellstack.top().at(0)).fontstate += gGUIManager::FONTTYPE_BOLD;
			}
		}
		else {
			if(allcells.at(redocellstack.top().at(0)).isbold) {
				allcells.at(redocellstack.top().at(0)).isbold = false;
				allcells.at(redocellstack.top().at(0)).fontstate -= gGUIManager::FONTTYPE_BOLD;
			}
			else {
				allcells.at(redocellstack.top().at(0)).isbold = true;
				allcells.at(redocellstack.top().at(0)).fontstate += gGUIManager::FONTTYPE_BOLD;
			}
		}
	}
	textbox.setTextFont(manager->getFont(allcells.at(selectedbox).fontnum, allcells.at(selectedbox).fontstate));
}

void gGUIGrid::setCellFontItalic() {
	if(allcells.empty()) return;
	if(!ctrlzpressed) {
		addUndoStack(PROCESS_FONTSTATE);
		for(int i = 0; i < selectedcells.size(); i++) {
			if(allcells.at(selectedcells.at(i)).isitalic) {
				allcells.at(selectedcells.at(i)).isitalic = false;
				allcells.at(selectedcells.at(i)).fontstate -= gGUIManager::FONTTYPE_ITALIC;
			}
			else {
				allcells.at(selectedcells.at(i)).isitalic = true;
				allcells.at(selectedcells.at(i)).fontstate += gGUIManager::FONTTYPE_ITALIC;
			}
		}
	}
	else {
		if(!ctrlypressed) {
			if(allcells.at(undocellstack.top().at(0)).isitalic) {
				allcells.at(undocellstack.top().at(0)).isitalic = false;
				allcells.at(undocellstack.top().at(0)).fontstate -= gGUIManager::FONTTYPE_ITALIC;
			}
			else {
				allcells.at(undocellstack.top().at(0)).isitalic = true;
				allcells.at(undocellstack.top().at(0)).fontstate += gGUIManager::FONTTYPE_ITALIC;
			}
		}
		else {
			if(allcells.at(redocellstack.top().at(0)).isitalic) {
				allcells.at(redocellstack.top().at(0)).isitalic = false;
				allcells.at(redocellstack.top().at(0)).fontstate -= gGUIManager::FONTTYPE_ITALIC;
			}
			else {
				allcells.at(redocellstack.top().at(0)).isitalic = true;
				allcells.at(redocellstack.top().at(0)).fontstate += gGUIManager::FONTTYPE_ITALIC;
			}
		}
	}
}

void gGUIGrid::setCellAlignment(int cellAlignment, bool clicked) {
	if(allcells.empty()) return;
	if(clicked) {
		addUndoStack(PROCESS_ALIGNMENT);
		for(int i = 0; i < selectedcells.size(); i++) {
			if(clicked && !allcells.at(selectedcells.at(i)).iscellaligned) allcells.at(selectedcells.at(i)).iscellaligned = true;
			else if(clicked && allcells.at(selectedcells.at(i)).iscellaligned && allcells.at(selectedcells.at(i)).celltype == Cell::TYPE_DIGIT) {
				allcells.at(selectedcells.at(i)).iscellaligned = false;
				cellAlignment = gBaseGUIObject::TEXTALIGNMENT_RIGHT;
			}
			else if(clicked && allcells.at(selectedcells.at(i)).cellalignment == cellAlignment) {
				allcells.at(selectedcells.at(i)).iscellaligned = false;
				cellAlignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
			}
			allcells.at(selectedcells.at(i)).cellalignment = cellAlignment;
			allcells.at(selectedcells.at(i)).textmoveamount = 0.5f * cellAlignment;
		}
	}
	else {
		if(!ctrlzpressed) {
			allcells.at(selectedbox).cellalignment = cellAlignment;
			allcells.at(selectedbox).textmoveamount = 0.5f * cellAlignment;
		}
		else {
			if(!ctrlypressed) {
				allcells.at(undocellstack.top().at(0)).cellalignment = cellAlignment;
				allcells.at(undocellstack.top().at(0)).textmoveamount = 0.5f * cellAlignment;
			}
			else {
				allcells.at(redocellstack.top().at(0)).cellalignment = cellAlignment;
				allcells.at(redocellstack.top().at(0)).textmoveamount = 0.5f * cellAlignment;
			}
		}
	}
	textbox.setTextAlignment(cellAlignment, allcells.at(selectedbox).cellw, textbox.getInitX());
}

void gGUIGrid::setCellFontColor(gColor *fontColor) {
	if(allcells.empty()) return;
	if(!ctrlzpressed) {
		addUndoStack(PROCESS_COLOR);
		for(int i = 0; i < selectedcells.size(); i++) {
			allcells.at(selectedcells.at(i)).cellfontcolor = fontColor;
		}
	}
	else {
		if(!ctrlypressed) allcells.at(undocellstack.top().at(0)).cellfontcolor = fontColor;
		else allcells.at(redocellstack.top().at(0)).cellfontcolor = fontColor;
	}
	textbox.setTextColor(fontColor);
}

void gGUIGrid::setCellLine(int lineNo, bool clicked) {
	if(allcells.empty()) return;
	if(!ctrlzpressed) {
		addUndoStack(PROCESS_LINE);
		for(int i = 0; i < selectedcells.size(); i++) {
			if(clicked && lineNo == allcells.at(selectedcells.at(i)).lineno) allcells.at(selectedcells.at(i)).lineno = TEXTLINE_NONE;
			else allcells.at(selectedcells.at(i)).lineno = lineNo;
		}
	}
	else {
		if(!ctrlypressed) allcells.at(undocellstack.top().at(0)).lineno = lineNo;
		else allcells.at(redocellstack.top().at(0)).lineno = lineNo;
	}
}

void gGUIGrid::setSelectedFrameColor(gColor* selectedFrameColor) {
	selectedframecolor = *selectedFrameColor;
}

void gGUIGrid::setSelectedAreaColor(gColor* selectedAreaColor) {
	selectedareacolor = *selectedAreaColor;
}

int gGUIGrid::getCell(int rowNo, int columnNo) {
	for(int i = 0; i < allcells.size(); i++)
		if(allcells.at(i).cellrowno == rowNo && allcells.at(i).cellcolumnno == columnNo) return i;
	return -1;
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
							int copiedindex = getCell(std::stoi(row), columnindex);
							if(copiedindex == -1) {
								createCell(std::stoi(row) - 1, columnindex);
								copiedindex = allcells.size() - 1;
							}
							tempstr = allcells.at(copiedindex).showncontent;
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
						int copiedindex = getCell(std::stoi(tstr) - 1, int(tempstr[0]) % 65);
						if(copiedindex == -1) {
							createCell(std::stoi(tstr) - 1, int(tempstr[0]) % 65);
							copiedindex = allcells.size() - 1;
						}
						tempstr = allcells.at(copiedindex).showncontent;
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

int gGUIGrid::getNearestFilledCell(int index) {
	int nearestindex = -1;
	for(int i = 0; i < allcells.size(); i++) {
		if(i != index && allcells.at(i).celly == allcells.at(index).celly && !allcells.at(i).showncontent.empty()) {
			if(allcells.at(index).cellalignment != gBaseGUIObject::TEXTALIGNMENT_RIGHT && allcells.at(index).cellx + manager->getFont(allcells.at(index).fontnum)->getStringWidth(allcells.at(index).showncontent) > allcells.at(i).cellx && allcells.at(index).cellx < allcells.at(i).cellx) {
				if(nearestindex != -1 && allcells.at(nearestindex).cellx > allcells.at(i).cellx) nearestindex = i;
				else if(nearestindex == -1) nearestindex = i;
			}
			else if(allcells.at(index).cellalignment == gBaseGUIObject::TEXTALIGNMENT_RIGHT && allcells.at(index).cellx - (manager->getFont(allcells.at(index).fontnum)->getStringWidth(allcells.at(index).showncontent) - allcells.at(index).cellw) < allcells.at(i).cellx + allcells.at(i).cellw && allcells.at(index).cellx > allcells.at(i).cellx) {
				if(nearestindex != -1 && allcells.at(nearestindex).cellx < allcells.at(i).cellx) nearestindex = i;
				else if(nearestindex == -1) nearestindex = i;
			}
		}
	}
	return nearestindex;
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
			int currentindex = getCell(row, column);
			if(currentindex == -1) {
				createCell(row, column);
				currentindex = allcells.size() - 1;
			}
			functionindexes.push_back(currentindex);
			if(allcells.at(currentindex).celltype == Cell::TYPE_DIGIT) {
				result += std::stof(allcells.at(currentindex).showncontent);
			}
		}
	}
	return result;
}

float gGUIGrid::calculateCurrentX(int columnNum) {
	float currentx = gridx + gridboxw / 2 - firstx;
	for(int column = 0; column < columnNum; column++) currentx += gridboxesw[column];
	return currentx;
}

float gGUIGrid::calculateCurrentY(int rowNum) {
	float currenty = gridy + gridboxh - firsty;
	for(int row = 0; row < rowNum; row++) currenty += gridboxesh[row];
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
				undovaluestack.push(allcells.at(undocellstack.top().at(i)).cellcontent);
				break;
			case PROCESS_FONT:
				undovaluestack.push(std::to_string(allcells.at(undocellstack.top().at(i)).fontnum));
				break;
			case PROCESS_FONTSTATE:
				undovaluestack.push(std::to_string(allcells.at(undocellstack.top().at(i)).fontstate));
				break;
			case PROCESS_ALIGNMENT:
				undovaluestack.push(std::to_string(allcells.at(undocellstack.top().at(i)).cellalignment));
				break;
			case PROCESS_COLOR:
				undovaluestack.push(std::to_string(allcells.at(undocellstack.top().at(i)).cellfontcolor.r) + ":" + std::to_string(allcells.at(undocellstack.top().at(i)).cellfontcolor.g) + ":" + std::to_string(allcells.at(undocellstack.top().at(i)).cellfontcolor.b));
				break;
			case PROCESS_LINE:
				undovaluestack.push(std::to_string(allcells.at(undocellstack.top().at(i)).lineno));
				break;
			case PROCESS_ALL:
				undovaluestack.push(allcells.at(undocellstack.top().at(i)).cellcontent + ":" + std::to_string(allcells.at(undocellstack.top().at(i)).fontnum) + ":" + std::to_string(allcells.at(undocellstack.top().at(i)).fontstate) + ":" + std::to_string(allcells.at(undocellstack.top().at(i)).cellalignment) + ":" + std::to_string(allcells.at(undocellstack.top().at(i)).cellfontcolor.r) + ":" + std::to_string(allcells.at(undocellstack.top().at(i)).cellfontcolor.g) + ":" + std::to_string(allcells.at(undocellstack.top().at(i)).cellfontcolor.b) + ":" + std::to_string(allcells.at(undocellstack.top().at(i)).lineno));
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
			undovaluestack.push(allcells.at(selectedbox).cellcontent);
			break;
		case PROCESS_FONT:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				undovaluestack.push(std::to_string(allcells.at(selectedcells.at(i)).fontnum));
			}
			break;
		case PROCESS_FONTSTATE:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				undovaluestack.push(std::to_string(allcells.at(selectedcells.at(i)).fontstate));
			}
			break;
		case PROCESS_ALIGNMENT:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				undovaluestack.push(std::to_string(allcells.at(selectedcells.at(i)).cellalignment));
			}
			break;
		case PROCESS_COLOR:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				std::string tmpstr = std::to_string(allcells.at(selectedcells.at(i)).cellfontcolor.r) + ":" + std::to_string(allcells.at(selectedcells.at(i)).cellfontcolor.g) + ":" + std::to_string(allcells.at(selectedcells.at(i)).cellfontcolor.b);
				undovaluestack.push(tmpstr);
			}
			break;
		case PROCESS_LINE:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				undovaluestack.push(std::to_string(allcells.at(selectedcells.at(i)).lineno));;
			}
			break;
		case PROCESS_ALL:
			if(!ctrlvpressed) {
				undocellstack.push(selectedcells);
				std::reverse(undocellstack.top().begin(), undocellstack.top().end());
				for(int i = 0; i < selectedcells.size(); i++) {
					int amount = selectedcells.at(i) - selectedbox;
					std::string tmpstr = allcells.at(selectedbox + amount).cellcontent + ":" + std::to_string(allcells.at(selectedbox + amount).fontnum) + ":" + std::to_string(allcells.at(selectedbox + amount).fontstate) + ":" + std::to_string(allcells.at(selectedbox + amount).cellalignment) + ":" + std::to_string(allcells.at(selectedbox + amount).cellfontcolor.r) + ":" + std::to_string(allcells.at(selectedbox + amount).cellfontcolor.g) + ":" + std::to_string(allcells.at(selectedbox + amount).cellfontcolor.b) + ":" + std::to_string(allcells.at(selectedbox + amount).lineno);
					undovaluestack.push(tmpstr);
				}
			}
			else {
				int index = selectedbox;
				int row = (allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno) / columnnum;
				int column = (allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno) % columnnum;
				int colon = copiedcellvalues.at(copiedcellvalues.size() - 1).find(':');
				int maxcolumn = column + std::stoi(copiedcellvalues.at(copiedcellvalues.size() - 1).substr(0, colon));
				colon = copiedcellvalues.at(0).find(':');
				maxcolumn -= std::stoi(copiedcellvalues.at(0).substr(0, colon));
				std::deque<int> indexes;
				for(int i = 0; i < copiedcellvalues.size(); i++) {
					if(column > maxcolumn) {
						column = (allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno) % columnnum;
						row++;
					}
					index = getCell(row, column);
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
					std::string tmpstr = allcells.at(selectedbox + amount).cellcontent + ":" + std::to_string(allcells.at(selectedbox + amount).fontnum) + ":" + std::to_string(allcells.at(selectedbox + amount).fontstate) + ":" + std::to_string(allcells.at(selectedbox + amount).cellalignment) + ":" + std::to_string(allcells.at(selectedbox + amount).cellfontcolor.r) + ":" + std::to_string(allcells.at(selectedbox + amount).cellfontcolor.g) + ":" + std::to_string(allcells.at(selectedbox + amount).cellfontcolor.b) + ":" + std::to_string(allcells.at(selectedbox + amount).lineno);
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
			redovaluestack.push(allcells.at(redocellstack.top().at(i)).cellcontent);
			break;
		case PROCESS_FONT:
			redovaluestack.push(std::to_string(allcells.at(redocellstack.top().at(i)).fontnum));
			break;
		case PROCESS_FONTSTATE:
			redovaluestack.push(std::to_string(allcells.at(redocellstack.top().at(i)).fontstate));
			break;
		case PROCESS_ALIGNMENT:
			redovaluestack.push(std::to_string(allcells.at(redocellstack.top().at(i)).cellalignment));
			break;
		case PROCESS_COLOR:
			redovaluestack.push(std::to_string(allcells.at(redocellstack.top().at(i)).cellfontcolor.r) + ":" + std::to_string(allcells.at(redocellstack.top().at(i)).cellfontcolor.g) + ":" + std::to_string(allcells.at(redocellstack.top().at(i)).cellfontcolor.b));
			break;
		case PROCESS_LINE:
			redovaluestack.push(std::to_string(allcells.at(redocellstack.top().at(i)).lineno));
			break;
		case PROCESS_ALL:
			redovaluestack.push(allcells.at(redocellstack.top().at(i)).cellcontent + ":" + std::to_string(allcells.at(redocellstack.top().at(i)).fontnum) + ":" + std::to_string(allcells.at(redocellstack.top().at(i)).fontstate) + ":" + std::to_string(allcells.at(redocellstack.top().at(i)).cellalignment) + ":" + std::to_string(allcells.at(redocellstack.top().at(i)).cellfontcolor.r) + ":" + std::to_string(allcells.at(redocellstack.top().at(i)).cellfontcolor.g) + ":" + std::to_string(allcells.at(redocellstack.top().at(i)).cellfontcolor.b) + ":" + std::to_string(allcells.at(redocellstack.top().at(i)).lineno));
			break;
		}
	}
	redostack.push(redovaluestack);
}

void gGUIGrid::removeFunction(int functionNum) {
	functionindexes.clear();
	functions.erase(functions.begin() + functionNum);
}

void gGUIGrid::operateFunction(int functionNum) {
	switch(functions[functionNum][FUNCTION_TYPE]) {
	case FUNCTION_COPY:
		allcells.at(functions[functionNum][FUNCTION_FIRSTINDEX]).showncontent = allcells.at(functions[functionNum][FUNCTION_SENDER]).showncontent;
		break;
	case FUNCTION_SUM:
		std::string result = std::to_string(makeSum(functions[functionNum][FUNCTION_FIRSTINDEX] % columnnum, functions[functionNum][FUNCTION_FIRSTINDEX] / columnnum, functions[functionNum][functions[functionNum].size() - 1] % columnnum, functions[functionNum][functions[functionNum].size() - 1] / columnnum));
		while(result[result.size() - 1] == '0') result.erase(result.size() - 1, 1);
		if(result[result.size() - 1] == '.') result.erase(result.size() - 1, 1);
		allcells.at(functions[functionNum][FUNCTION_SENDER]).showncontent = result;
		break;
	}
}

void gGUIGrid::makeDefaultCell() {
	for(int i = 0; i < selectedcells.size(); i++) {
		allcells.at(selectedcells.at(i)).iscellselected = false;
		allcells.at(selectedcells.at(i)).iscellaligned = false;
		allcells.at(selectedcells.at(i)).fontnum = gGUIManager::FONTTYPE_REGULAR;
		allcells.at(selectedcells.at(i)).cellalignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
		allcells.at(selectedcells.at(i)).lineno = TEXTLINE_NONE;
		allcells.at(selectedcells.at(i)).cellh = 30.0f;
		allcells.at(selectedcells.at(i)).cellw = 80.0f;
		allcells.at(selectedcells.at(i)).textmoveamount = 0;
		allcells.at(selectedcells.at(i)).cellcontent = "";
		allcells.at(selectedcells.at(i)).showncontent = "";
		allcells.at(selectedcells.at(i)).overflowcontent = "";
		allcells.at(selectedcells.at(i)).celltype = Cell::TYPE_STRING;
		allcells.at(selectedcells.at(i)).cellfontcolor = fontcolor;
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
		if(allcells.at(i).cellcolumnno > currentcolumn) allcells.at(i).cellx += diff;
		else if(allcells.at(i).cellcolumnno == currentcolumn) allcells.at(i).cellw = gridboxesw[currentcolumn];
	}
	gridw += diff;
}
void gGUIGrid::changeAllAffectedCellsYH(float diff) {
	for(int i = 0; i < allcells.size(); i++) {
		if(allcells.at(i).cellrowno > currentrow) allcells.at(i).celly += diff;
		else if(allcells.at(i).cellrowno == currentrow) allcells.at(i).cellh = gridboxesh[currentrow];
	}
	gridh += diff;
}

void gGUIGrid::changeSelectedCell(int amount) {
	if(isselected) {
		if(allcells.at(selectedbox).cellcolumnno == firstselectedcell % columnnum) {
			if(amount == 1 && lastselectedcell % columnnum + 1 < columnnum) lastselectedcell += amount;
			else if(amount == -1 && lastselectedcell % columnnum > firstselectedcell % columnnum) lastselectedcell += amount;
			else if(amount == -1) firstselectedcell += amount;
			else if(amount == columnnum && int(firstselectedcell / columnnum) == allcells.at(selectedbox).cellrowno && lastselectedcell + columnnum < rownum * columnnum) lastselectedcell += amount;
			else if(amount == columnnum && firstselectedcell + columnnum <= lastselectedcell && lastselectedcell + columnnum < rownum * columnnum) firstselectedcell += amount;
			else if(amount == -columnnum && int(lastselectedcell / columnnum) != allcells.at(selectedbox).cellrowno && lastselectedcell / columnnum > firstselectedcell / columnnum) lastselectedcell += amount;
			else if(amount == -columnnum && firstselectedcell - columnnum >= firstselectedcell % columnnum) firstselectedcell += amount;
		}
		else if(allcells.at(selectedbox).cellcolumnno == lastselectedcell % columnnum) {
			if(amount == 1 && lastselectedcell % columnnum + 1 < columnnum) {
				if(firstselectedcell % columnnum < lastselectedcell % columnnum) firstselectedcell += amount;
				else lastselectedcell += amount;
			}
			else if(amount == -1 && firstselectedcell % columnnum > 0) firstselectedcell += amount;
			else if(amount == columnnum && int(firstselectedcell / columnnum) == allcells.at(selectedbox).cellrowno && lastselectedcell + columnnum < rownum * columnnum) lastselectedcell += amount;
			else if(amount == columnnum && firstselectedcell + columnnum <= lastselectedcell && lastselectedcell + columnnum < rownum * columnnum) firstselectedcell += amount;
			else if(amount == -columnnum && int(lastselectedcell / columnnum) != allcells.at(selectedbox).cellrowno && lastselectedcell / columnnum > firstselectedcell / columnnum) lastselectedcell += amount;
			else if(amount == -columnnum && firstselectedcell - columnnum >= firstselectedcell % columnnum) firstselectedcell += amount;
		}
	}
	else if(iscolumnselected) {
		if(amount == 1 && lastselectedcell % columnnum + 1 < columnnum) {
			if(allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno == firstselectedcell) lastselectedcell += amount;
			else firstselectedcell += amount;
		}
		else if(amount == 1 && allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno != firstselectedcell) firstselectedcell += amount;
		else if(amount == -1 && lastselectedcell % columnnum > firstselectedcell && allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno == firstselectedcell) lastselectedcell += amount;
		else if(amount == -1 && firstselectedcell > 0) firstselectedcell += amount;
		else if(amount == columnnum && lastselectedcell + columnnum < rownum * columnnum) lastselectedcell += amount;
		else if(amount == -columnnum && lastselectedcell > lastselectedcell % columnnum + firstselectedcell) lastselectedcell += amount;
	}
	else if(isrowselected) {
		if(amount == 1 && lastselectedcell % columnnum + 1 < columnnum) lastselectedcell += amount;
		else if(amount == -1 && lastselectedcell % columnnum > 0) lastselectedcell += amount;
		else if(amount == columnnum && lastselectedcell + columnnum < rownum * columnnum) {
			if(allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno == firstselectedcell) lastselectedcell += amount;
			else firstselectedcell += amount;
		}
		else if(amount == columnnum && allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno != firstselectedcell) firstselectedcell += amount;
		else if(amount == -columnnum && lastselectedcell / columnnum > firstselectedcell / columnnum && allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno == firstselectedcell) lastselectedcell += amount;
		else if(amount == -columnnum && firstselectedcell > 0) firstselectedcell += amount;
	}

	setSelectedCells();
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
			int index = getCell(row, column);
			if(index == -1) {
				createCell(row, column);
				index = allcells.size() - 1;
			}
			selectedcells.push_back(index);
		}
	}
}

void gGUIGrid::copyCells() {
	copiedcellvalues.clear();
	for(int i = 0; i < selectedcells.size(); i++) {
		std::string tmpstr = std::to_string(allcells.at(selectedcells.at(i)).cellcolumnno) + ":" + allcells.at(selectedcells.at(i)).cellcontent + ":" + std::to_string(allcells.at(selectedcells.at(i)).fontnum) + ":" + std::to_string(allcells.at(selectedcells.at(i)).fontstate) + ":" + std::to_string(allcells.at(selectedcells.at(i)).cellalignment) + ":" + std::to_string(allcells.at(selectedcells.at(i)).cellfontcolor.r) + ":" + std::to_string(allcells.at(selectedcells.at(i)).cellfontcolor.g) + ":" + std::to_string(allcells.at(selectedcells.at(i)).cellfontcolor.b) + ":" + std::to_string(allcells.at(selectedcells.at(i)).lineno);
		copiedcellvalues.push_back(tmpstr);
	}
}

void gGUIGrid::pasteCells() {
	int index = selectedbox;
	int row = (allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno) / columnnum;
	int column = (allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno) % columnnum;
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
				allcells.at(index).cellcontent = tmpstr.substr(0, colon);
				break;
			case PROCESS_FONT:
				allcells.at(index).fontnum = std::stoi(tmpstr.substr(0, colon));
				break;
			case PROCESS_FONTSTATE:
				allcells.at(index).fontstate = std::stoi(tmpstr.substr(0, colon));
				break;
			case PROCESS_ALIGNMENT:
				allcells.at(index).cellalignment = std::stoi(tmpstr.substr(0, colon));
				allcells.at(index).textmoveamount = 0.5f * std::stoi(tmpstr.substr(0, colon));
				break;
			case PROCESS_LINE:
				allcells.at(index).lineno = std::stoi(tmpstr.substr(0, colon));
				break;
			case PROCESS_COLOR:
				float r = std::stof(tmpstr.substr(0, colon));
				tmpstr.erase(0, colon + 1);
				colon = tmpstr.find(':');
				float g = std::stof(tmpstr.substr(0, colon));
				tmpstr.erase(0, colon + 1);
				colon = tmpstr.find(':');
				float b = std::stof(tmpstr.substr(0, colon));
				allcells.at(index).cellfontcolor = gColor(r, g ,b);
				break;
			}
			tmpstr.erase(0, colon + 1);
			process++;
		}

		fillCell(index, allcells.at(index).cellcontent);
		column++;
		if(column > maxcolumn) {
			column = (allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno) % columnnum;
			row++;
		}
		index = getCell(row, column);
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
			if(std::stoi(undostack.top().top().substr(0, colon)) - allcells.at(undocellstack.top().at(0)).fontstate == 1 || std::stoi(undostack.top().top().substr(0, colon)) - allcells.at(undocellstack.top().at(0)).fontstate == -1)  {
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
			gColor c = gColor(r, g, b);
			setCellFontColor(&c);
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
			if(std::stoi(undostack.top().top().substr(0, colon)) - allcells.at(undocellstack.top().at(0)).fontstate == 1 || std::stoi(undostack.top().top().substr(0, colon)) - allcells.at(undocellstack.top().at(0)).fontstate == -1) setCellFontBold();
			else if(std::stoi(undostack.top().top().substr(0, colon)) - allcells.at(undocellstack.top().at(0)).fontstate == 2 || std::stoi(undostack.top().top().substr(0, colon)) - allcells.at(undocellstack.top().at(0)).fontstate == -2) setCellFontItalic();
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
			gColor c = gColor(r, g, b);
			setCellFontColor(&c);
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
			if(std::stoi(redostack.top().top().substr(0, colon)) - allcells.at(redocellstack.top().at(0)).fontstate == 1 || std::stoi(redostack.top().top().substr(0, colon)) - allcells.at(redocellstack.top().at(0)).fontstate == -1)  {
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
			gColor c = gColor(r, g, b);
			setCellFontColor(&c);
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
			if(std::stoi(redostack.top().top().substr(0, colon)) - allcells.at(redocellstack.top().at(0)).fontstate == 1 || std::stoi(redostack.top().top().substr(0, colon)) - allcells.at(redocellstack.top().at(0)).fontstate == -1) setCellFontBold();
			else if(std::stoi(redostack.top().top().substr(0, colon)) - allcells.at(redocellstack.top().at(0)).fontstate == 2 || std::stoi(redostack.top().top().substr(0, colon)) - allcells.at(redocellstack.top().at(0)).fontstate == -2) setCellFontItalic();
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
			gColor c = gColor(r, g, b);
			setCellFontColor(&c);
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

void gGUIGrid::fillCell(int cellNo, std::string tempstr) { //when rowNo = 1, columnNO = 4; tempstr = "happyyyy";
	if(cellNo > rownum * columnnum - 1) return;
	int cellindex = cellNo;
	if(strflag != tempstr && !ctrlzpressed && !ctrlypressed && !ctrlvpressed) {
//		undocellstack.push(allcells.at(cellindex));
//		undocellstack.top().cellcontent = strflag;
//		undocellstack.top().showncontent = fixTextFunction(strflag, cellindex);
	}
	allcells.at(cellindex).cellcontent = tempstr;
	allcells.at(cellindex).showncontent = fixTextFunction(tempstr, cellindex);

	checkCellType(cellindex);

	int nearestindex = -1;
	for(int i = 0; i < allcells.size(); i++) {
		if(allcells.at(i).celly == allcells.at(cellindex).celly) {
			nearestindex = getNearestFilledCell(i);
			if(nearestindex != -1) allcells.at(i).overflowcontent = fixOverflowText(allcells.at(i), allcells.at(nearestindex));
			else allcells.at(i).overflowcontent = "";
		}
	}
	if(allcells.at(cellindex).cellw < manager->getFont(allcells.at(cellindex).fontnum)->getStringWidth(allcells.at(cellindex).showncontent)) {
		nearestindex = getNearestFilledCell(cellindex);
		if(nearestindex != -1) allcells.at(cellindex).overflowcontent = fixOverflowText(allcells.at(cellindex), allcells.at(nearestindex));
		else allcells.at(cellindex).overflowcontent = "";
	}
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
		if(!ctrlzpressed && !ctrlypressed) fillCell(cellNo, allcells.at(cellNo).cellcontent);
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
			font->drawText(rowtitlestring, gridx + gridboxw / 4 - font->getStringWidth(rowtitlestring) / 2, calculateCurrentY(i) - gridboxesh[i - 1] / 2 + font->getStringHeight(rowtitlestring) / 2);
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
			font->drawText(columntitlestring, calculateCurrentX(i + 1) - gridboxesw[i] / 2 - (font->getStringWidth(columntitlestring) / 2), gridy + (gridboxh / 2) + (font->getStringHeight(columntitlestring) / 2));
			renderer->setColor(*pressedbuttoncolor);
			gDrawLine(calculateCurrentX(i + 1), gridy - firsty, calculateCurrentX(i + 1), gridy + gridboxh + gridh - firsty);
		}
	}
	Cell emptycell;
	gDrawRectangle(gridx, gridy, emptycell.cellw / 2, emptycell.cellh, true);
}

void gGUIGrid::drawTitleLines() {
	renderer->setColor(*backgroundcolor);
	gDrawLine(gridx + (gridboxw / 2) + 1 - firstx, gridy - firsty, gridx + (gridboxw / 2) + 1 - firstx, gridy + gridboxh + gridh - firsty);
	gDrawLine(gridx - firstx, gridy + (gridboxh) - firsty, gridx + gridboxw / 2 + gridw - firstx, gridy + (gridboxh) - firsty);
}

void gGUIGrid::drawCellContents() {
	for(int i = 0; i < allcells.size(); i++) {
		Cell& currentcell = allcells.at(i);
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
		if(currentcell.cellx + currentstringwidth * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment < gridx + firstx || currentcell.cellx + currentstringwidth * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment > boxw + firstx || currentcell.celly < gridy + firsty || currentcell.celly > boxh + firsty) continue;
		if(currentcell.cellx + currentstringwidth * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment >= gridx + firstx && currentcell.cellx + currentstringwidth * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment <= boxw + firstx && currentcell.celly >= gridy + firsty && currentcell.celly <= boxh + firsty) {
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
	int sw = calculateCurrentX(lastselectedcell % columnnum) - sx + gridboxesw[lastselectedcell % columnnum];
	int sh = calculateCurrentY(int(lastselectedcell / columnnum)) - sy + gridboxesh[int(lastselectedcell / columnnum)];
	renderer->setColor(selectedareacolor);
	gDrawRectangle(sx, sy, sw, sh, true);
	renderer->setColor(*textbackgroundcolor);
	gDrawRectangle(allcells.at(selectedbox).cellx - firstx, allcells.at(selectedbox).celly - firsty, allcells.at(selectedbox).cellw, allcells.at(selectedbox).cellh, true);
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
				while(calculateCurrentY(row) + gridboxesh[row] < pressedy - mousetolerance - firsty) row++;
				int index = getCell(row, 0);
				if(index != -1 && !allcells.at(index).showncontent.empty()) {
					int diff = gridboxh - allcells.at(index).cellh;
					currentrow = row;
					gridboxesh[currentrow] = gridboxh;
					changeAllAffectedCellsYH(diff);
				}
			}
		}
		else if(cursor == gGUIForm::CURSOR_HRESIZE) {
			int column = 0;
			while(calculateCurrentX(column) + gridboxesw[column] < pressedx - mousetolerance - firstx) column++;
			int index = getCell(0, column);
			if(index != -1 && !allcells.at(index).showncontent.empty()) {
				gFont* tmpfont = manager->getFont(allcells.at(index).fontnum, allcells.at(index).fontstate);
				for(int i = 0; i < allcells.size(); i++)
					if(allcells.at(i).cellcolumnno == column && tmpfont->getStringWidth(allcells.at(i).showncontent) > tmpfont->getStringWidth(allcells.at(index).showncontent)) index = i;
				float neww = tmpfont->getStringWidth(allcells.at(index).showncontent) + textbox.getInitX() + 1;
				if(neww < font->getSize() * 1.8f) neww = font->getSize() * 1.8f;
				int diff = neww - allcells.at(index).cellw;
				currentcolumn = column;
				gridboxesw[currentcolumn] = neww;
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
			while(calculateCurrentY(row) + gridboxesh[row] < pressedy - firsty) row++;
			while(calculateCurrentX(column) + gridboxesw[column] < pressedx - firstx) column++;
			int index = getCell(row, column);
			if(index == -1) {
				createCell(row, column);
				index = allcells.size() - 1;
			}
			if(istextboxactive) changeCell(selectedbox);
			allcells.at(selectedbox).iscellselected = false;
			allcells.at(index).iscellselected = true;
			selectedbox = index;
			resetSelectedIndexes();
		}
		else if(pressedx >= gridx + firstx && pressedx < gridx + (gridboxw / 2) + firstx && pressedy >= gridy + gridboxh + firsty && pressedy <= gridy + gridboxh + gridh + firsty) {
			int row = 0;
			while(calculateCurrentY(row) + gridboxesh[row] < pressedy - firsty) row++;
			int index = getCell(row, 0);
			if(index == -1) {
				createCell(row, 0);
				index = allcells.size() - 1;
			}
			if(istextboxactive) changeCell(selectedbox);
			allcells.at(selectedbox).iscellselected = false;
			allcells.at(index).iscellselected = true;
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
			while(calculateCurrentX(column) + gridboxesw[column] < pressedx - firstx) column++;
			int index = getCell(0, column);
			if(index == -1) {
				createCell(0, column);
				index = allcells.size() - 1;
			}
			if(istextboxactive) changeCell(selectedbox);
			allcells.at(selectedbox).iscellselected = false;
			allcells.at(index).iscellselected = true;
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
			strflag = allcells.at(selectedbox).cellcontent;
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
		if(diff > 0 || gridboxesh[currentrow] > gridboxh) {
			if(gridboxesh[currentrow] + diff >= gridboxh) gridboxesh[currentrow] += diff;
			else {
				diff = gridboxh - gridboxesh[currentrow];
				gridboxesh[currentrow] += diff;
			}
			changeAllAffectedCellsYH(diff);
		}
	}
	else if(cursor == gGUIForm::CURSOR_HRESIZE) {
		int diff = x - firstcursorposx;
		firstcursorposx = x;
		if(diff > 0 || gridboxesw[currentcolumn] > gridboxw) {
			if(gridboxesw[currentcolumn] + diff >= gridboxh) gridboxesw[currentcolumn] += diff;
			else {
				diff = gridboxw - gridboxesw[currentcolumn];
				gridboxesw[currentcolumn] += diff;
			}
			changeAllAffectedCellsXW(diff);
		}
	}
	else if(isselected ||isrowselected || iscolumnselected) {
		int row = 0;
		int column = 0;
		if(!iscolumnselected) while(calculateCurrentY(row) + gridboxesh[row] < y - top - titledy) row++;
		if(!isrowselected) while(calculateCurrentX(column) + gridboxesw[column] < x - left) column++;
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
		appmanager->setClipboardString(allcells.at(selectedbox).cellcontent);
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
		appmanager->setClipboardString(allcells.at(selectedbox).cellcontent);
		textbox.cleanText();
		makeDefaultCell();
	}
	else if(key == G_KEY_Z && ctrlpressed) {
		ctrlzpressed = true;
		if(!undostack.empty()) makeUndo();
	}
	else if(key == G_KEY_Y && ctrlpressed) {
		ctrlypressed = true;
		if(!redostack.empty()) makeRedo();
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
			firstselectedcell = allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno;
			lastselectedcell = firstselectedcell;
		}
		else if(isrowselected && firstselectedcell == -1) {
			firstselectedcell = allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno;
			lastselectedcell = allcells.at(selectedbox).cellrowno * columnnum + columnnum - 1;
		}
		else if(iscolumnselected && firstselectedcell == -1) {
			firstselectedcell = allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno;
			lastselectedcell = (rownum - 1) * columnnum + allcells.at(selectedbox).cellcolumnno;
		}
	}
	else if((isselected || isrowselected || iscolumnselected) && !ctrlpressed && key != G_KEY_ENTER && key != G_KEY_UP && key != G_KEY_DOWN && key != G_KEY_RIGHT && key != G_KEY_LEFT && key != G_KEY_ESC && key != G_KEY_F2) {
		if(!isdoubleclicked) addUndoStack(PROCESS_TEXT);
		textbox.cleanText();
		strflag = allcells.at(selectedbox).cellcontent;
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
	if(key == G_KEY_ENTER && isdoubleclicked) addUndoStack(PROCESS_TEXT);
	if((key == G_KEY_ENTER && firstselectedcell != lastselectedcell)) {
		changeCell(selectedbox);
		istextboxactive = false;
		textbox.setEditable(false);
		bool godown = (allcells.at(selectedbox).cellrowno + 1 <= int(lastselectedcell / columnnum));
		int newrow;
		int newcolumn;
		if(!godown) {
			newrow = int(firstselectedcell / columnnum);
			if(allcells.at(selectedbox).cellcolumnno < lastselectedcell % columnnum) newcolumn = allcells.at(selectedbox).cellcolumnno + 1;
			else newcolumn = firstselectedcell % columnnum;
		}
		else {
			newrow = allcells.at(selectedbox).cellrowno + 1;
			newcolumn = allcells.at(selectedbox).cellcolumnno;
		}
		int index = getCell(newrow, newcolumn);
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
		bool godown = (allcells.at(selectedbox).cellrowno + 1 < rownum);
		int newrow;
		int newcolumn;
		if(!godown) {
			newrow = 0;
			if(allcells.at(selectedbox).cellrowno * columnnum + allcells.at(selectedbox).cellcolumnno + 1 < rownum * columnnum) newcolumn = allcells.at(selectedbox).cellcolumnno + 1;
			else newcolumn = 0;
		}
		else {
			newrow = allcells.at(selectedbox).cellrowno + 1;
			newcolumn = allcells.at(selectedbox).cellcolumnno;
		}
		int index = getCell(newrow, newcolumn);
		if(index == -1) {
			createCell(newrow, newcolumn);
			index = allcells.size() - 1;
		}
		selectedbox = index;
		resetSelectedIndexes();
	}
	else if(key == G_KEY_UP && !istextboxactive) {
		if(!shiftpressed && allcells.at(selectedbox).cellrowno - 1 >= 0) {
			isrowselected = false;
			isselected = true;
			int index = getCell(allcells.at(selectedbox).cellrowno - 1, allcells.at(selectedbox).cellcolumnno);
			if(index == -1) {
				createCell(allcells.at(selectedbox).cellrowno - 1, allcells.at(selectedbox).cellcolumnno);
				index = allcells.size() - 1;
			}
			selectedbox = index;
			resetSelectedIndexes();
		}
	}
	else if(key == G_KEY_RIGHT && !istextboxactive) {
		if(!shiftpressed && allcells.at(selectedbox).cellcolumnno + 1 < columnnum) {
			iscolumnselected = false;
			isselected = true;
			int index = getCell(allcells.at(selectedbox).cellrowno, allcells.at(selectedbox).cellcolumnno + 1);
			if(index == -1) {
				createCell(allcells.at(selectedbox).cellrowno, allcells.at(selectedbox).cellcolumnno + 1);
				index = allcells.size() - 1;
			}
			selectedbox = index;
			resetSelectedIndexes();
		}
	}
	else if(key == G_KEY_LEFT && !istextboxactive) {
		if(!shiftpressed && allcells.at(selectedbox).cellcolumnno - 1 >= 0) {
			iscolumnselected = false;
			isselected = true;
			int index = getCell(allcells.at(selectedbox).cellrowno, allcells.at(selectedbox).cellcolumnno - 1);
			if(index == -1) {
				createCell(allcells.at(selectedbox).cellrowno, allcells.at(selectedbox).cellcolumnno - 1);
				index = allcells.size() - 1;
			}
			selectedbox = index;
			resetSelectedIndexes();
		}
	}
	else if(key == G_KEY_F2) {
		strflag = allcells.at(selectedbox).cellcontent;
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
	if(currentx >= gridx + firstx && currentx < gridx + (gridboxw / 2) + firstx && currenty >= gridy + gridboxh + gridboxesh[currentrow] - mousetolerance - firsty && currenty <= gridy + gridboxh + gridh + mousetolerance - firsty) {
		int currenth = gridy + gridboxh + gridboxesh[currentrow] - mousetolerance;
		while(currentrow + 1 < rownum && currenth + gridboxesh[currentrow + 1] <= currenty) {
			currenth += gridboxesh[currentrow + 1];
			currentrow++;
		}
		if(currenth <= currenty && currenth + mousetolerance * 2 >= currenty) cursor = gGUIForm::CURSOR_VRESIZE;
	}
	else if(currentx >= gridx + (gridboxw / 2) + gridboxesw[currentcolumn] - mousetolerance - firstx && currentx < gridx + (gridboxw / 2) + gridw + mousetolerance - firstx && currenty >= gridy + firsty && currenty <= gridy + gridboxh + firsty) {
		int currentw = gridx + (gridboxw / 2) + gridboxesw[currentcolumn] - mousetolerance;
		while(currentcolumn + 1 < columnnum && currentw + gridboxesw[currentcolumn + 1] <= currentx) {
			currentw += gridboxesw[currentcolumn + 1];
			currentcolumn++;
		}
		if(currentw <= currentx && currentw + mousetolerance * 2 >= currentx) cursor = gGUIForm::CURSOR_HRESIZE;
	}
	return cursor;
}
