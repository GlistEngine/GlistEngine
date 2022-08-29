/*
 * gGUICandleStickChart.h
 *
 *  Created on: 15 Aðu 2022
 *      Author: Berke Adil
 */

#ifndef UI_GGUICANDLESTICKCHART_H_
#define UI_GGUICANDLESTICKCHART_H_

#include "gImage.h"
#include "gGUIFrame.h"
#include "gGUIPanel.h"
#include "gGUIButton.h"
#include "gGUITextbox.h"
#include "gGUIControl.h"
#include "gLine.h"
#include "gRectangle.h"



class gGUICandleStickChart: public gGUIControl  {

public:
	gGUICandleStickChart();
	virtual ~gGUICandleStickChart();

	void setSize(int width, int height);

	void setGraphColor(gColor color);


	gColor* getGraphColor();

	virtual void update();

	void draw();
	void graph();
	void addValue(float y, float ch, float open, float close);
	void addpoint();


	void drawXAxis();
	void drawYAxis();
	void setXAxisValues(int min, int max, int range);
	void setYAxisValues(int min, int max, int range);

protected:
	int candlew, candleh;
	gColor cscolor;

	void resetTitlePosition();

private:
	float x[300];
	float y[300];
	gLine lines[300];
	int pointnum;
	bool shown;
	gRectangle candle[300];
	float ch[300];
	float open[300];
	float close[300];
	float a[300];
	int locationnum;
	int xAxis[300];
	int xx;
	int maxx;
	int maxy;

	std::vector<std::vector<int>> linesX;
	std::vector<std::vector<int>> linesY;

};



#endif /* UI_GGUICANDLESTICKCHART_H_ */
