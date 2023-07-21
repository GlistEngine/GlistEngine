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
	    bool iscellselected;
	    bool iscellaligned;
	    bool hasfunction;
	    int cellx;
	    int celly;
	    int cellrowno;
	    int cellcolumnno;
	    int copiedno;
	    int fontnum;
	    int cellalignment;
	    int lineno;
	    float cellh;
	    float cellw;
	    float textmoveamount;
	    std::string cellcontent;
	    std::string showncontent;
	    std::string celltype;
	    gColor cellfontcolor;
	    Cell(){
	    	iscellselected = false;
	    	iscellaligned = false;
	    	hasfunction = false;
	    	cellx = -1;
	    	celly = -1;
	    	copiedno = -1;
	    	fontnum = gGUIManager::FONT_REGULAR;
	    	cellalignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
	    	lineno = TEXTLINE_NONE;
	    	cellh = 30.0f;
	    	cellw = 80.0f;
	    	textmoveamount = 0;
	    	cellcontent = "";
	    	showncontent = "";
	    	celltype = "string";
	    	cellfontcolor = fontcolor;
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
	void setColumnNum(int columnNum);
	void setCellFont(int fontNum);
	void setCellAlignment(int cellAlignment, bool clicked);
	void setCellFontColor(gColor* fontColor);
	void setCellLine(int lineNo, bool clicked);
	void setCopiedCell(Cell* cell);

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
	std::string fixNumeric(std::string text);
	std::string getTextColumn(std::string text);
	int getCellIndex(std::string text);
	Cell getCopiedCell(int cellIndex);
	int makeSum(int c1, int r1, int c2, int r2);
	bool isNumeric(std::string text);

	std::deque<Cell> allcells;
	std::stack<Cell> undocellstack;
	std::stack<Cell> redocellstack;
	Cell copiedcell;
	gGUIManager* manager;
	gGUITextbox textbox;
	bool isselected, isrowselected, iscolumnselected;
	bool istextboxactive;
	bool isdoubleclicked;
	bool shiftpressed, ctrlpressed;
	bool ctrlcpressed, ctrlvpressed, ctrlzpressed, ctrlypressed;
	int selectedbox;
	int selectedtitle;
	int rownum, columnnum;
	int rowtitle;
	int columntitle;
	float gridboxw, gridboxh;
	float newgridboxw;
	float gridx, gridy, gridw, gridh;
	long clicktime, previousclicktime, firstclicktime, clicktimediff;
	std::string strflag;
};

#endif /* UI_GGUIGRID_H_ */
