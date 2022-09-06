/*
 * gGUIGrid.h
 *
 *  Created on: 5 Aðu 2022
 *      Author: murat
 */

#ifndef UI_GGUIGRID_H_
#define UI_GGUIGRID_H_

#include "gGUIScrollable.h"
#include "gGUITextbox.h"
#include <deque>

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
	    Cell(){
	    	cellx = -1;
	    	celly = -1;
	    	cellh = 30.0f;
	    	cellw = 80.0f;
	    	iscellselected = false;
	    	cellcontent = "";
	    	showncontent = "";
	    }
	};
	gGUIGrid();
	virtual ~gGUIGrid();

	void mousePressed(int x, int y, int button);

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void setGrid(int rowNum, int columnNum);
	void setRowNum(int rowNum);
	void setcolumnNum(int columnNum);

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
	bool isRightCellFull(int cellIndex);
	void createTextBox(int x, int y);

private:

	static const int SELECTEDBOX_X, SELECTEDBOX_Y;

	std::deque<Cell> allcells;
	int rownum, columnnum;
	float gridx, gridy, gridw, gridh;
	float gridboxw, gridboxh;
	int selectedbox[2];
	bool isselected;
	int rowtitle;
	int columntitle;
	gGUITextbox textbox;


};

#endif /* UI_GGUIGRID_H_ */
