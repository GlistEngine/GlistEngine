/*
 * gGUIColumnChart.h
 *
 *  Created on: 11 Aðu 2022
 *      Author: gsb4t
 */

#ifndef UI_GGUICOLUMNCHART_H_
#define UI_GGUICOLUMNCHART_H_

#include "gGUIControl.h"

class gGUIColumnChart: public gGUIControl {
public:
	gGUIColumnChart();
	virtual ~gGUIColumnChart();

	void draw();
	void mouseMoved(int x, int y);
	void setColumn();
	void setChartProp(int chartx, int charty, int chartw, int charth, int chartnumberrange); //number range : 0 - chartnumberrange
	void addColumn(int columnh, gColor color);
	void addMultiColumn(int columnum = 1, int columnh = 0, int columnh2 = 0, int columnh3 = 0, int columnh4 = 0, gColor color = {100, 100, 100}, gColor color2 = {100, 100, 100}, gColor color3 = {100, 100, 100}, gColor color4 = {100, 100, 100}, std::string title = "");
private:
	int chartw, charth;
	int chartx, charty;
	int columnh;
	int columnx, columny;
	int multicolumnx, multicolumny;
	int indexcolumn;
	int indexmulticolumn;
	int initialmcspadding;
	int columnsw;
	int chartaspect;
	int chartliney;
	int numberrange;
	int numberrangey;
	int numberrangeaspect;
	int colorindex;
	int columnnumber;
	std::string numbersinchart;
	std::vector<int> columnhe;
	std::vector<int> columnxe;
	std::vector<std::vector<int>> multicolumnh;
	std::vector<int> multicolumnxe;
	std::vector<std::vector<int>> singlecolumscolor;
	std::vector<std::vector<int>> multicolumscolor;
	std::vector<std::string> textvector;

	void drawColumns();
	void drawMultiColumns();
	void drawLinesAndNumbers();
};

#endif /* UI_GGUICOLUMNCHART_H_ */
