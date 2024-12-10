/*
 * gGUISurface.h
 *
 *  Created on: 27 Jul 2022
 *      Author: Ezgi Lena Sonmez
 */

#ifndef UI_GGUISURFACE_H_
#define UI_GGUISURFACE_H_

#include "gGUIScrollable.h"
#include "gRenderer.h"
#include "gImage.h"
#include "gGUISizer.h"
#include <deque>


class gGUISurface: public gGUIScrollable {
public:
	enum {
		SHAPE_RECTANGLE, SHAPE_CIRCLE, SHAPE_LINE, SHAPE_ARROW, SHAPE_TRIANGLE,
		SHAPE_IMAGE, SHAPE_TEXT, SHAPE_GUISIZER
	};
	gGUISurface();
	virtual ~gGUISurface();
	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void drawContent();

	void setBorder(float thickness, float borderposition);
	int addRectangle(float x, float y, float w, float h, bool isFilled, gColor color = gColor(0.0f, 0.0f, 0.0f));
	//void addRectangle(float x, float y, float w, float h, bool isFilled, float thickness, float borderposition, gColor color = gColor(0.0f, 0.0f, 0.0f));
	int addLine(float x, float y, float x2, float y2, gColor color = gColor(0.0f, 0.0f, 0.0f));
	//void drawLine(float x1, float y1, float z1, float x2, float y2, float z2);
	int addCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides = 64.0f, gColor color = gColor(0.0f, 0.0f, 0.0f));
	int addArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle, gColor color = gColor(0.0f, 0.0f, 0.0f));
	int addTriangle(float px, float py, float qx, float qy, float rx, float ry, bool isFilled, gColor color = gColor(0.0f, 0.0f, 0.0f));
	int addImage(float x, float y, float w, float h, gImage* image);
	int addText(std::string text, float x, float y, int fontFace, int fontType, gColor color = gColor(0.0f, 0.0f, 0.0f));
	int addSizer(float x, float y, float w, float h, gGUISizer* newSizer);

	std::vector<float> getShape(int shapeNo);
	void setText(int shapeNo, std::string text, float x, float y, int fontFace, int fontType, gColor color = gColor(0.0f, 0.0f, 0.0f));
	void setImage(int shapeNo, float x, float y, float w, float h, gImage* image);

	void clear();

	//void drawImage(float x, float y, float w, float h, std::string fullPath);
	//	void drawBox(float x, float y, float z, float w = 1.0f, float h = 1.0f, float d = 1.0f, bool isFilled = true);
//	void drawBox(glm::mat4 transformationMatrix, bool isFilled = true);
//	void drawSphere(float xPos, float yPos, float zPos, int xSegmentNum = 64, int ySegmentNum = 64, float scale = 1.0f, bool isFilled = true);

private:
	void drawBackground();
	void resetColorAndBorder();
	void drawShapes();

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

	std::vector<std::vector<float>> shapes;
	std::deque<gImage*> images;
	std::deque<std::string> texts;
	std::deque<gGUISizer*> sizers;

	int imageNum, textnum, sizernum;
	int maxHeight;
	//color
	float r, g, b, a;
	//frame of rectangle
	float thickness, borderposition;
	//boolean isFilled: initialize this as false
	bool isFilled, mousepressedonlist;

};

#endif /* UI_GGUISURFACE_H_ */
