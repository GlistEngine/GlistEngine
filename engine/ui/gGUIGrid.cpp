/*
 * gGUIGrid.cpp
 *
 *  Created on: 5 Aug 2022
 *      Author: murat
 */

#include "gGUIGrid.h"
#include "gBaseApp.h"

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
	gridboxwhalf = gridboxw / 2;
	gridx = 0.0f;
	gridy = 0.0f;
	gridw = gridboxw * columnnum;
	gridh = gridboxh * rownum;
	clicktimediff = 250;
	clicktime = gGetSystemTimeMillis();
	previousclicktime = clicktime - 2 * clicktimediff;
	firstclicktime = previousclicktime - 2 * clicktimediff;
	strflag = "";
	totalw = 0;
	totalh = 0;
	enableScrollbars(true, false);
	Cell tempcell;
	istextboxactive = false;
	updateTotalSize();
}

gGUIGrid::~gGUIGrid() {

}

void gGUIGrid::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	setDimensions(w, h);
	textbox.set(root, this, this, 0, 0, gridx + gridboxwhalf + 1, gridy + gridboxh - 5, gridboxw - 6, gridboxh);
	textbox.setSize(gridboxw - 6, gridboxh - 2);
	textbox.enableBackground(false);
	manager = root->getAppManager()->getGUIManager();
	updateTotalSize();
//	gLogi("Grid") << "Textbox:" << textbox.left << " " << textbox.top << " " << textbox.right << " " << textbox.bottom;
}

void gGUIGrid::setGrid(int rowNum, int columnNum) {
	rownum = std::min(rowNum, maxrownum);
	columnnum = std::min(columnNum, maxcolumnnum);
	updateTotalSize();
}

void gGUIGrid::setRowNum(int rowNum) {
	rownum = rowNum;
}

int gGUIGrid::getRowNum() {
	return rownum;
}

void gGUIGrid::setColumnNum(int columnNum) {
	columnnum = columnNum;
}

int gGUIGrid::getColumnNum() {
	return columnnum;
}

void gGUIGrid::update() {
	textbox.update();
}

void gGUIGrid::setCellFont(int fontNo) {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		setSelectedCells(true);
		addUndoStack(PROCESS_FONT);
		for(int i = 0; i < selectedcells.size(); i++) {
			allcells[selectedcells[i]].fontnum = fontNo;
			allcells[selectedcells[i]].fontstate = gGUIManager::FONTTYPE_REGULAR;
			if(allcells[selectedcells[i]].isbold) allcells[selectedcells[i]].fontstate += gGUIManager::FONTTYPE_BOLD;
			if(allcells[selectedcells[i]].isitalic) allcells[selectedcells[i]].fontstate += gGUIManager::FONTTYPE_ITALIC;
		}
	}
	else {
		if(!ctrlypressed) {
			allcells[undocellstack.top()[0]].fontnum = fontNo;
			allcells[undocellstack.top()[0]].fontstate = gGUIManager::FONTTYPE_REGULAR;
			if(allcells[undocellstack.top()[0]].isbold) allcells[undocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_BOLD;
			if(allcells[undocellstack.top()[0]].isitalic) allcells[undocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_ITALIC;
		}
		else {
			allcells[redocellstack.top()[0]].fontnum = fontNo;
			allcells[redocellstack.top()[0]].fontstate = gGUIManager::FONTTYPE_REGULAR;
			if(allcells[redocellstack.top()[0]].isbold) allcells[redocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_BOLD;
			if(allcells[redocellstack.top()[0]].isitalic) allcells[redocellstack.top()[0]].fontstate += gGUIManager::FONTTYPE_ITALIC;
		}
	}
	textbox.setTextFont(manager->getFont(allcells[selectedbox].fontnum, allcells[selectedbox].fontstate/*, allcells[selectedbox].fontsize*/));
}

void gGUIGrid::setCellFont(Cell* cell, int fontNo) {
	cell->fontnum = fontNo;
	cell->fontstate = gGUIManager::FONTTYPE_REGULAR;
	if(cell->isbold) cell->fontstate += gGUIManager::FONTTYPE_BOLD;
	if(cell->isitalic) cell->fontstate += gGUIManager::FONTTYPE_ITALIC;
}

void gGUIGrid::setCellFont(const std::string& cell, int fontNo) {
	Cell* c = getCell(cell);
	setCellFont(c, fontNo);
}

void gGUIGrid::setCellsFont(std::deque<Cell*> cells, int fontNo) {
	for(int i = 0; i < cells.size(); i++) setCellFont(cells[i], fontNo);
}

void gGUIGrid::setCellsFont(Cell* cell1, Cell* cell2, int fontNo) {
	int c1 = cell1->cellcolumnno;
	int c2 = cell2->cellcolumnno;
	int r1 = cell1->cellrowno;
	int r2 = cell2->cellrowno;
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
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
			Cell* c = &allcells[index];
			setCellFont(c, fontNo);
		}
	}
}

void gGUIGrid::setCellsFont(const std::string& cell1, const std::string& cell2, int fontNo) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	setCellsFont(c1, c2, fontNo);
}

void gGUIGrid::setCellFontBold() {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		setSelectedCells(true);
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
	textbox.setTextFont(manager->getFont(allcells[selectedbox].fontnum, allcells[selectedbox].fontstate/*, allcells[selectedbox].fontsize*/));
}

void gGUIGrid::setCellFontBold(Cell* cell) {
	if(cell->isbold) {
		cell->isbold = false;
		cell->fontstate -= gGUIManager::FONTTYPE_BOLD;
	}
	else {
		cell->isbold = true;
		cell->fontstate += gGUIManager::FONTTYPE_BOLD;
	}
}

void gGUIGrid::setCellFontBold(const std::string& cell) {
	Cell* c = getCell(cell);
	setCellFontBold(c);
}

void gGUIGrid::setCellsFontBold(std::deque<Cell*> cells) {
	for(int i = 0; i < cells.size(); i++) setCellFontBold(cells[i]);
}

void gGUIGrid::setCellsFontBold(Cell* cell1, Cell* cell2) {
	int c1 = cell1->cellcolumnno;
	int c2 = cell2->cellcolumnno;
	int r1 = cell1->cellrowno;
	int r2 = cell2->cellrowno;
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
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
			Cell* c = &allcells[index];
			setCellFontBold(c);
		}
	}
}

void gGUIGrid::setCellsFontBold(const std::string& cell1, const std::string& cell2) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	setCellsFontBold(c1, c2);
}

void gGUIGrid::setCellFontItalic() {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		setSelectedCells(true);
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

void gGUIGrid::setCellFontItalic(Cell* cell) {
	if(cell->isitalic) {
		cell->isitalic = false;
		cell->fontstate -= gGUIManager::FONTTYPE_ITALIC;
	}
	else {
		cell->isitalic = true;
		cell->fontstate += gGUIManager::FONTTYPE_ITALIC;
	}
}

void gGUIGrid::setCellFontItalic(const std::string& cell) {
	Cell* c = getCell(cell);
	setCellFontItalic(c);
}

void gGUIGrid::setCellsFontItalic(std::deque<Cell*> cells) {
	for(int i = 0; i < cells.size(); i++) setCellFontItalic(cells[i]);
}

void gGUIGrid::setCellsFontItalic(Cell* cell1, Cell* cell2) {
	int c1 = cell1->cellcolumnno;
	int c2 = cell2->cellcolumnno;
	int r1 = cell1->cellrowno;
	int r2 = cell2->cellrowno;
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
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
			Cell* c = &allcells[index];
			setCellFontItalic(c);
		}
	}
}

void gGUIGrid::setCellsFontItalic(const std::string& cell1, const std::string& cell2) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	setCellsFontItalic(c1, c2);
}

void gGUIGrid::setCellFontSize(int fontSize) {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		setSelectedCells(true);
		addUndoStack(PROCESS_FONTSIZE);
		for(int i = 0; i < selectedcells.size(); i++) {
			allcells[selectedcells[i]].fontsize = fontSize;
		}
	}
	else {
		if(!ctrlypressed) allcells[undocellstack.top()[0]].fontsize = fontSize;
		else allcells[redocellstack.top()[0]].fontsize = fontSize;
	}
}

void gGUIGrid::setCellFontSize(Cell* cell, int fontSize) {
	cell->fontsize = fontSize;
}

void gGUIGrid::setCellFontSize(const std::string& cell, int fontSize) {
	Cell* c = getCell(cell);
	setCellFontSize(c, fontSize);

}

void gGUIGrid::setCellsFontSize(std::deque<Cell*> cells, int fontSize) {
	for(int i = 0; i < cells.size(); i++) setCellFontSize(cells[i], fontSize);
}

void gGUIGrid::setCellsFontSize(Cell* cell1, Cell* cell2, int fontSize) {
	int c1 = cell1->cellcolumnno;
	int c2 = cell2->cellcolumnno;
	int r1 = cell1->cellrowno;
	int r2 = cell2->cellrowno;
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
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
			Cell* c = &allcells[index];
			setCellFontSize(c, fontSize);
		}
	}
}

void gGUIGrid::setCellsFontSize(const std::string& cell1, const std::string& cell2, int fontSize) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	setCellsFontSize(c1, c2, fontSize);
}

void gGUIGrid::setCellFontColor(gColor *fontColor) {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		setSelectedCells(true);
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

void gGUIGrid::setCellFontColor(Cell* cell, gColor* fontColor) {
	cell->cellfontcolor = fontColor;
	if(fontColor != fontcolor) cell->iscolorchanged = true;
	else cell->iscolorchanged = false;
}

void gGUIGrid::setCellFontColor(const std::string& cell, gColor* fontColor) {
	Cell* c = getCell(cell);
	setCellFontColor(c, fontColor);
}

void gGUIGrid::setCellsFontColor(std::deque<Cell*> cells, gColor* fontColor) {
	for(int i = 0; i < cells.size(); i++) setCellFontColor(cells[i], fontColor);
}

void gGUIGrid::setCellsFontColor(Cell* cell1, Cell* cell2, gColor* fontColor) {
	int c1 = cell1->cellcolumnno;
	int c2 = cell2->cellcolumnno;
	int r1 = cell1->cellrowno;
	int r2 = cell2->cellrowno;
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
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
			Cell* c = &allcells[index];
			setCellFontColor(c, fontColor);
		}
	}
}

void gGUIGrid::setCellsFontColor(const std::string& cell1, const std::string& cell2, gColor* fontColor) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	setCellsFontColor(c1, c2, fontColor);
}

void gGUIGrid::setCellLine(int lineNo, bool clicked) {
	if(allcells.empty()) return;
	if(!ctrlzpressed && !ctrlypressed) {
		setSelectedCells(true);
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
void gGUIGrid::setCellLine(Cell* cell, int lineNo, bool clicked) {
	if(clicked && lineNo == cell->lineno) cell->lineno = TEXTLINE_NONE;
	else cell->lineno = lineNo;
}

void gGUIGrid::setCellLine(const std::string& cell, int lineNo, bool clicked) {
	Cell* c = getCell(cell);
	setCellLine(c, lineNo, clicked);
}

void gGUIGrid::setCellsLine(std::deque<Cell*> cells, int lineNo, bool clicked) {
	for(int i = 0; i < cells.size(); i++) setCellLine(cells[i], lineNo, clicked);
}

void gGUIGrid::setCellsLine(Cell* cell1, Cell* cell2, int lineNo, bool clicked) {
	int c1 = cell1->cellcolumnno;
	int c2 = cell2->cellcolumnno;
	int r1 = cell1->cellrowno;
	int r2 = cell2->cellrowno;
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
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
			Cell* c = &allcells[index];
			setCellLine(c, lineNo, clicked);
		}
	}
}

void gGUIGrid::setCellsLine(const std::string& cell1, const std::string& cell2, int lineNo, bool clicked) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	setCellsLine(c1, c2, lineNo, clicked);
}

void gGUIGrid::setCellContent(gGUIGrid::Cell* cell, const std::string& cellContent) {
	cell->cellcontent = cellContent;
	uint64_t hash = hashCell(cell->cellrowno, cell->cellcolumnno);
	auto it = cellmap.find(hash);
	if (it == cellmap.end()) {
		return;
	}
	int cellno = it->second;
	changeCell(cellno);
}

void gGUIGrid::setCellContent(int rowNo, int columnNo, const std::string& cellContent) {
	setCellContent(getCell(rowNo, columnNo), cellContent);
}

void gGUIGrid::setCellContent(const std::string& cell, const std::string& cellContent) {
	Cell* c = getCell(cell);
	setCellContent(c, cellContent);
}

void gGUIGrid::setCellsContent(std::deque<Cell*> cells, std::vector<std::string> contents) {
	if(cells.size() <= contents.size())
		for(int i = 0; i < cells.size(); i++) setCellContent(cells[i], contents[i]);
	else
		for(int i = 0; i < contents.size(); i++) setCellContent(cells[i], contents[i]);
}

void gGUIGrid::setCellsContent(Cell* cell1, Cell* cell2, std::vector<std::string> contents) {
	int c1 = cell1->cellcolumnno;
	int c2 = cell2->cellcolumnno;
	int r1 = cell1->cellrowno;
	int r2 = cell2->cellrowno;
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
	if(r1 > r2) {
		r1 = r1 ^ r2;
		r2 = r1 ^ r2;
		r1 = r1 ^ r2;
	}
	int i = 0;
	int cellsize = (r2 - r1 + 1) * (c2 - c1 + 1);
	for(int column = c1; column <= c2; column++) {
		for(int row = r1; row <= r2; row++) {
			if(i + 1 > contents.size()) break;
			int index = getCellNo(row, column);
			if(index == -1) {
				createCell(row, column);
				index = allcells.size() - 1;
			}
			Cell* c = &allcells[index];
			setCellContent(c, contents[i]);
			i++;
		}
	}
}

void gGUIGrid::setCellsContent(const std::string& cell1, const std::string& cell2, std::vector<std::string> contents) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	setCellsContent(c1, c2, contents);
}

void gGUIGrid::setCellAlignment(int cellAlignment, bool clicked) {
	if(allcells.empty()) return;
	if(clicked) {
		setSelectedCells(true);
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

void gGUIGrid::setCellAlignment(Cell* cell, int cellAlignment) {
	cell->cellalignment = cellAlignment;
	cell->textmoveamount = 0.5f * cellAlignment;
}

void gGUIGrid::setCellAlignment(const std::string& cell, int cellAlignment) {
	Cell* c = getCell(cell);
	setCellAlignment(c, cellAlignment);
}
void gGUIGrid::setCellsAlignment(std::deque<Cell*> cells, int cellAlignment) {
	for(int i = 0; i < cells.size(); i++) setCellAlignment(cells[i], cellAlignment);
}

void gGUIGrid::setCellsAlignment(Cell* cell1, Cell* cell2, int cellAlignment) {
	int c1 = cell1->cellcolumnno;
	int c2 = cell2->cellcolumnno;
	int r1 = cell1->cellrowno;
	int r2 = cell2->cellrowno;
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
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
			Cell* c = &allcells[index];
			setCellAlignment(c, cellAlignment);
		}
	}
}

void gGUIGrid::setCellsAlignment(const std::string& cell1, const std::string& cell2, int cellAlignment) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	setCellsAlignment(c1, c2, cellAlignment);
}

void gGUIGrid::setSelectedFrameColor(gColor* selectedFrameColor) {
	selectedframecolor = *selectedFrameColor;
}

void gGUIGrid::setSelectedAreaColor(gColor* selectedAreaColor) {
	selectedareacolor = *selectedAreaColor;
}

void gGUIGrid::setColumnWidth(int columnNo, float width) {
	currentcolumn = columnNo;
	int windex = -1;
	float diff = 0.0f;
	for(int i = 0; i < gridboxesw.size(); i++) {
		if(columnNo == gridboxesw[i][0]) {
			windex = i;
			break;
		}
	}
	if(windex == -1) {
		diff = width - gridboxw;
		gridboxesw.push_back({(float)columnNo, width});
	}
	else {
		diff = width - gridboxesw[windex][1];
		gridboxesw[windex][1] = width;
	}
	updateAllAffectedCellWidths(columnNo, diff);
}

void gGUIGrid::setRowHeight(int rowNo, float height) {
	currentrow = rowNo;
	int hindex = -1;
	float diff = 0.0f;
	for(int i = 0; i < gridboxesh.size(); i++) {
		if(rowNo == gridboxesh[i][0]) {
			hindex = i;
			break;
		}
	}
	if(hindex == -1) {
		diff = height - gridboxh;
		gridboxesh.push_back({(float)rowNo, height});
	} else {
		diff = height - gridboxesh[hindex][1];
		gridboxesh[hindex][1] = height;
	}
	updateAllAffectedCellHeights(rowNo, diff);
}

void gGUIGrid::selectCell(Cell* cell) {
	selectCell(cell, cell);
}

void gGUIGrid::selectCell(int rowNo, int columnNo) {
	selectCell(getCell(rowNo, columnNo));
}

void gGUIGrid::selectCell(Cell* cell1, Cell* cell2) {
	isselected = true;
	firstselectedcell = cell1->cellrowno * columnnum + cell1->cellcolumnno;
	lastselectedcell = cell2->cellrowno * columnnum + cell2->cellcolumnno;
	int index = getCellNo(cell1->cellrowno, cell1->cellcolumnno);
	allcells[selectedbox].iscellselected = false;
	allcells[index].iscellselected = true;
	selectedbox = index;
	setSelectedCells();
}

void gGUIGrid::selectCell(const std::string& cell) {
	Cell* c = getCell(cell);
	selectCell(c, c);
}

void gGUIGrid::selectCell(const std::string& cell1, const std::string& cell2) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	selectCell(c1, c2);
}

gGUIGrid::Cell* gGUIGrid::getCell(const std::string& cellID) {
	std::string temp = cellID;
	std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
	std::string column = getTextColumn(temp);
	std::string row = temp.substr(column.size());
	for(int i = 0; i < row.size(); i++) {
		if(!isdigit(row[i])) {
			return nullptr;
		}
	}
	int columnindex = int(column[column.size() - 1]) % 65;
	for(int i = 0; i < column.size() - 1; i++) {
		columnindex += int((column[i] % 65 + 1) * (26 * ((column.size() - 1) - i)));
	}
	if(gToInt(row) - 1 < 0 || gToInt(row) - 1 >= rownum || columnindex < 0 || columnindex >= columnnum) {
		return nullptr;
	}
	int index = getCellNo(std::stoi(row) - 1, columnindex);
	if(index == -1) {
		index = createCell(gToInt(row) - 1, columnindex);
	}
	return &allcells[index];
}

gGUIGrid::Cell* gGUIGrid::getCell(int rowNo, int columnNo) {
	int index = getCellNo(rowNo, columnNo);
	if(index == -1) {
		index = createCell(rowNo, columnNo);
	}
	return &allcells[index];
}

gColor* gGUIGrid::getSelectedFrameColor() {
	return &selectedframecolor;
}

gColor* gGUIGrid::getSelectedAreaColor() {
	return &selectedareacolor;
}

std::deque<gGUIGrid::Cell*> gGUIGrid::getSelectedCells() {
	std::deque<Cell*> sc;
	for(int i = 0; i < selectedcells.size(); i++) sc.push_back(&allcells[selectedcells[i]]);
	return sc;
}

std::string gGUIGrid::getColumnName(int columnNo) {
	std::string result = "";
    while (columnNo >= 0) {
        int remainder = columnNo % 26;
        char letter = 'A' + remainder;
        result = letter + result;
        columnNo = (columnNo - remainder) / 26 - 1;
    }
	return result;
}

int gGUIGrid::getColumnNo(const std::string& columnName) {
	std::string temp = columnName;
	std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
	int result = 0;
	for (char c : temp) {
		result = result * 26 + (c - 'A' + 1);
	}
	return result - 1;
}

void gGUIGrid::digitToString() {
	if(allcells.empty()) return;
	allcells[selectedbox].celltype = Cell::TYPE_STRING;
	allcells[selectedbox].iscellaligned = true;
	setCellAlignment(gBaseGUIObject::TEXTALIGNMENT_LEFT, false);
}

std::string gGUIGrid::fixTextFunction(const std::string& text, int index) {
	std::string tempstr = text;
	for(int i = 0; i < functions.size(); i++) {
		if(gToInt(functions[i][FUNCTION_SENDER]) == index) {
			functions[i][FUNCTION_TYPE] = gToStr(FUNCTIONTYPE_NONE);
			break;
		}
	}
	if(tempstr == "") return tempstr;
	bool function = (tempstr[0] == '=');
	if(int(tempstr[0]) == 39) tempstr.erase(0, 1);
	else if(function) {
		tempstr.erase(0, 1);
		std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::toupper);
		tempstr = fixNumeric(tempstr);
		bool isnegative = (tempstr[0] == '-');

		if(!isdigit(tempstr[isnegative])) {
			tempstr.erase(0, isnegative);
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
							std::string row = parenthesesstr.substr(column.size());
							for(int i = 0; i < row.size(); i++)
								if(!isdigit(row[i])) return errormessage;
							int columnindex = int(column[column.size() - 1]) % 65;
							for(int i = 0; i < column.size() - 1; i++) columnindex += int((column[i] % 65 + 1) * (26 * ((column.size() - 1) - i)));
							if(columnindex < 0 || columnindex >= columnnum || gToInt(row) - 1 < 0 || gToInt(row) - 1 >= rownum) return errormessage;
							int copiedindex = getCellNo(gToInt(row) - 1, columnindex);
							if(copiedindex == -1) {
								copiedindex = createCell(gToInt(row) - 1, columnindex);
							}
							tempstr = allcells[copiedindex].showncontent;
							functionindexes.clear();
							functionindexes.push_back(gToStr(FUNCTIONTYPE_COPY));
							functionindexes.push_back(gToStr(index));
							functionindexes.push_back(gToStr(copiedindex));
							addOrChangeFunction(index);
						}
					}
					else {
						std::string str1 = parenthesesstr.substr(0, doubledat);
						std::string str2 = parenthesesstr.substr(str1.size() + 1);
						Cell* cell1 = getCell(str1);
						Cell* cell2 = getCell(str2);
						if(cell1 == nullptr || cell2 == nullptr) {
							return errormessage;
						}
						int columnno1 = cell1->cellcolumnno;
						int columnno2 = cell2->cellcolumnno;
						int rowno1 = cell1->cellrowno;
						int rowno2 = cell2->cellrowno;

						functionindexes.clear();
						functionindexes.push_back(gToStr(FUNCTIONTYPE_SUM));
						functionindexes.push_back(gToStr(index));
						tempstr = std::to_string(makeSum(columnno1, rowno1, columnno2, rowno2));
						addOrChangeFunction(index);
					}
					if(isnegative != !isparenthesesnegative) {
						tempstr = "-" + tempstr;
					}
				}
			}
			else {
				std::string cstr1 = tempstr;
				std::string csymbol1 = "";
				if(isnegative) csymbol1 = "-";
				int c1lastindex = cstr1.size();
				if(cstr1.find('/') != std::string::npos) {
					c1lastindex = cstr1.find('/');
					if(c1lastindex < cstr1.find('*') && cstr1.find('*') != std::string::npos) c1lastindex = cstr1.find('*');
				}
				else if(cstr1.find('*') != std::string::npos) c1lastindex = cstr1.find('*');
				else if(cstr1.find('+') != std::string::npos) c1lastindex = cstr1.find('+');
				else if(cstr1.find('-') != std::string::npos) c1lastindex = cstr1.find('-');
				cstr1 = cstr1.substr(0, c1lastindex);
				if(!isdigit(cstr1[cstr1.size() - 1])) return errormessage;
				if(cstr1.size() != tempstr.size()) {
					std::string cstr2 = tempstr.substr(cstr1.size());
					std::string csymbol2 = "";
					char operation = '\0';
					if(cstr2[0] == '/' || cstr2[0] == '*') {
						operation = cstr2[0];
						cstr2.erase(0, 1);
					}
					cstr2 = fixNumeric(cstr2);
					if(operation == '\0') {
						if(cstr2[0] != '+' && cstr2[0] != '-' && cstr2[0] != '/' && cstr2[0] != '*') operation = '+';
						else operation = cstr2[0];
					}
					bool c2neg = (cstr2[0] == '-' && operation != '-');
					if(c2neg) csymbol2 = "-";
					if(c2neg || operation == '-') cstr2.erase(0, 1);
					if(getCell(cstr1) == 0 || getCell(cstr2) == 0) return errormessage;
					if(operation == '+' || operation == '-' || operation == '/' || operation == '*') {
						tempstr = gToStr(makeFourOperation(cstr1, cstr2, operation, csymbol1, csymbol2));
						functionindexes.clear();
						switch(operation) {
						case '+':
							functionindexes.push_back(gToStr(FUNCTIONTYPE_ADD));
							break;
						case '-':
							functionindexes.push_back(gToStr(FUNCTIONTYPE_SUB));
							break;
						case '/':
							functionindexes.push_back(gToStr(FUNCTIONTYPE_DIVIDE));
							break;
						case '*':
							functionindexes.push_back(gToStr(FUNCTIONTYPE_MULTIPLY));
							break;
						}
						functionindexes.push_back(gToStr(index));
						functionindexes.push_back(gToStr(cstr1));
						functionindexes.push_back(gToStr(cstr2));
						functionindexes.push_back(gToStr(csymbol1));
						functionindexes.push_back(gToStr(csymbol2));
						addOrChangeFunction(index);
					}
					else return errormessage;
				}
				else {
					Cell* c = getCell(cstr1);
					if(c == nullptr) {
						return errormessage;
					}
					tempstr = c->showncontent;
					if(cstr1[0] == '-') {
						if(tempstr[0] != '-') tempstr = "-" + tempstr;
						else tempstr.erase(0, 1);
					}
					functionindexes.clear();
					functionindexes.push_back(gToStr(FUNCTIONTYPE_COPY));
					functionindexes.push_back(gToStr(index));
					functionindexes.push_back(gToStr(getCellNo(c->cellrowno, c->cellcolumnno)));
					addOrChangeFunction(index);
				}
			}
		}
		else {
			std::string number1 = "";
			if(isnegative) number1 += "-";
			for(int i = isnegative; i < tempstr.size(); i++) {
				if(isdigit(tempstr[i]) || tempstr[i] == '.') number1 += tempstr[i];
				else break;
			}
			if(number1.size() == tempstr.size()) return tempstr;
			else {
				std::string number2 = tempstr.substr(number1.size());
				char operation = '\0';
				if(number2[0] == '/' || number2[0] == '*') {
					operation = number2[0];
					number2.erase(0, 1);
				}
				number2 = fixNumeric(number2);
				if(operation == '\0') {
					if(number2[0] != '+' && number2[0] != '-' && number2[0] != '/' && number2[0] != '*') operation = '+';
					else {
						operation = number2[0];
						number2.erase(0, 1);
					}
				}
				bool digitn2 = (number2.size() > 0);
				for(int i = (number2[0] == '-'); i < number2.size(); i++) {
					if(!isdigit(number2[i]) && number2[i] != '.') {
						digitn2 = false;
						break;
					}
				}
				if(digitn2) {
					switch(operation) {
					case '+':
						tempstr = gToStr(gToFloat(number1) + gToFloat(number2));
						break;
					case '-':
						tempstr = gToStr(gToFloat(number1) - gToFloat(number2));
						break;
					case '/':
						tempstr = gToStr(gToFloat(number1) / gToFloat(number2));
						break;
					case '*':
						tempstr = gToStr(gToFloat(number1) * gToFloat(number2));
						break;
					}
				}
				else return errormessage;
			}
		}
	}
	return tempstr;
}

std::string gGUIGrid::fixNumeric(const std::string& text) {
	std::string tempstr = text;
	std::stack<int> unnecessaryindexes;
	if(tempstr[0] == '+') {
		unnecessaryindexes.push(0);
		for(int i = 1; i < tempstr.size(); i++) {
			if(tempstr[i] != '+' && tempstr[i] != '-') break;
			if(tempstr[i] == '+') unnecessaryindexes.push(i);
			else if(tempstr[i] == '-') continue;
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
		if (thisCell.showncontent.length() <= charindex) {
			break;
		}
		tmpstr += thisCell.showncontent[charindex];
		charindex++;
	}
	return tmpstr;
}

std::string gGUIGrid::getTextColumn(const std::string& text) {
    int found = text.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    if (found != std::string::npos && std::isalpha(text[0])) return text.substr(0, found);
    return text;
}

int gGUIGrid::getCellNo(int rowNo, int columnNo) {
	auto it = cellmap.find(hashCell(rowNo, columnNo));
	if (it == cellmap.end()) {
		return -1;
	}
	return it->second;
}

int gGUIGrid::getNearestFilledCell(int index) {
	int nearestindex = -1;
	Cell& cell = allcells[index];
	int rowno = cell.cellrowno;
	for (int column = 0; column < columnnum; column++) {
		if (column == cell.cellcolumnno) {
			continue;
		}
		int currentindex = getCellNo(rowno, column);
		if (currentindex == -1) {
			continue;
		}
		Cell& currentcell = allcells[currentindex];
		if(currentcell.showncontent.empty()) {
			continue;
		}
		if(cell.cellalignment != gBaseGUIObject::TEXTALIGNMENT_RIGHT && cell.cellx + font->getStringWidth(cell.showncontent) > currentcell.cellx && cell.cellcolumnno < currentcell.cellcolumnno) {
			if(nearestindex == -1 || allcells[nearestindex].cellcolumnno > currentcell.cellcolumnno) {
				nearestindex = currentindex;
			}
		} else if(cell.cellalignment == gBaseGUIObject::TEXTALIGNMENT_RIGHT && cell.cellx - (font->getStringWidth(cell.showncontent) - cell.cellw) < currentcell.cellx + currentcell.cellw && cell.cellcolumnno > currentcell.cellcolumnno) {
			if(nearestindex == -1 || allcells[nearestindex].cellcolumnno < currentcell.cellcolumnno) {
				nearestindex = currentindex;
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

void gGUIGrid::fillCell(int cellNo, const std::string& tempstr) {
	if(cellNo > rownum * columnnum - 1) return;
	allcells[cellNo].cellcontent = tempstr;
	allcells[cellNo].showncontent = fixTextFunction(tempstr, cellNo);

	checkCellType(cellNo);

	int delindex = -1;
	int passindex = 0;
	for(int i = 0; i < functions.size(); i++) {
		if(gToInt(functions[i][FUNCTION_TYPE]) == FUNCTIONTYPE_NONE) delindex = i;
		else {
			if(gToInt(functions[i][FUNCTION_TYPE]) == FUNCTIONTYPE_ADD || gToInt(functions[i][FUNCTION_TYPE]) == FUNCTIONTYPE_SUB || gToInt(functions[i][FUNCTION_TYPE]) == FUNCTIONTYPE_DIVIDE || gToInt(functions[i][FUNCTION_TYPE]) == FUNCTIONTYPE_MULTIPLY)
				passindex = 2;
			for(int j = FUNCTION_FIRSTINDEX; j < functions[i].size() - passindex; j++) {
				if((passindex != 2 && gToInt(functions[i][j]) == cellNo) || (passindex == 2 && getCell(functions[i][j]) == &allcells[cellNo])) {
					operateFunction(i);
					break;
				}
			}
		}
	}
	if(delindex != -1) functions.erase(functions.begin() + delindex);

	Cell& cell = allcells[cellNo];
	int nearestindex = -1;
	for (int i = cell.cellrowno + 1; i < columnnum; ++i) {
		Cell* next = getCell(cell.cellrowno, i);
		if (!next) {
			continue;
		}
		nearestindex = getNearestFilledCell(i);
		if(nearestindex != -1) {
			next->overflowcontent = fixOverflowText(*next, allcells[nearestindex]);
		} else {
			next->overflowcontent = "";
		}
	}

	if (cell.cellw < font->getStringWidth(cell.showncontent)) {
		nearestindex = getNearestFilledCell(cellNo);
		if(nearestindex != -1) {
			cell.overflowcontent = fixOverflowText(cell, allcells[nearestindex]);
		} else {
			cell.overflowcontent = "";
		}
	}
}

float gGUIGrid::makeSum(int c1, int r1, int c2, int r2) {
	if(c1 > c2) {
		c1 = c1 ^ c2;
		c2 = c1 ^ c2;
		c1 = c1 ^ c2;
	}
	if(r1 > r2) {
		r1 = r1 ^ r2;
		r2 = r1 ^ r2;
		r1 = r1 ^ r2;
	}
	float result = 0;
	for(int row = r1; row <= r2; row++) {
		for(int column = c1; column <= c2; column++) {
			int currentindex = getCellNo(row, column);
			if(currentindex == -1) {
				currentindex = createCell(row, column);
			}
			if(allcells[currentindex].celltype == Cell::TYPE_DIGIT) {
				std::string value = "";
				if(allcells[currentindex].showncontent[0] == '-') value += '-';
				for(int i = value.size(); i < allcells[currentindex].showncontent.size(); i++) {
					if(isdigit(allcells[currentindex].showncontent[i]) || allcells[currentindex].showncontent[i] == '.') value += allcells[currentindex].showncontent[i];
				}
				result += gToFloat(value);
			}
			functionindexes.push_back(gToStr(currentindex));
		}
	}
	return result;
}

float gGUIGrid::makeFourOperation(const std::string& cell1, const std::string& cell2, char operation, const std::string& value1symbol, const std::string& value2symbol) {
	Cell* c1 = getCell(cell1);
	Cell* c2 = getCell(cell2);
	std::string value1 = c1->showncontent;
	if(value1symbol == "-") {
		if(value1[0] == '-') value1.erase(0, 1);
		else value1 = '-' + value1;
	}
	std::string value2 = c2->showncontent;
	if(operation == '-' || value2symbol == "-") {
		if(value2[0] == '-') {
			value2.erase(0, 1);
			if(operation == '-') operation = '+';
		}
		else value2 = '-' + value2;
	}

	bool d1 = false;
	bool d2 = false;
	std::stack<int> spaceindexes;
	for(int i = 0; i < value1.size(); i++) {
		if(value1[i] == ',') spaceindexes.push(i);
	}
	while(!spaceindexes.empty()) {
		value1.erase(spaceindexes.top(), 1);
		spaceindexes.pop();
	}
	for(int i = 0; i < value2.size(); i++) {
		if(value2[i] == ',') spaceindexes.push(i);
	}
	while(!spaceindexes.empty()) {
		value2.erase(spaceindexes.top(), 1);
		spaceindexes.pop();
	}
	for(int i = (value1[0] == '-'); i < value1.size(); i++) {
		if(!isdigit(value1[i]) && value1[i] != '.') {
			value1 = "0";
			break;
		}
		d1 = true;
	}
	for(int i = (value2[0] == '-'); i < value2.size(); i++) {
		if(!isdigit(value2[i]) && value2[i] != '.') {
			value2 = "0";
			break;
		}
		d2 = true;
	}
	if(!d1) value1 = "0";
	if(!d2) value2 = "0";

	switch(operation) {
	case '+':
		return gToFloat(value1) + gToFloat(value2);
	case '-':
		return gToFloat(value1) + gToFloat(value2);
	case '/':
		return gToFloat(value1) / gToFloat(value2);
	case '*':
		return gToFloat(value1) * gToFloat(value2);
	default:
		return 0;
	}
}

float gGUIGrid::calculateCurrentX(int columnNo) {
	float currentx = gridx + gridboxwhalf - horizontalscroll;
	for(int column = 0; column < columnNo; column++) currentx += getColumnWidth(column);
	return currentx;
}

float gGUIGrid::calculateCurrentY(int rowNo) {
	float currenty = gridy + gridboxh - verticalscroll;
	for(int row = 0; row < rowNo; row++) currenty += getRowHeight(row);
	return currenty;
}

bool gGUIGrid::isNumeric(const std::string& text) {
	bool hasdigit = true;
	for(int i = 0; i < text.size(); i++) {
		if(!isdigit(text[i])) {
			hasdigit = false;
			break;
		}
	}
	return hasdigit;
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
			case PROCESS_FONTSIZE:
				undovaluestack.push(std::to_string(allcells[undocellstack.top()[i]].fontsize));
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
				undovaluestack.push(allcells[undocellstack.top()[i]].cellcontent + ":" + std::to_string(allcells[undocellstack.top()[i]].fontnum) + ":" + std::to_string(allcells[undocellstack.top()[i]].fontstate) + ":" + std::to_string(allcells[undocellstack.top()[i]].fontsize) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellalignment) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellfontcolor.r) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellfontcolor.g) + ":" + std::to_string(allcells[undocellstack.top()[i]].cellfontcolor.b) + ":" + std::to_string(allcells[undocellstack.top()[i]].lineno));
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
		case PROCESS_FONTSIZE:
			undocellstack.push(selectedcells);
			std::reverse(undocellstack.top().begin(), undocellstack.top().end());
			for(int i = 0; i < selectedcells.size(); i++) {
				undovaluestack.push(std::to_string(allcells[selectedcells[i]].fontsize));
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
					std::string tmpstr = allcells[selectedbox + amount].cellcontent + ":" + std::to_string(allcells[selectedbox + amount].fontnum) + ":" + std::to_string(allcells[selectedbox + amount].fontstate) + ":" + std::to_string(allcells[selectedbox + amount].fontsize) + ":" + std::to_string(allcells[selectedbox + amount].cellalignment) + ":" + std::to_string(allcells[selectedbox + amount].cellfontcolor.r) + ":" + std::to_string(allcells[selectedbox + amount].cellfontcolor.g) + ":" + std::to_string(allcells[selectedbox + amount].cellfontcolor.b) + ":" + std::to_string(allcells[selectedbox + amount].iscolorchanged) + ":" + std::to_string(allcells[selectedbox + amount].lineno);
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
						index = createCell(row, column);
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
		case PROCESS_FONTSIZE:
			redovaluestack.push(std::to_string(allcells[redocellstack.top()[i]].fontsize));
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
			redovaluestack.push(allcells[redocellstack.top()[i]].cellcontent + ":" + std::to_string(allcells[redocellstack.top()[i]].fontnum) + ":" + std::to_string(allcells[redocellstack.top()[i]].fontstate) + ":" + std::to_string(allcells[redocellstack.top()[i]].fontsize) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellalignment) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellfontcolor.r) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellfontcolor.g) + ":" + std::to_string(allcells[redocellstack.top()[i]].cellfontcolor.b) + ":" + std::to_string(allcells[redocellstack.top()[i]].iscolorchanged) + ":" + std::to_string(allcells[redocellstack.top()[i]].lineno));
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

void gGUIGrid::addOrChangeFunction(int functionSenderNo) {
	bool same = false;
	for(int i = 0; i < functions.size(); i++) {
		if(gToInt(functions[i][FUNCTION_SENDER]) == functionSenderNo) {
			functions[i] = functionindexes;
			same = true;
			operateFunction(i);
			break;
		}
	}
	if(!same) {
		functions.push_back(functionindexes);
		operateFunction(functions.size() - 1);
	}
	functionindexes.clear();
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
		allcells[selectedcells[i]].fontsize = 11;
		allcells[selectedcells[i]].cellalignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
		allcells[selectedcells[i]].lineno = TEXTLINE_NONE;
		allcells[selectedcells[i]].celltype = Cell::TYPE_STRING;
		allcells[selectedcells[i]].textmoveamount = 0;
		allcells[selectedcells[i]].cellcontent = "";
		allcells[selectedcells[i]].showncontent = "";
		allcells[selectedcells[i]].overflowcontent = "";
		allcells[selectedcells[i]].cellfontcolor = fontcolor;
		removeFunction(selectedcells[i]);
	}
}

void gGUIGrid::removeFunction(int cellNo) {
	for(int i = 0; i < functions.size(); i++) {
		if(gToInt(functions[i][FUNCTION_SENDER]) == cellNo) {
			functions.erase(functions.begin() + i);;
			break;
		}
	}
}

void gGUIGrid::operateFunction(int functionNo) {
	switch(gToInt(functions[functionNo][FUNCTION_TYPE])) {
	case FUNCTIONTYPE_COPY:
		allcells[gToInt(functions[functionNo][FUNCTION_SENDER])].showncontent = allcells[gToInt(functions[functionNo][FUNCTION_FIRSTINDEX])].showncontent;
		break;
	case FUNCTIONTYPE_SUM: {
		Cell* c1 = &allcells[gToInt(functions[functionNo][FUNCTION_FIRSTINDEX])];
		Cell* c2 = &allcells[gToInt(functions[functionNo][functions[functionNo].size() - 1])];
		allcells[gToInt(functions[functionNo][FUNCTION_SENDER])].showncontent = gToStr(makeSum(c1->cellcolumnno, c1->cellrowno, c2->cellcolumnno, c2->cellrowno));
		break;
	}
	case FUNCTIONTYPE_ADD:
		allcells[gToInt(functions[functionNo][FUNCTION_SENDER])].showncontent = gToStr(makeFourOperation(functions[functionNo][FUNCTION_FIRSTINDEX], functions[functionNo][FUNCTION_FIRSTINDEX + 1], '+', functions[functionNo][FUNCTION_FIRSTINDEX + 2], functions[functionNo][FUNCTION_FIRSTINDEX + 3]));
		break;
	case FUNCTIONTYPE_SUB:
		allcells[gToInt(functions[functionNo][FUNCTION_SENDER])].showncontent = gToStr(makeFourOperation(functions[functionNo][FUNCTION_FIRSTINDEX], functions[functionNo][FUNCTION_FIRSTINDEX + 1], '-', functions[functionNo][FUNCTION_FIRSTINDEX + 2], functions[functionNo][FUNCTION_FIRSTINDEX + 3]));
		break;
	case FUNCTIONTYPE_DIVIDE:
		allcells[gToInt(functions[functionNo][FUNCTION_SENDER])].showncontent = gToStr(makeFourOperation(functions[functionNo][FUNCTION_FIRSTINDEX], functions[functionNo][FUNCTION_FIRSTINDEX + 1], '/', functions[functionNo][FUNCTION_FIRSTINDEX + 2], functions[functionNo][FUNCTION_FIRSTINDEX + 3]));
		break;
	case FUNCTIONTYPE_MULTIPLY:
		allcells[gToInt(functions[functionNo][FUNCTION_SENDER])].showncontent = gToStr(makeFourOperation(functions[functionNo][FUNCTION_FIRSTINDEX], functions[functionNo][FUNCTION_FIRSTINDEX + 1], '*', functions[functionNo][FUNCTION_FIRSTINDEX + 2], functions[functionNo][FUNCTION_FIRSTINDEX + 3]));
		break;
	}

	checkCellType(gToInt(functions[functionNo][FUNCTION_SENDER]));
}

void gGUIGrid::updateAllAffectedCellWidths(int columnNo, float diff) {
	for(int i = 0; i < allcells.size(); i++) {
		if(allcells[i].cellcolumnno > columnNo) {
			allcells[i].cellx += diff;
		} else if(allcells[i].cellcolumnno == columnNo) {
			allcells[i].cellw += diff;
		}
	}
	updateTotalSize();
}

void gGUIGrid::updateAllAffectedCellHeights(int rowNo, float diff) {
	for(int i = 0; i < allcells.size(); i++) {
		if(allcells[i].cellrowno > rowNo) {
			allcells[i].celly += diff;
		} else if(allcells[i].cellrowno == rowNo) {
			allcells[i].cellh += diff;
		}
	}
	updateTotalSize();
}

void gGUIGrid::changeSelectedCell(int amount) {
	int firstcellbefore = firstselectedcell;
	int lastcellbefore = lastselectedcell;
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
	if(firstcellbefore != firstselectedcell) {
		if(calculateCurrentY(firstselectedcell / columnnum) < gridy + gridboxh) {
			verticalscroll += calculateCurrentY(firstselectedcell / columnnum) - gridboxh;
		} else if(calculateCurrentY(firstselectedcell / columnnum) + getRowHeight(firstselectedcell / columnnum) > gridy + boxh - titleheight) {
			verticalscroll += calculateCurrentY(firstselectedcell / columnnum) - (gridy + boxh - titleheight) + getRowHeight(firstselectedcell / columnnum);
		} else if(calculateCurrentX(firstselectedcell % columnnum) < gridx + gridboxwhalf) {
			horizontalscroll += calculateCurrentX(firstselectedcell % columnnum) - gridboxwhalf;
		} else if(calculateCurrentX(firstselectedcell % columnnum) + getColumnWidth(firstselectedcell % columnnum) > gridx + boxw) {
			horizontalscroll += calculateCurrentX(firstselectedcell % columnnum) - (gridx + boxw) + getColumnWidth(firstselectedcell % columnnum);
		}
	}
	else if(lastcellbefore != lastselectedcell){
		if(calculateCurrentY(lastselectedcell / columnnum) < gridy + gridboxh) {
			verticalscroll += calculateCurrentY(lastselectedcell / columnnum) - gridboxh;
		} else if(calculateCurrentY(lastselectedcell / columnnum) + getRowHeight(lastselectedcell / columnnum) > gridy + boxh - titleheight) {
			verticalscroll += calculateCurrentY(lastselectedcell / columnnum) - (gridy + boxh - titleheight) + getRowHeight(lastselectedcell / columnnum);
		} else if(calculateCurrentX(lastselectedcell % columnnum) < gridx + gridboxwhalf) {
			horizontalscroll += calculateCurrentX(lastselectedcell % columnnum) - gridboxwhalf;
		} else if(calculateCurrentX(lastselectedcell % columnnum) + getColumnWidth(lastselectedcell % columnnum) > gridx + boxw) {
			horizontalscroll += calculateCurrentX(lastselectedcell % columnnum) - (gridx + boxw) + getColumnWidth(lastselectedcell % columnnum);
		}
	}
	setSelectedCells();
	adjustScrollToFocusSelected();
}

void gGUIGrid::changeCell(int cellNo) {
	if(istextboxactive) {
		fillCell(cellNo, textbox.getText());
		textbox.cleanText();
		istextboxactive = false;
	} else {
		if(!ctrlzpressed && !ctrlypressed) {
			fillCell(cellNo, allcells[cellNo].cellcontent);
		}
		ctrlvpressed = false;
	}
}

void gGUIGrid::setSelectedCells(bool takeAll) {
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
	if(takeAll) {
		for(int column = c1; column <= c2; column++) {
			for(int row = r1; row <= r2; row++) {
				int index = getCellNo(row, column);
				if(index == -1) {
					index = createCell(row, column);
				}
				selectedcells.push_back(index);
			}
		}
	} else {
		int rowamount = r2 - r1 + 1;
		int columnamount = c2 - c1 + 1;
		for(int column = c1; column <= c2; column++) {
			for(int row = r1; row <= r2; row++) {
				int index = getCellNo(row, column);
				if(index == -1) {
					index = createCell(row, column);
				}
				selectedcells.push_back(index);
			}
		}
	}
}

void gGUIGrid::resetSelectedIndexes() {
	selectedcells.clear();
	if (selectedbox > -1) {
		firstselectedcell = allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno;
		lastselectedcell = firstselectedcell;
		selectedcells.push_back(selectedbox);
	} else {
		firstselectedcell = -1;
		lastselectedcell = -1;
	}
}

void gGUIGrid::copyCells() {
	copiedcellvalues.clear();
	for(int i = 0; i < selectedcells.size(); i++) {
		Cell& cell = allcells[selectedcells[i]];
		std::string tmpstr = std::to_string(cell.cellrowno) + ":" + cell.cellcontent + ":" + std::to_string(cell.fontnum) + ":" + std::to_string(cell.fontstate) + ":" + std::to_string(cell.fontsize) + ":" + std::to_string(cell.cellalignment) + ":" + std::to_string(cell.cellfontcolor.r) + ":" + std::to_string(cell.cellfontcolor.g) + ":" + std::to_string(cell.cellfontcolor.b) + ":" + std::to_string(cell.iscolorchanged) + ":" + std::to_string(cell.lineno);
		copiedcellvalues.push_back(tmpstr);
	}
}

void gGUIGrid::pasteCells() {
	int index = selectedbox;
	int row = (allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno) / columnnum;
	int column = (allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno) % columnnum;
	int colon = copiedcellvalues.at(copiedcellvalues.size() - 1).find(':');
	int maxrow = row + std::stoi(copiedcellvalues.at(copiedcellvalues.size() - 1).substr(0, colon));
	colon = copiedcellvalues.at(0).find(':');
	maxrow -= std::stoi(copiedcellvalues.at(0).substr(0, colon));
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
			case PROCESS_FONTSIZE:
				allcells[index].fontsize = std::stoi(tmpstr.substr(0, colon));
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
		row++;
		if(row > maxrow) {
			row = (allcells[selectedbox].cellrowno * columnnum + allcells[selectedbox].cellcolumnno) / columnnum;
			column++;
		}
		index = getCellNo(row, column);
		if(index == -1) {
			index = createCell(row, column);
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
		else if(undoprocessstack.top() == PROCESS_FONTSIZE) {
			setCellFontSize(std::stoi(undostack.top().top().substr(0, colon)));
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
			setCellFontSize(std::stoi(undostack.top().top().substr(0, colon)));
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
		else if(redoprocessstack.top() == PROCESS_FONTSIZE) {
			setCellFontSize(std::stoi(redostack.top().top().substr(0, colon)));
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
			setCellFontSize(std::stoi(redostack.top().top().substr(0, colon)));
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

int gGUIGrid::createCell(int rowNo, int columnNo) {
	Cell tempcell;
	tempcell.cellx = calculateCurrentX(columnNo) + horizontalscroll;
	tempcell.celly = calculateCurrentY(rowNo) + verticalscroll;
	tempcell.cellw = getColumnWidth(columnNo);
	tempcell.cellh = getRowHeight(rowNo);
	tempcell.cellrowno = rowNo;
	tempcell.cellcolumnno = columnNo;
	allcells.push_back(tempcell);
	int index = allcells.size() - 1;
	cellmap[hashCell(rowNo, columnNo)] = index;
	return index;
}

void gGUIGrid::createTextBox() {
	//allcells[selectedbox].cellx + 1
	//allcells.at(cellindexcounter).cellx + (allcells.at(cellindexcounter).cellw - font->getStringWidth(allcells.at(cellindexcounter).showncontent)) * textbox.getTextMoveAmount() - textbox.getInitX() * textbox.getTextAlignment() - horizontalscroll
	textbox.setEditable(true);
	textbox.set(root, this, this, 0, 0, allcells[selectedbox].cellx + 1, allcells[selectedbox].celly - 2, allcells[selectedbox].cellw - 10, allcells[selectedbox].cellh - 2);
	textbox.setTextFont(manager->getFont(allcells[selectedbox].fontnum, allcells[selectedbox].fontstate/*,allcells[selectedbox].fontsize*/));
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
	bool digit;
	bool isdate;
	if(int(allcells[cellIndex].cellcontent[0]) != 39) {
		bool isnegative = false;
		isdate = true;
		digit = true;
		std::stack<int> spaceindexes;
		bool isfractional = false;
		if(allcells[cellIndex].showncontent[0] == '-') isnegative = true;
		if(isdigit(allcells[cellIndex].showncontent[isnegative])) {
			for(int i = 1 + isnegative; i < allcells[cellIndex].showncontent.length(); i++) {
				if(allcells[cellIndex].showncontent[i] == '.') {
					if(!isdigit(allcells[cellIndex].showncontent[i - 1])) {
						digit = false;
						break;
					}
					isfractional = true;
					for(int j = i + 1; j < allcells[cellIndex].showncontent.length(); j++) {
						if(!isdigit(allcells[cellIndex].showncontent[j])) {
							isfractional = false;
							digit = false;
							break;
						}
					}
					if(!isfractional) break;
				}
				else if(allcells[cellIndex].showncontent[i] == ',' || allcells[cellIndex].showncontent[i] == ' ') {
					if(isdigit(allcells[cellIndex].showncontent[i - 1])) {
						if(i + 1 < allcells[cellIndex].showncontent.length()) {
							int next = 3;
							while(next > 0) {
								if(!isdigit(allcells[cellIndex].showncontent[i + next])) {
									digit = false;
									while(!spaceindexes.empty()) spaceindexes.pop();
									next = 0;
								}
								next--;
							}
							if(digit) spaceindexes.push(i);
						}
						else {
							digit = false;
							while(!spaceindexes.empty()) spaceindexes.pop();
							break;
						}
					}
					else {
						digit = false;
						while(!spaceindexes.empty()) spaceindexes.pop();
						break;
					}
				}
				else if(!isdigit(allcells[cellIndex].showncontent[i])) {
					digit = false;
					isnegative = false;
					isfractional = false;
					while(!spaceindexes.empty()) spaceindexes.pop();
					break;
				}
			}
		}
		else digit = false;
		if(digit) {
			if(isfractional) {
				int dotindex = allcells[cellIndex].showncontent.find('.');
				for(int i = dotindex + 1; i < allcells[cellIndex].showncontent.length(); i++) {
					if(allcells[cellIndex].showncontent[i] != '0') {
						dotindex = -1;
						break;
					}
				}
				if(dotindex != -1) allcells[cellIndex].showncontent.erase(dotindex, allcells[cellIndex].showncontent.length() - dotindex);
			}
			if(spaceindexes.empty()) {
				std::string tmpstr;
				if(isfractional && allcells[cellIndex].showncontent.find('.') != -1) tmpstr = allcells[cellIndex].showncontent.substr(0, allcells[cellIndex].showncontent.find('.'));
				else tmpstr = allcells[cellIndex].showncontent;
				int next = 3;
				std::deque<int> indexes;
				for(int i = tmpstr.length() - 1; i > isnegative; i--) {
					next--;
					if(next == 0) {
						next = 3;
						indexes.push_back(i);
					}
				}
				while(!indexes.empty()) {
					tmpstr.insert(indexes.front(), ",");
					indexes.pop_front();
				}
				if(isfractional && allcells[cellIndex].showncontent.find('.') != -1) allcells[cellIndex].showncontent = tmpstr + allcells[cellIndex].showncontent.substr(allcells[cellIndex].showncontent.find('.'));
				else allcells[cellIndex].showncontent = tmpstr;
			}
			else {
				while(!spaceindexes.empty()) {
					if(allcells[cellIndex].showncontent[spaceindexes.top()] != ',') allcells[cellIndex].showncontent[spaceindexes.top()] = ',';
					spaceindexes.pop();
				}
			}
		}
		else if(allcells[cellIndex].showncontent.length() == 10) {
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
			    else {
			    	if(allcells[cellIndex].showncontent[2] != '.') allcells[cellIndex].showncontent[2] = '.';
			    	if(allcells[cellIndex].showncontent[5] != '.') allcells[cellIndex].showncontent[5] = '.';
			    }
		    }
		}
		else if(allcells[cellIndex].showncontent.length() == 12) {
			std::string tmpstr = allcells[cellIndex].showncontent;
			std::transform(tmpstr.begin(), tmpstr.end(), tmpstr.begin(), ::toupper);
			for(int i = 0; i < 12; i++) {
				if(i == 4) {
					if(allcells[cellIndex].showncontent[i] != ',') {
						isdate = false;
						break;
					}
				}
				else if(i == 5 || i == 8) {
					if(allcells[cellIndex].showncontent[i] != ' ') {
						isdate = false;
						break;
					}
				}
				else if(i > 8) {
					if(int(tmpstr[i]) < 65 || int(tmpstr[i]) > 90) {
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
				int year = stoi(allcells[cellIndex].showncontent.substr(0, 4));
			    int day = stoi(allcells[cellIndex].showncontent.substr(6, 2));
			    std::string month = allcells[cellIndex].showncontent.substr(9, 3);
			    std::transform(month.begin(), month.end(), month.begin(), ::toupper);
			    int mon = 0;
			    if(month == "JAN") mon = 1;
			    else if(month == "FEB") mon = 2;
			    else if(month == "MAR") mon = 3;
			    else if(month == "APR") mon = 4;
			    else if(month == "MAY") mon = 5;
			    else if(month == "JUN") mon = 6;
			    else if(month == "JUL") mon = 7;
			    else if(month == "AUG") mon = 8;
			    else if(month == "SEP") mon = 9;
			    else if(month == "OCT") mon = 10;
			    else if(month == "NOV") mon = 11;
			    else if(month == "DEC") mon = 12;

			    if(day < 1 || day > 31 || mon < 1 || mon > 12 || year < 0) isdate = false;
			    else std::transform(allcells[cellIndex].showncontent.begin(), allcells[cellIndex].showncontent.end(), allcells[cellIndex].showncontent.begin(), ::toupper);
			}
		}
		else isdate = false;
	}
	else {
		digit = false;
		isdate = false;
	}
	if(!digit && !isdate) {
		if(allcells[cellIndex].celltype == Cell::TYPE_DIGIT && !allcells[cellIndex].iscellaligned) setCellAlignment(&allcells[cellIndex], gBaseGUIObject::TEXTALIGNMENT_LEFT);
		allcells[cellIndex].celltype = Cell::TYPE_STRING;
	}
	else {
		allcells[cellIndex].celltype = Cell::TYPE_DIGIT;
		if(!allcells[cellIndex].iscellaligned) setCellAlignment(&allcells[cellIndex], gBaseGUIObject::TEXTALIGNMENT_RIGHT);
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
//	gLogi("Grid") << "cellx: "<< allcells[cellindex].cellx
//		<< " celly: " << allcells[cellindex].celly
//		<< " cellrowno: " << allcells[cellindex].cellrowno
//		<< " cellcolumnno: " << allcells[cellindex].cellcolumnno
//		<< " cellcontent: " << allcells[cellindex].cellcontent
//		<< " celltype: " << allcells[cellindex].celltype;
}

void gGUIGrid::updateTotalSize() {
	gridw = 0;
	gridh = 0;
	for (int row = 0; row < rownum; ++row) {
		gridh += getRowHeight(row);
	}
	for (int column = 0; column < columnnum; ++column) {
		gridw += getColumnWidth(column);
	}
	totalw = gridw + gridboxh + 10;
	totalh = gridh + gridboxh + titleheight;
}

void gGUIGrid::adjustScrollToFocusSelected() {
	int cellindex = selectedbox;
	if (cellindex < 0) {
		return;
	}
	int cellx = allcells[cellindex].cellx - gridboxwhalf;
	int celly = allcells[cellindex].celly - gridboxh;
	int cellw = allcells[cellindex].cellw + gridboxwhalf;
	int cellh = allcells[cellindex].cellh + gridboxh;
	int cellx2 = cellx + cellw;
	int celly2 = celly + cellh;
	if(cellx < horizontalscroll) {
		horizontalscroll = cellx;
	} else if(cellx2 > horizontalscroll + boxw) {
		horizontalscroll = cellx2 - boxw;
	}

	if(celly < verticalscroll) {
		verticalscroll = celly;
	} else if(celly2 > verticalscroll + boxh) {
		verticalscroll = celly2 - boxh;
	}
}

void gGUIGrid::drawContent() {
//	gLogi("Scrollable") << "drawContent fx:" << horizontalscroll << ", fy:" << verticalscroll;
	gColor oldcolor = renderer->getColor();
	drawCellBackground();
	if(isselected || isrowselected || iscolumnselected) {
		drawSelectedArea();
	}
	drawCellContents();
	if(istextboxactive) {
		textbox.setFirstX(horizontalscroll);
		textbox.setFirstY(verticalscroll);
		textbox.draw();
	}

	drawColumnLines();
	drawRowLines();

	drawColumnHeader();
	drawRowHeader();
	drawHeaderFinal();

	renderer->setColor(oldcolor);
}

void gGUIGrid::drawCellBackground() {
	renderer->setColor(textbackgroundcolor);
	gDrawRectangle(gridx + gridboxwhalf, gridy + gridboxh, gridw - horizontalscroll, gridh - verticalscroll, true);
}

void gGUIGrid::drawRowHeader() {
	// draw header
	renderer->setColor(buttoncolor);
	gDrawRectangle(gridx, gridy + gridboxh, gridboxwhalf, gridh - verticalscroll, true);

	for(int i = 0; i < rownum; i++) {
		Cell* cell = getCell(i + 1, 0);
		int currenty = cell->celly - verticalscroll;
		if(currenty < gridx) {
			continue;
		}
		int rowheight = cell->cellh;
		if (currenty - rowheight > boxh) {
			break;
		}
		Cell* previouscell = getCell(i, 0);
		std::string rowtitlestring = std::to_string(i + 1);
		renderer->setColor(fontcolor);
		font->drawText(rowtitlestring,gridx + (gridboxw / 4) - (font->getStringWidth(rowtitlestring) / 2),
					   currenty - (previouscell->cellh / 2) + (font->getStringHeight(rowtitlestring) / 2)
		);
		renderer->setColor(pressedbuttoncolor);
		gDrawLine(gridx, currenty, gridx + gridboxwhalf, currenty);
	}
}

void gGUIGrid::drawRowLines() {
	// draw content lines
	for(int i = 0; i < rownum; i++) {
		Cell* cell = getCell(i + 1, 0);
		int currenty = cell->celly - verticalscroll;
		if(currenty < gridx) {
			continue;
		}
		int rowheight = cell->cellh;
		if (currenty - rowheight > boxh) {
			break;
		}
		renderer->setColor(pressedbuttoncolor);
		gDrawLine(gridx - horizontalscroll + gridboxwhalf, currenty, gridx + gridw + gridboxwhalf - horizontalscroll, currenty);
	}
}

void gGUIGrid::drawColumnHeader() {
	renderer->setColor(buttoncolor);
	gDrawRectangle(gridx + gridboxwhalf, gridy, gridw - horizontalscroll, gridboxh, true);

	for(int i = 0; i < columnnum; i++) {
		Cell* cell = getCell(0, i + 1);
		int currentx = cell->cellx - horizontalscroll;
		if(currentx < gridx) {
			continue;
		}
		int columnwidth = cell->cellw;
		if (currentx - columnwidth > boxw) {
			break;
		}
		Cell* previouscell = getCell(i, 0);

		std::string columntitlestring;
		if(i / 26 > 26) {
			columntitlestring = (char)(columntitle + i / (26 * 26) - 1);
			columntitlestring += (char)(columntitle + (i / 26 - 1) % 26);
			columntitlestring += (char)(columntitle + i % 26);
		} else if(i / 26 > 0) {
			columntitlestring = (char)(columntitle + i / 26 - 1);
			columntitlestring += (char)(columntitle + i % 26);
		} else {
			columntitlestring = (char)(columntitle + i);
		}
		renderer->setColor(fontcolor);
		font->drawText(columntitlestring,currentx - (previouscell->cellw / 2) - (font->getStringWidth(columntitlestring) / 2),
					   gridy + (gridboxh / 2) + (font->getStringHeight(columntitlestring) / 2));
		renderer->setColor(pressedbuttoncolor);
		gDrawLine(currentx,
				  gridy,
				  currentx,
				  gridy + gridboxh);
	}
}

void gGUIGrid::drawColumnLines() {
	// draw content lines
	for(int i = 0; i < columnnum; i++) {
		Cell* cell = getCell(0, i + 1);
		int currentx = cell->cellx - horizontalscroll;
		if(currentx < gridx) {
			continue;
		}
		int columnwidth = cell->cellw;
		if(currentx - columnwidth > boxw) {
			break;
		}
		renderer->setColor(pressedbuttoncolor);
		gDrawLine(currentx, gridy - verticalscroll + gridboxh, currentx, gridy + gridboxh + gridh - verticalscroll);
	}
}

void gGUIGrid::drawHeaderFinal() {
	// draws the square at the top left
	renderer->setColor(pressedbuttoncolor);
	gDrawRectangle(gridx, gridy, gridboxwhalf, gridboxh, true);

	// draw the lines for the headers (column and row)
	renderer->setColor(backgroundcolor);
	gDrawLine(gridx + gridboxwhalf + 1, gridy, gridx + gridboxwhalf + 1, gridh + gridboxh - verticalscroll);
	gDrawLine(gridx, gridy + gridboxh, gridw + gridboxwhalf - horizontalscroll, gridy + gridboxh);
}

void gGUIGrid::drawCellContents() {
	for(int i = 0; i < allcells.size(); i++) {
		Cell& currentcell = allcells[i];
		// visibility checks
		if(currentcell.cellx + currentcell.cellw - horizontalscroll < gridx) {
			continue;
		}
		if(currentcell.cellx - horizontalscroll > gridx + boxw) {
			continue;
		}
		if(currentcell.celly + currentcell.cellh - verticalscroll < gridy) {
			continue;
		}
		if(currentcell.celly - verticalscroll > gridy + boxh) {
			continue;
		}
		gFont& cellfont = *manager->getFont(currentcell.fontnum, currentcell.fontstate/*, currentcell.fontsize*/);
		int shownwidth = cellfont.getStringWidth(currentcell.showncontent);
		if(currentcell.cellx + shownwidth * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment < gridx + horizontalscroll) {
			continue;
		}

		if(!currentcell.iscolorchanged) {
			currentcell.cellfontcolor = fontcolor;
		}
		float currentstringwidth;
		float currentstringheight;
		if(currentcell.overflowcontent.empty()) {
			currentstringwidth = shownwidth;
			currentstringheight = cellfont.getStringHeight(currentcell.showncontent);

			renderer->setColor(currentcell.cellfontcolor);
			cellfont.drawText(currentcell.showncontent,
							   currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - horizontalscroll,
							   currentcell.celly + (gridboxh / 2) + (currentstringheight / 2) - verticalscroll);
		} else {
			currentstringwidth = cellfont.getStringWidth(currentcell.overflowcontent);
			currentstringheight = cellfont.getStringHeight(currentcell.overflowcontent);

			renderer->setColor(currentcell.cellfontcolor);
			cellfont.drawText(currentcell.overflowcontent,
							   currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - horizontalscroll,
							   currentcell.celly + (gridboxh / 2) + (currentstringheight / 2) - verticalscroll);
		}
		switch(currentcell.lineno) {
		case TEXTLINE_UNDER:
			gDrawLine(currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - horizontalscroll, currentcell.celly + (gridboxh / 2) + currentstringheight - verticalscroll,
					currentcell.cellx + currentstringwidth + ((currentcell.cellw - currentstringwidth) * currentcell.textmoveamount) - horizontalscroll, currentcell.celly + (gridboxh / 2) + currentstringheight - verticalscroll);
			break;
		case TEXTLINE_DOUBLEUNDER:
			gDrawLine(currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - horizontalscroll, currentcell.celly + (gridboxh / 2) + currentstringheight - verticalscroll,
					currentcell.cellx + currentstringwidth + ((currentcell.cellw - currentstringwidth) * currentcell.textmoveamount) - horizontalscroll, currentcell.celly + (gridboxh / 2) + currentstringheight - verticalscroll);
			gDrawLine(currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - horizontalscroll, currentcell.celly + (gridboxh / 2) + currentstringheight - verticalscroll + 2,
					currentcell.cellx + currentstringwidth + ((currentcell.cellw - currentstringwidth) * currentcell.textmoveamount) - horizontalscroll, currentcell.celly + (gridboxh / 2) + currentstringheight - verticalscroll + 2);
			break;
		case TEXTLINE_STRIKE:
			gDrawLine(currentcell.cellx + (currentcell.cellw - currentstringwidth) * currentcell.textmoveamount - textbox.getInitX() * currentcell.cellalignment - horizontalscroll, currentcell.celly + currentcell.cellh / 2 + textbox.getInitX() - verticalscroll,
					currentcell.cellx + currentstringwidth + ((currentcell.cellw - currentstringwidth) * currentcell.textmoveamount) - horizontalscroll, currentcell.celly + currentcell.cellh / 2 + textbox.getInitX() - verticalscroll);
			break;
		default:
			break;
		}
	}
}

void gGUIGrid::clear() {
	allcells.clear();
	cellmap.clear();
	selectedcells.clear();
	gridboxesw.clear();
	gridboxesh.clear();
	setGrid(1, 1);
}

void gGUIGrid::drawSelectedArea() {
	int sx = calculateCurrentX(firstselectedcell % columnnum);
	int sy = calculateCurrentY(int(firstselectedcell / columnnum));
	int sw = calculateCurrentX(lastselectedcell % columnnum) - sx + getColumnWidth(lastselectedcell % columnnum);
	int sh = calculateCurrentY(int(lastselectedcell / columnnum)) - sy + getRowHeight(int(lastselectedcell / columnnum));
	if(appmanager->getGUIManager()->getTheme() == gGUIManager::GUITHEME_DARK) renderer->setColor(selectedareadarkcolor);
	else renderer->setColor(selectedareacolor);
	gDrawRectangle(sx , sy, sw, sh, true);
	renderer->setColor(*textbackgroundcolor);
	gDrawRectangle(allcells[selectedbox].cellx - horizontalscroll, allcells[selectedbox].celly - verticalscroll, allcells[selectedbox].cellw, allcells[selectedbox].cellh, true);
	renderer->setColor(selectedframecolor);
	gDrawRectangle(sx + 1, sy + 1, sw - 2, sh - 2, false);
	gDrawRectangle(sx + sw - 2 - 6, sy + sh - 2 - 4, 6, 6, true); // FLAG
}

void gGUIGrid::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	if(x >= left + boxw || y >= top + boxh + titleheight) {
		return;
	}
	int pressedx = x - left + horizontalscroll;
	int pressedy = y - top + verticalscroll - titleheight;
	if(cursor == gGUIForm::CURSOR_VRESIZE || cursor == gGUIForm::CURSOR_HRESIZE) {
		firstcursorposx = x;
		firstcursorposy = y;

		previousclicktime = clicktime;
		clicktime = gGetSystemTimeMillis();
		if(clicktime - previousclicktime <= clicktimediff) {
			if(cursor == gGUIForm::CURSOR_VRESIZE) {
				int row = 0;
				while(calculateCurrentY(row) + getRowHeight(row) < pressedy - mousetolerance - verticalscroll) {
					row++;
				}
				int index = getCellNo(row, 0);
				if(index == -1) {
					index = createCell(row, 0);
				}
				currentrow = row;
				addOrChangeRowHeight(currentrow, gridboxh);
				int diff = gridboxh - allcells[index].cellh;
				updateAllAffectedCellHeights(currentrow, diff);
			} else if(cursor == gGUIForm::CURSOR_HRESIZE) {
				int column = 0;
				while(calculateCurrentX(column) + getColumnWidth(column) < pressedx - mousetolerance - horizontalscroll) column++;
				int index = getCellNo(0, column);
				if(index == -1) {
					index = createCell(0, column);
				}
				Cell& cell = allcells[index];
				gFont& cellfont = *manager->getFont(cell.fontnum, cell.fontstate/*, allcells[index].fontsize*/);
				for (int i = 1; i < rownum; ++i) {
					int loopcellindex = getCellNo(i, column);
					if (loopcellindex == -1) {
						continue;
					}
					Cell& loopcell = allcells[loopcellindex];
					gFont& loopcellfont = *manager->getFont(loopcell.fontnum, loopcell.fontstate/*, allcells[loopcellindex].fontsize*/);
					if(loopcellfont.getStringWidth(loopcell.showncontent) > cellfont.getStringWidth(cell.showncontent)) {
						index = i;
					}
				}
				cell = allcells[index];
				float neww = cellfont.getStringWidth(cell.showncontent) + textbox.getInitX() + 1;
				if(neww < font->getSize() * 1.8f) neww = font->getSize() * 1.8f;
				int diff = neww - cell.cellw;
				currentcolumn = column;
				addOrChangeColumnWidth(currentcolumn, neww);
				updateAllAffectedCellWidths(currentcolumn, diff);
			}
		}
	} else if(pressedx >= gridx + horizontalscroll && pressedx < gridx + gridboxwhalf + horizontalscroll && pressedy >= gridy + verticalscroll && pressedy < gridy + gridboxh + verticalscroll) {
		isselected = true;
		isrowselected = true;
		iscolumnselected = true;
		selectedbox = 0;
		firstselectedcell = 0;
		lastselectedcell = rownum * columnnum - 1;
		setSelectedCells();
	} else if(pressedx >= gridx + horizontalscroll && pressedx <= gridx + gridboxwhalf + gridw && pressedy >= gridy + verticalscroll && pressedy <= gridy + gridboxh + gridh) {
		if(pressedx >= gridx + gridboxwhalf + horizontalscroll && pressedx <= gridx + gridboxwhalf + gridw && pressedy >= gridy + gridboxh + verticalscroll && pressedy <= gridy + gridboxh + gridh) {
			isselected = true;
			isrowselected = false;
			iscolumnselected = false;
			int row = 0;
			int column = 0;
			while(calculateCurrentY(row) + getRowHeight(row) < pressedy - verticalscroll) row++;
			while(calculateCurrentX(column) + getColumnWidth(column) < pressedx - horizontalscroll) column++;
			int index = getCellNo(row, column);
			if(index == -1) {
				index = createCell(row, column);
			}
			if(istextboxactive) changeCell(selectedbox);
			allcells[selectedbox].iscellselected = false;
			allcells[index].iscellselected = true;
			selectedbox = index;
			resetSelectedIndexes();
			root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_CELLSELECTED, gToStr(row), gToStr(column));
		} else if(pressedx >= gridx + horizontalscroll && pressedx < gridx + gridboxwhalf + horizontalscroll && pressedy >= gridy + gridboxh + verticalscroll && pressedy <= gridy + gridboxh + gridh) {
			int row = 0;
			while(calculateCurrentY(row) + getRowHeight(row) < pressedy - verticalscroll) {
				row++;
			}
			int index = getCellNo(row, 0);
			if(index == -1) {
				index = createCell(row, 0);
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
			setSelectedCells();
		} else if(pressedx >= gridx + gridboxwhalf + horizontalscroll && pressedx < gridx + gridboxwhalf + gridw && pressedy >= gridy + verticalscroll && pressedy <= gridy + gridboxh + verticalscroll) {
			int column = 0;
			while(calculateCurrentX(column) + getColumnWidth(column) < pressedx - horizontalscroll) column++;
			int index = getCellNo(0, column);
			if(index == -1) {
				index = createCell(0, column);
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
	if(istextboxactive) textbox.mouseReleased((x - left), (y - top - verticalscroll - titleheight), button);
	gGUIScrollable::mouseReleased(x, y, button);
}

void gGUIGrid::mouseDragged(int x, int y, int button) {
//	gLogi("Grid") << x << " " << y;
	if(istextboxactive) textbox.mouseDragged((x - left), (y - top - verticalscroll), button);
	gGUIScrollable::mouseDragged(x, y, button);
	if(isdragginghorizontalscroll || isdraggingverticalscroll) {
		return;
	}
	if(cursor == gGUIForm::CURSOR_VRESIZE) {
		int diff = y - firstcursorposy;
		firstcursorposy = y;
		int h = getRowHeight(currentrow);
		if(diff > 0 || h > gridboxh) {
			if(h + diff >= gridboxh) {
				addOrChangeRowHeight(currentrow, h + diff);
			} else {
				diff = gridboxh - h;
				addOrChangeRowHeight(currentrow, h + diff);
			}
			updateAllAffectedCellHeights(currentrow, diff);
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
			updateAllAffectedCellWidths(currentcolumn, diff);
		}
	}
	else if(isselected ||isrowselected || iscolumnselected) {
		int row = 0;
		int column = 0;
		if(!iscolumnselected) while(calculateCurrentY(row) + getRowHeight(row) < y - top - titleheight) row++;
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
		if(!copiedcellvalues.empty()) {
			std::string tmpstr = copiedcellvalues[0];
			tmpstr.erase(0, tmpstr.find(':') + 1);
			tmpstr = tmpstr.substr(0, tmpstr.find(':'));
			if(tmpstr == appmanager->getClipboardString()) {
				addUndoStack(PROCESS_ALL);
				pasteCells();
			}
			else  {
				addUndoStack(PROCESS_TEXT);
				fillCell(selectedbox, appmanager->getClipboardString());
			}
		}
		else {
			addUndoStack(PROCESS_TEXT);
			fillCell(selectedbox, appmanager->getClipboardString());
		}
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
			index = createCell(newrow, newcolumn);
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
		adjustScrollToFocusSelected();
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
			adjustScrollToFocusSelected();
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
			adjustScrollToFocusSelected();
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
			adjustScrollToFocusSelected();
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
	int currentx = x - left + horizontalscroll;
	int currenty = y - top + verticalscroll - titleheight;
	cursor = gGUIForm::CURSOR_ARROW;
	currentrow = 0;
	currentcolumn = 0;
	if(currentx >= gridx + horizontalscroll && currentx < gridx + gridboxwhalf + horizontalscroll && currenty >= gridy + gridboxh + getRowHeight(currentrow) - mousetolerance - verticalscroll && currenty <= gridy + gridh + mousetolerance + getRowHeight(rownum)) {
		int currenth = gridy + gridboxh + getRowHeight(currentrow) - mousetolerance;
		while(currentrow + 1 < rownum && currenth + getRowHeight(currentrow + 1) <= currenty) {
			currenth += getRowHeight(currentrow + 1);
			currentrow++;
		}
		if(currenth <= currenty && currenth + mousetolerance * 2 >= currenty) cursor = gGUIForm::CURSOR_VRESIZE;
	}
	else if(currentx >= gridx + gridboxwhalf + getColumnWidth(currentcolumn) - mousetolerance - horizontalscroll && currentx < gridx + gridw + mousetolerance + getColumnWidth(columnnum) && currenty >= gridy + verticalscroll && currenty <= gridy + gridboxh + verticalscroll) {
		int currentw = gridx + gridboxwhalf + getColumnWidth(currentcolumn) - mousetolerance;
		while(currentcolumn + 1 < columnnum && currentw + getColumnWidth(currentcolumn + 1) <= currentx) {
			currentw += getColumnWidth(currentcolumn + 1);
			currentcolumn++;
		}
		if(currentw <= currentx && currentw + mousetolerance * 2 >= currentx) cursor = gGUIForm::CURSOR_HRESIZE;
	}
	return cursor;
}
