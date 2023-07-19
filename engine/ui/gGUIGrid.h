/*
 * gGUIGrid.h
 *
 *  Created on: 5 Aug 2022
 *      Author: murat
 */

#ifndef UI_GGUIGRID_H_
#define UI_GGUIGRID_H_


#include "gGUIScrollable.h"
#include "gGUITextbox.h"
#include "gGUIManager.h"
#include <deque>
#include <string.h>

//#include "gGUISizer.h"

class gGUIGrid: public gGUIScrollable {
public:
	struct Cell {
	    int cellx;
	    int celly;
	    float cellh;
	    float cellw;
	    int cellrowno;
	    int cellcolumnno;
	    bool iscellselected;
	    bool iscellaligned;
	    std::string cellcontent;
	    std::string showncontent;
	    std::string celltype;
	    int fontnum;
	    int cellalignment;
	    float textmoveamount;
	    gColor cellfontcolor;
	    int copiedno;
	    bool hasfunction;
	    int lineno;
	    Cell(){
	    	cellx = -1;
	    	celly = -1;
	    	cellh = 30.0f;
	    	cellw = 80.0f;
	    	iscellselected = false;
	    	iscellaligned = false;
	    	cellcontent = "";
	    	showncontent = "";
	    	celltype = "string";
	    	fontnum = gGUIManager::FONT_REGULAR;
	    	cellalignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
	    	textmoveamount = 0;
	    	cellfontcolor = fontcolor;
	    	copiedno = -1;
	    	hasfunction = false;
	    	lineno = TEXTLINE_NONE;
	    }
	};

	enum {
		TEXTLINE_NONE,
		TEXTLINE_UNDER,
		TEXTLINE_DOUBLEUNDER,
		TEXTLINE_STRIKE
	};

	gGUIGrid();
	virtual ~gGUIGrid();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void setGrid(int rowNum, int columnNum);
	void setRowNum(int rowNum);
	void setcolumnNum(int columnNum);

	Cell* getCell(int rowNo, int columnNo);

	void drawContent();
	void drawCellBackground();
	void drawSelectedBox();
	void drawSelectedRow();
	void drawSelectedColumn();
	void drawTitleRowBackground();
	void drawTitleColumnBackground();
	void drawRowContents();
	void drawColumnContents();
	void drawTitleLines();
	void drawCellContents();

	void fillCell(int rowNo, int columnNo, std::string tempstr);
	void createCells();
	void showCells();
	void showCell(int rowNo, int columnNo);
	void createTextBox();
	void changeCell();
	void checkCellType(int cellIndex);
	void changeCellFont(int fontNum);
	void changeCellAlignment(int cellAlignment, bool clicked);
	void changeCellFontColor(gColor* fontColor);
	void changeCellLine(int lineNo, bool clicked);
	void setCopiedCell(Cell* cell);

	void update();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseScrolled(int x, int y);

private:
	std::string fixTextFunction(std::string text);
	bool isNumeric(std::string text);
	std::string fixNumeric(std::string text);
	std::string getTextColumn(std::string text);
	int getCellIndex(std::string text);
	Cell getCopiedCell(int cellIndex);
	int makeSum(int c1, int r1, int c2, int r2);

	std::deque<Cell> allcells;
	int rownum, columnnum;
	float gridx, gridy, gridw, gridh;
	float gridboxw, gridboxh;
	int selectedbox;
	bool isselected, isrowselected, iscolumnselected;
	int rowtitle;
	int columntitle;
	gGUITextbox textbox;
	bool istextboxactive;
	long clicktime, previousclicktime, firstclicktime, clicktimediff;
	bool isdoubleclicked;
	int newgridboxw;
	gGUIManager* manager;
	int selectedtitle;
	bool shiftpressed, ctrlpressed;
	bool ctrlcpressed, ctrlvpressed, ctrlzpressed, ctrlypressed;
	Cell copiedcell;
	std::stack<Cell> undocellstack;
	std::stack<Cell> redocellstack;
};

#endif /* UI_GGUIGRID_H_ */
