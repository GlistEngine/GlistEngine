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
#include <array>
#include <vector>
#include <unordered_map>

//#include "gGUISizer.h"

class gGUIGrid : public gGUIScrollable {
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
	    bool iscolorchanged;
	    int cellx;
	    int celly;
	    int cellrowno;
	    int cellcolumnno;
	    int fontnum;
	    int fontstate;
	    int fontsize;
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
		bool removed;
		bool readonly;

	    Cell() {
			readonly = false;
			removed = false;
	    	iscellselected = false;
	    	iscellaligned = false;
	    	isbold = false;
	    	isitalic = false;
	    	iscolorchanged = false;
	    	cellx = -1;
	    	celly = -1;
	    	fontnum = gGUIManager::FONT_FREESANS;
	    	fontstate = gGUIManager::FONTTYPE_REGULAR;
	    	fontsize = 11;
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
		FUNCTIONTYPE_NONE,
		FUNCTIONTYPE_COPY,
		FUNCTIONTYPE_SUM,
		FUNCTIONTYPE_ADD,
		FUNCTIONTYPE_SUB,
		FUNCTIONTYPE_DIVIDE,
		FUNCTIONTYPE_MULTIPLY
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
		PROCESS_FONTSIZE,
		PROCESS_ALIGNMENT,
		PROCESS_COLOR,
		PROCESS_LINE,
		PROCESS_ALL
	};

	const int maxcolumnnum = 16384;
	const int maxrownum = 131071;

	gGUIGrid();
	virtual ~gGUIGrid();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void setGrid(int rowNum, int columnNum);
	void setRowNum(int rowNum);
	int getRowNum();
	void setColumnNum(int columnNum);
	int getColumnNum();

	void setCellFont(int fontNo);
	void setCellFont(Cell* cell, int fontNo);
	void setCellFont(const std::string& cell, int fontNo);
	void setCellsFont(std::deque<Cell*> cells, int fontNo);
	void setCellsFont(Cell* cell1, Cell* cell2, int fontNo);
	void setCellsFont(const std::string& cell1, const std::string& cell2, int fontNo);

	void setCellFontBold();
	void setCellFontBold(Cell* cell);
	void setCellFontBold(const std::string& cell);
	void setCellsFontBold(std::deque<Cell*> cells);
	void setCellsFontBold(Cell* cell1, Cell* cell2);
	void setCellsFontBold(const std::string& cell1, const std::string& cell2);

	void setCellFontItalic();
	void setCellFontItalic(Cell* cell);
	void setCellFontItalic(const std::string& cell);
	void setCellsFontItalic(std::deque<Cell*> cells);
	void setCellsFontItalic(Cell* cell1, Cell* cell2);
	void setCellsFontItalic(const std::string& cell1, const std::string& cell2);

	void setCellFontSize(int fontSize);
	void setCellFontSize(Cell* cell, int fontSize);
	void setCellFontSize(const std::string& cell, int fontSize);
	void setCellsFontSize(std::deque<Cell*> cells, int fontSize);
	void setCellsFontSize(Cell* cell1, Cell* cell2, int fontSize);
	void setCellsFontSize(const std::string& cell1, const std::string& cell2, int fontSize);

	void setCellFontColor(gColor* fontColor = fontcolor);
	void setCellFontColor(Cell* cell, gColor* fontColor = fontcolor);
	void setCellFontColor(const std::string& cell, gColor* fontColor = fontcolor);
	void setCellsFontColor(std::deque<Cell*> cells, gColor* fontColor = fontcolor);
	void setCellsFontColor(Cell* cell1, Cell* cell2, gColor* fontColor = fontcolor);
	void setCellsFontColor(const std::string& cell1, const std::string& cell2, gColor* fontColor = fontcolor);

	void setCellLine(int lineNo, bool clicked = true);
	void setCellLine(Cell* cell, int lineNo, bool clicked = true);
	void setCellLine(const std::string& cell, int lineNo, bool clicked = true);
	void setCellsLine(std::deque<Cell*> cells, int lineNo, bool clicked = true);
	void setCellsLine(Cell* cell1, Cell* cell2, int lineNo, bool clicked = true);
	void setCellsLine(const std::string& cell1, const std::string& cell2, int lineNo, bool clicked = true);

    void setCellContent(Cell* cell, const std::string& cellContent);
    void setCellContent(int rowNo, int columnNo, const std::string& cellContent);
    void setCellContent(const std::string& cell, const std::string& cellContent);
    void setCellsContent(std::deque<Cell*> cells, std::vector<std::string> contents);
    void setCellsContent(Cell* cell1, Cell* cell2, std::vector<std::string> contents);
    void setCellsContent(const std::string& cell1, const std::string& cell2, std::vector<std::string> contents);

	void setCellAlignment(int cellAlignment, bool clicked = true);
    void setCellAlignment(Cell* cell, int cellAlignment);
    void setCellAlignment(const std::string& cell, int cellAlignment);
    void setCellsAlignment(std::deque<Cell*> cells, int cellAlignment);
    void setCellsAlignment(Cell* cell1, Cell* cell2, int cellAlignment);
    void setCellsAlignment(const std::string& cell1, const std::string& cell2, int cellAlignment);

	void setCellReadOnly(Cell* cell, bool readonly);
	void setCellReadOnly(int rowNo, int columnNo, bool readonly);
	void setCellReadOnly(const std::string& cell, bool readonly);

	void setSelectedFrameColor(gColor* selectedFrameColor);
	void setSelectedAreaColor(gColor* selectedAreaColor);

	void setColumnWidth(int columnNo, float width);
	void setRowHeight(int rowNo, float height);

    void selectCell(Cell* cell);
    void selectCell(int rowNo, int columnNo);
    void selectCell(Cell* cell1, Cell* cell2);
    void selectCell(const std::string& cell);
    void selectCell(const std::string& cell1, const std::string& cell2);

	Cell* getCell(const std::string& cellID);
	Cell* getCell(int rowNo, int columnNo);
	gColor* getSelectedFrameColor();
	gColor* getSelectedAreaColor();
	std::deque<Cell*> getSelectedCells();
	std::string getColumnName(int columnNo);
	int getColumnNo(const std::string& columnName);
	float getColumnWidth(int columnNo);
	float getRowHeight(int rowNo);

	void digitToString();

	void drawContent();
	void drawCellBackground();
	void drawRowHeader();
	void drawRowLines();
	void drawColumnHeader();
	void drawColumnLines();
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

	void clear();

private:
	static const int mousetolerance = 5;
	const std::string errormessage = "Error";

	std::string fixTextFunction(const std::string& text, int index);
	std::string fixNumeric(const std::string& text);
	std::string fixOverflowText(Cell& thisCell, Cell& otherCell);

	std::string getTextColumn(const std::string& text);
	int getCellNo(int rowNo, int columnNo);
	int getNearestFilledCell(int index);

	void fillCell(int cellNo, const std::string& tempstr);
	float makeSum(int c1, int r1, int c2, int r2);
	float makeFourOperation(const std::string& cell1, const std::string& cell2, char operation, const std::string& value1symbol = "", const std::string& value2symbol = "");
	float calculateCurrentX(int columnNo);
	float calculateCurrentY(int rowNo);
	bool isNumeric(const std::string& text);

	void addUndoStack(int process);
	void addRedoStack();
	void addOrChangeColumnWidth(int columnNo, float w);
	void addOrChangeRowHeight(int rowNo, float h);
	void addOrChangeFunction(int functionSenderNo);

	void makeDefaultCell();
	void removeFunction(int cellNo);
	void operateFunction(int functionNo);

	void updateAllAffectedCellWidths(int columnNo, float diff);
	void updateAllAffectedCellHeights(int rowNo, float diff);
	void changeSelectedCell(int amount);
	void changeCell(int cellNo);
	void setSelectedCells(bool takeAll = false);
	void resetSelectedIndexes();

	void copyCells();
	void pasteCells();
	void makeUndo();
	void makeRedo();

	int createCell(int rowNo, int columnNo);
	/**
	 * Creates and overrides the cell at given index.
	 * If the given index is out of bounds, it creates a new cell at the end of the list.
	 */
	int createCell(int index, int rowNo, int columnNo);
	void createTextBox();

	void checkCellType(int cellIndex);
	void showCells();
	void showCell(int rowNo, int columnNo);

	/**
	 * Enables the text input for cell. If cell is read-only, it will do nothing.
	 *
	 * @param cell Target cell
	 * @param clear Whether or not to clear the text before enabling the input
	 */
	void editCell(Cell& cell, bool clear = false);

	void disableTextbox();

	void updateTotalSize();
	void adjustScrollToFocusSelected();

	uint64_t hashCell(int row, int column) {
		return (uint64_t)row << 32L | column;
	}

	std::unordered_map<uint64_t, int> cellmap;
	std::deque<Cell> allcells;
	std::deque<int> selectedcells;
	std::deque<std::array<float, 2>> gridboxesw;
	std::deque<std::array<float, 2>> gridboxesh;
	std::deque<std::string> functionindexes;
	std::deque<std::deque<std::string>> functions;
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
	gColor selectedframecolor, selectedareacolor, selectedareadarkcolor;
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
	float gridboxw, gridboxh, gridboxwhalf;
	float gridx, gridy, gridw, gridh;
	long clicktime, previousclicktime, firstclicktime, clicktimediff;
	std::string strflag;
};

#endif /* UI_GGUIGRID_H_ */
