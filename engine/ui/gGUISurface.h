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


class gGUISurface: public gGUIControl {
public:
	gGUISurface();
	virtual ~gGUISurface();
	void draw();
	void setColor(float r, float g, float b, float a);
	void setBorder(float thickness, float borderposition);
	void drawRectangle(float x, float y, float w, float h, bool isFilled = false);
	void drawLine(float x, float y, float x2, float y2);
	//void drawLine(float x, float y, float z1, float x2, float y2, float z2);
	void drawCircle(float xCenter, float yCenter, float radius, bool isFilled = false, float numberOfSides = 64.0f);
	void drawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle);
	void drawTriangle(float px, float py, float qx, float qy, float rx, float ry, bool isFilled = true);
//	void drawBox(float x, float y, float z, float w = 1.0f, float h = 1.0f, float d = 1.0f, bool isFilled = true);
//	void drawBox(glm::mat4 transformationMatrix, bool isFilled = true);
//	void drawSphere(float xPos, float yPos, float zPos, int xSegmentNum = 64, int ySegmentNum = 64, float scale = 1.0f, bool isFilled = true);


private:
	void drawBackground();
	void resetColorAndBorder();
	void drawShapes();

	std::vector<std::vector<float>> shapes;
	//color
	float r, g, b, a;
	//frame of rectangle
	float thickness, borderposition;
	//boolean isFilled: initialize this as false
	bool isFilled;
};

#endif /* UI_GGUISURFACE_H_ */
