/*
 * gGUIColumnChart.h
 *
 *  Created on: 11 Ağu 2022
 *      Author: Batuhan
 *-------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *                                         HOW TO USE
 *    First, create an object of this class on the canvas.
 *    Then call the class's setChartProp method using this object.
 *    Enter the expected values ​​respectively as parameters of this method. Parameter description below.
 *     -chartx and charty variables set position of x and y axis.
 *     -chartw and charth variables set width and height of x and y axis.
 *     -columnsw is width of the columns (both normal column and multi columns)
 *     -columnsmargin is distance of between columns.(is the distance between multi columns and other multi columns for multicolumns)
 *     -chartnumberrange is the value range on on the y-axis.The value you enter will be printed as equal divisions from 0 to the number you enter.
 *    After that you can add single columns by using addColumn function. Parameter description below.
 *     -columnh is the height of the column which you add.
 *     -color is the color of the column. (Example usage on the canvas : , gColor(100, 100, 100) )
 *    If you want to create multicolumn group, you can use addMultiColumn function. Parameter description below.
 *     -(ATTENTION) columnnum is how many column that you want to add to group (max limit 4 column for now).
 *     	 This parameter provides that x location of last columns which you specified in multicolum group is calculated automatically.
 *     	 So, even If you are going to enter a value of 0 to your column, please enter to the columnum parameter by counting the column which you value 0.
 *     	 If you want to add fewer than 4 column, after you write columnum, you have to write 0 again other columnh which you don't want to add.
 *     	 ***thus, the width of the column which you don't want to add is not calculated for margin.***
 *     -columnh ,columnh2, columnh3, columnh4 are the heights of the columns in the group.
 *     -The next 4 color parameters specify the colors of the columns, respectively.
 *       Even if you are going to create less than 4 columns, enter a gColor value for these 4 parameters.
 *      -title parameter is what typing under multi columns group.
 *      -multicolumnspadding is a parameter where you specify how much space you want between multi columns group.
 *    Finally, you can set visibility of additions. (Ex : visibility of grid lines, numbers on y-axis or text under x-axis).
 *    	-Parameters are girlineison, numberison, textison. (Event you don't use this method, values ​​will be accepted TRUE).
 *
 *
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
	void setChartProp(int chartx, int charty, int chartw, int charth, int columnsw, int columnsmargin, int chartnumberrange); //number range : 0 - chartnumberrange
	void setChartProp(int chartX, int chartY, int chartW, int chartH, int columnsW, int columnsMargin, int minNumRangeX, int maxNumRangeX, int minNumRangeY, int maxNumRangeY); //overloaded
	void addColumn(int columnh, gColor color);
	void addMultiColumn(int columnum = 1, int columnh = 0, int columnh2 = 0, int columnh3 = 0, int columnh4 = 0, gColor color = {100, 100, 100}, gColor color2 = {100, 100, 100}, gColor color3 = {100, 100, 100}, gColor color4 = {100, 100, 100}, std::string title = "", int multicolumnspadding = 0);
	void setVisibility(bool gridlineison = 1, bool numbersison = 1, bool textison = 1);
	void clear();
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
	int columnsmargin;
	int minnumrangex, maxnumrangex;
	int minnumrangey, maxnumrangey;
	int multicolumnspadding;
	bool gridlineison, numbersison, textison;
	std::string numbersinchart;
	std::vector<int> columnhe;
	std::vector<int> columnxe;
	std::vector<std::vector<int>> multicolumnh;
	std::vector<int> multicolumnxe;
	std::vector<std::vector<int>> singlecolumscolor;
	std::vector<std::vector<int>> multicolumscolor;
	std::vector<std::string> textvector;
	std::vector<int> multicolumnspaddingvector;

	void drawColumns();
	void drawMultiColumns();
	void drawLinesAndNumbers();
};

#endif /* UI_GGUICOLUMNCHART_H_ */
