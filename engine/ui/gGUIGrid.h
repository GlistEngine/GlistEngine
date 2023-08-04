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
	    enum {
	    	TYPE_STRING,
			TYPE_DIGIT
	    };

	    bool iscellselected;
	    bool iscellaligned;
	    bool isbold;
	    bool isitalic;
	    int cellx;
	    int celly;
	    int cellrowno;
	    int cellcolumnno;
	    int fontnum;
	    int fontstate;
	    int cellalignment;
	    int lineno;
	    int celltype;
	    float cellh;
	    float cellw;
	    float textmoveamount;
	    std::string cellcontent;
	    std::string showncontent;
	    std::string overflowcontent;
	    gColor cellfontcolor;
	    Cell(){
	    	iscellselected = false;
	    	iscellaligned = false;
	    	isbold = false;
	    	isitalic = false;
	    	cellx = -1;
	    	celly = -1;
	    	fontnum = gGUIManager::FONT_FREESANS;
	    	fontstate = gGUIManager::FONTTYPE_REGULAR;
	    	cellalignment = gBaseGUIObject::TEXTALIGNMENT_LEFT;
	    	lineno = TEXTLINE_NONE;
	    	celltype = TYPE_STRING;
	    	cellh = 30.0f;
	    	cellw = 80.0f;
	    	textmoveamount = 0;
	    	cellcontent = "";
	    	showncontent = "";
	    	overflowcontent = "";
	    	cellfontcolor = fontcolor;
	    }
	};

	enum {
		TEXTLINE_NONE,
		TEXTLINE_UNDER,
		TEXTLINE_DOUBLEUNDER,
		TEXTLINE_STRIKE
	};

	enum {
		FUNCTION_COPY,
		FUNCTION_SUM
	};

	enum {
		FUNCTION_TYPE,
		FUNCTION_SENDER,
		FUNCTION_FIRSTINDEX
	};

	enum {
		PROCESS_TEXT,
		PROCESS_FONT,
		PROCESS_FONTSTATE,
		PROCESS_ALIGNMENT,
		PROCESS_COLOR,
		PROCESS_LINE,
		PROCESS_ALL
	};

	static const int maxcolumnnum = 16384;
	static const int maxrownum = 131071;

	gGUIGrid();
	virtual ~gGUIGrid();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void setGrid(int rowNum, int columnNum);
	void setRowNum(int rowNum);
	void setColumnNum(int columnNum);
	void setCellFont(int fontNum);
	void setCellFontBold();
	void setCellFontItalic();
	void setCellAlignment(int cellAlignment, bool clicked);
	void setCellFontColor(gColor* fontColor);
	void setCellLine(int lineNo, bool clicked);
	void setSelectedFrameColor(gColor* selectedFrameColor);
	void setSelectedAreaColor(gColor* selectedAreaColor);

	gColor* getSelectedFrameColor();
	gColor* getSelectedAreaColor();

	void drawContent();
	void drawCellBackground();
	void drawTitleRowBackground();
	void drawTitleColumnBackground();
	void drawRowContents();
	void drawColumnContents();
	void drawTitleLines();
	void drawCellContents();
	void drawSelectedArea();

	void update();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseScrolled(int x, int y);
	int getCursor(int x, int y);

private:
	static const int mousetolerance = 5;

	std::string fixTextFunction(std::string text, int index);
	std::string fixNumeric(std::string text);
	std::string fixOverflowText(Cell& thisCell, Cell& otherCell);

	std::string getTextColumn(std::string text);
	int getCellNo(int rowNo, int columnNo);
	int getNearestFilledCell(int index);

	void fillCell(int cellNo, std::string tempstr);
	float makeSum(int c1, int r1, int c2, int r2);
	float calculateCurrentX(int columnNum);
	float calculateCurrentY(int rowNum);
	bool isNumeric(std::string text);

	void addFunction(int functionType, int functionSender);
	void addUndoStack(int process);
	void addRedoStack();

	void removeFunction(int functionNum);

	void operateFunction(int functionNum);
	void makeDefaultCell();

	void changeAllAffectedCellsXW(float diff);
	void changeAllAffectedCellsYH(float diff);
	void changeSelectedCell(int amount);
	void changeCell(int cellNo);
	void resetSelectedIndexes();
	void setSelectedCells();

	void copyCells();
	void pasteCells();
	void makeUndo();
	void makeRedo();

	void createCell(int rowNo, int columnNo);
	void createTextBox();

	void checkCellType(int cellIndex);
	void showCells();
	void showCell(int rowNo, int columnNo);

	std::deque<Cell> allcells;
	std::deque<int> selectedcells;
	std::deque<float> gridboxesw;
	std::deque<float> gridboxesh;
	std::vector<int> functionindexes;
	std::vector<std::vector<int>> functions;
	std::vector<std::string> copiedcellvalues;
	std::stack<int> undoprocessstack;
	std::stack<std::string> undovaluestack;
	std::stack<std::stack<std::string>> undostack;
	std::stack<std::deque<int>> undocellstack;
	std::stack<int> redoprocessstack;
	std::stack<std::string> redovaluestack;
	std::stack<std::stack<std::string>> redostack;
	std::stack<std::deque<int>> redocellstack;
	gGUIManager* manager;
	gGUITextbox textbox;
	gColor selectedframecolor, selectedareacolor;
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
	int cursor;
	int currentrow, currentcolumn;
	int firstcursorposx, firstcursorposy;
	int firstselectedcell, lastselectedcell;
	int lastdraggedcell;
	float gridboxw, gridboxh;
	float gridx, gridy, gridw, gridh;
	long clicktime, previousclicktime, firstclicktime, clicktimediff;
	std::string strflag;
};

#endif /* UI_GGUIGRID_H_ */
