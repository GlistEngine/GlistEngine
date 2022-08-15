/*
 * gGUISurface.h
 *
 *  Created on: 27 Jul 2022
 *      Author: Ezgi Lena Sonmez
 */

#ifndef UI_GGUISURFACE_H_
#define UI_GGUISURFACE_H_

#include "gGUIControl.h"
#include "gRenderer.h"
#include "gImage.h"
#include <deque>


class gGUISurface: public gGUIControl {
public:
	gGUISurface();
	virtual ~gGUISurface();
	void draw();

	void setBorder(float thickness, float borderposition);
	void addRectangle(float x, float y, float w, float h, bool isFilled, gColor color = gColor(0.0f, 0.0f, 0.0f));
	//void addRectangle(float x, float y, float w, float h, bool isFilled, float thickness, float borderposition, gColor color = gColor(0.0f, 0.0f, 0.0f));
	void addLine(float x, float y, float x2, float y2, gColor color = gColor(0.0f, 0.0f, 0.0f));
	//void drawLine(float x1, float y1, float z1, float x2, float y2, float z2);
	void addCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides = 64.0f, gColor color = gColor(0.0f, 0.0f, 0.0f));
	void addArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle, gColor color = gColor(0.0f, 0.0f, 0.0f));
	void addTriangle(float px, float py, float qx, float qy, float rx, float ry, bool isFilled, gColor color = gColor(0.0f, 0.0f, 0.0f));
	void addImage(float x, float y, float w, float h, gImage* image);

	//void drawImage(float x, float y, float w, float h, std::string fullPath);
	//	void drawBox(float x, float y, float z, float w = 1.0f, float h = 1.0f, float d = 1.0f, bool isFilled = true);
//	void drawBox(glm::mat4 transformationMatrix, bool isFilled = true);
//	void drawSphere(float xPos, float yPos, float zPos, int xSegmentNum = 64, int ySegmentNum = 64, float scale = 1.0f, bool isFilled = true);

private:
	void drawBackground();
	void resetColorAndBorder();
	void drawShapes();

	std::vector<std::vector<float>> shapes;
	std::deque<gImage*> images;

	int imageNum;
	//color
	float r, g, b, a;
	//frame of rectangle
	float thickness, borderposition;
	//boolean isFilled: initialize this as false
	bool isFilled;

};

#endif /* UI_GGUISURFACE_H_ */
