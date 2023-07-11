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
#include <deque>
#include <string.h>
#include "gGUIManager.h"

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
	    std::string cellcontent;
	    std::string showncontent;
	    std::string celltype;
	    int fontnum;
	    Cell(){
	    	cellx = -1;
	    	celly = -1;
	    	cellh = 30.0f;
	    	cellw = 80.0f;
	    	iscellselected = false;
	    	cellcontent = "";
	    	showncontent = "";
	    	fontnum = gGUIManager::FONT_REGULAR;
	    }
	};
	gGUIGrid();
	virtual ~gGUIGrid();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void setGrid(int rowNum, int columnNum);
	void setRowNum(int rowNum);
	void setcolumnNum(int columnNum);
	void setTextAlignment(int textAlignment);

	Cell* getCell(int rowNo, int columnNo);

	void drawContent();
	void drawCellBackground();
	void drawSelectedBox();
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

	void update();

	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseScrolled(int x, int y);

private:
	std::deque<Cell> allcells;
	int rownum, columnnum;
	float gridx, gridy, gridw, gridh;
	float gridboxw, gridboxh;
	int selectedbox;
	bool isselected;
	int rowtitle;
	int columntitle;
	gGUITextbox textbox;
	bool istextboxactive;
	long clicktime, previousclicktime, firstclicktime, clicktimediff;
	bool isdoubleclicked;
	int newgridboxw;
	gGUIManager* manager;
};

#endif /* UI_GGUIGRID_H_ */
