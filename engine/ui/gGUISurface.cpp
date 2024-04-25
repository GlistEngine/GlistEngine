/*
 * gGUISurface.cpp
 *
 *  Created on: 27 Jul 2022
 *      Author: Ezgi Lena Sonmez
 */

#include "gGUISurface.h"

#include "gBaseCanvas.h"
#include "gBaseApp.h"


gGUISurface::gGUISurface() {
	resetColorAndBorder();
	imageNum = 0;
	textnum = 0;
	totalh = 0;
	maxHeight = bottom;
	title = "Surface";
}
gGUISurface::~gGUISurface() {

}
void gGUISurface::resetColorAndBorder() {
	/*With this function, unless the user calls the chooseColor() function,
		 * a fixed color will be assigned to the shape itself, so now it should
		 * call the chooseColor repeatedly after each drawRectangle/drawLine/drawCircle
		 *
		 * as an alternative, if you don't want to reset it every time:
		 * If the user calls resetColorAndBorder with his hand,
		 * the color entered in the previous way continues with the next shape.
		 *
		 **/
	isFilled = false;
	thickness = 0.0f;
	borderposition = 0.0f;
}
//
void gGUISurface::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(width, height);
	gGUIScrollable::enableScrollbars(true, false);
}

void gGUISurface::drawContent() {
	drawBackground();
	drawShapes();
}

void gGUISurface::drawShapes() {
	for(int i = 0; i < shapes.size(); i++) {
		//rectangle = 0
		if((int)shapes[i][0] == SHAPE_RECTANGLE) {//for drawing RECTANGLE first index decides to the shape type
			renderer->setColor(gColor(shapes[i][9], shapes[i][10], shapes[i][11], shapes[i][12]));
			gDrawRectangle(shapes[i][1], shapes[i][2] - verticalscroll, shapes[i][3], shapes[i][4], (int)shapes[i][5]);
		}
		//circle = 1
		else if((int)shapes[i][0] == SHAPE_CIRCLE) {//for drawing CIRCLE first index decides to the shape type
			renderer->setColor(gColor(shapes[i][6], shapes[i][7], shapes[i][8], shapes[i][9]));
			gDrawCircle(shapes[i][1], shapes[i][2]  - verticalscroll, shapes[i][3], (int)shapes[i][4], shapes[i][5]);
		}
		//2d line = 2
		else if((int)shapes[i][0] == SHAPE_LINE) {//for drawing 2D LINE first index decides to the shape type
			renderer->setColor(gColor(shapes[i][5], shapes[i][6], shapes[i][7], shapes[i][8]));
			gDrawLine(shapes[i][1], shapes[i][2] - verticalscroll, shapes[i][3], shapes[i][4] - verticalscroll);
		}
		//arrow = 3
		else if((int)shapes[i][0] == SHAPE_ARROW) {//for drawing 3D LINE first index decides to the shape type
			renderer->setColor(gColor(shapes[i][7], shapes[i][8], shapes[i][9], shapes[i][10]));
			gDrawArrow(shapes[i][1], shapes[i][2] - verticalscroll, shapes[i][3], shapes[i][4], shapes[i][5], shapes[i][6]);
		}
		//triangle = 4
		else if((int)shapes[i][0] == SHAPE_TRIANGLE) {//for drawing 3D LINE first index decides to the shape type
			renderer->setColor(gColor(shapes[i][8], shapes[i][9], shapes[i][10], shapes[i][11]));
			gDrawTriangle(shapes[i][1], shapes[i][2] - verticalscroll, shapes[i][3], shapes[i][4] - verticalscroll, shapes[i][5], shapes[i][6] - verticalscroll, (int)shapes[i][7]);
		}
		//image = 5
		else if((int)shapes[i][0] == SHAPE_IMAGE) {//for drawing 3D LINE first index decides to the shape type
			renderer->setColor(gColor(1.0f,	1.0f, 1.0f, 1.0f));
			images[shapes[i][5]]->draw(shapes[i][1], shapes[i][2] - verticalscroll, shapes[i][3], shapes[i][4]);
		}
		//text = 6
		else if((int)shapes[i][0] == SHAPE_TEXT) {//for drawing 3D LINE first index decides to the shape type
			renderer->setColor(gColor(shapes[i][6], shapes[i][7], shapes[i][8], shapes[i][9]));
			root->getAppManager()->getGUIManager()->getFont(shapes[i][3], shapes[i][4])->drawText(texts[shapes[i][5]], shapes[i][1], shapes[i][2]);
		}
	}
}

void gGUISurface::drawBackground() {
	renderer->setColor(gColor(1.0f, 1.0f, 1.0f, 1.0f));//white
//	renderer->setColor(backgroundcolor);
	gDrawRectangle(0, 0, right, bottom, true);
	totalh = maxHeight;
}

void gGUISurface::setBorder(float thickness, float borderposition) {
	this->thickness = thickness;
	this->borderposition = borderposition;
}

void gGUISurface::addRectangle(float x, float y, float w, float h, bool isFilled, gColor color) {
	if(y >= maxHeight)
		maxHeight = y + h + 30;

	std::vector<float> newShape;
	newShape.push_back(SHAPE_RECTANGLE); //for drawing rectangle //shapes[i][0]
	newShape.push_back(x); //shapes[i][x] 1
	newShape.push_back(y); //shapes[i][y] 2
	newShape.push_back(w); //shapes[i][w] 3
	newShape.push_back(h); //shapes[i][h] 4
	newShape.push_back(isFilled); //to get whether the rectagnle is filled //shapes[i][5] 0/1 = false/true
	if(thickness != 0) {
		newShape.push_back(1); //shapes[i][1] 6 to understand that this rectangle has thickness and borderposition
		newShape.push_back(thickness); //shapes[i][thickness] 7
		newShape.push_back(borderposition); //shapes[i][borderposition] 8
	}
	else {
		newShape.push_back(0); //shapes[i][0] 6 to understand that this rectangle has thickness and borderposition
		newShape.push_back(0);
		newShape.push_back(0);
	}
	newShape.push_back(color.r); //shapes[i][r] 7 (if it does not have a thickness) or 9
	newShape.push_back(color.g); //shapes[i][g] 8 (if it does not have a thickness) or 10
	newShape.push_back(color.b); //shapes[i][b] 9 (if it does not have a thickness) or 11
	newShape.push_back(color.a); //shapes[i][a] 10 (if it does not have a thickness) or 12
	shapes.push_back(newShape);
	resetColorAndBorder();
}


void gGUISurface::addCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides, gColor color) {
	if(yCenter >= maxHeight)
		maxHeight = yCenter + radius + 30;
	std::vector<float> newShape;
	newShape.push_back(SHAPE_CIRCLE); //for drawing CIRCLE //shapes[i][0]
	newShape.push_back(xCenter); //shapes[i][xCenter] 1
	newShape.push_back(yCenter); //shapes[i][yCenter] 2
	newShape.push_back(radius); //shapes[i][radius] 3
	newShape.push_back(isFilled); //to get whether the circle is filled //shapes[i][4] 0/1 = false/true
	newShape.push_back(numberOfSides); //shapes[i][numberOfSides] 5
	newShape.push_back(color.r); //shapes[i][r] 6
	newShape.push_back(color.g); //shapes[i][g] 7
	newShape.push_back(color.b); //shapes[i][b] 8
	newShape.push_back(color.a); //shapes[i][a] 9

	shapes.push_back(newShape);
	resetColorAndBorder();
}

void gGUISurface::addLine(float x1, float y1, float x2, float y2, gColor color) {
	if(y1 >= maxHeight || y2 >= maxHeight) {
		if(y1 >= y2)
			maxHeight = y1 + 30;
		else
			maxHeight = y2 + 30;
	}

	std::vector<float> newShape;
	newShape.push_back(SHAPE_LINE); //for drawing 2D LINE //shapes[i][0]
	newShape.push_back(x1); //shapes[i][x1] 1
	newShape.push_back(y1); //shapes[i][y1] 2
	newShape.push_back(x2); //shapes[i][x2] 3
	newShape.push_back(y2); //shapes[i][y2] 4

	newShape.push_back(color.r); //shapes[i][r] 5
	newShape.push_back(color.g); //shapes[i][g] 6
	newShape.push_back(color.b); //shapes[i][b] 7
	newShape.push_back(color.a); //shapes[i][a] 8

	shapes.push_back(newShape);
	resetColorAndBorder();
}

void gGUISurface::addArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle, gColor color) {
	if(y1 >= maxHeight)
		maxHeight = y1 + length + tipLength;

	std::vector<float> newShape;
	newShape.push_back(SHAPE_ARROW); //for drawing ARROW //shapes[i][0]
	newShape.push_back(x1); //shapes[i][x1] 1
	newShape.push_back(y1); //shapes[i][y1] 2
	newShape.push_back(length); //shapes[i][length] 3
	newShape.push_back(angle); //shapes[i][angle] 4
	newShape.push_back(tipLength); //shapes[i][tipLength] 5
	newShape.push_back(tipAngle); //shapes[i][tipAngle] 6

	newShape.push_back(color.r); //shapes[i][r] 7
	newShape.push_back(color.g); //shapes[i][g] 8
	newShape.push_back(color.b); //shapes[i][b] 9
	newShape.push_back(color.a); //shapes[i][a] 10

	shapes.push_back(newShape);
	resetColorAndBorder();
}

void gGUISurface::addTriangle(float px, float py, float qx, float qy, float rx, float ry, bool isFilled, gColor color) {
	if(py >= maxHeight || qy >= maxHeight || ry >= maxHeight) {
		if(py >= qy && py >= ry)
			maxHeight = py + 30;
		else if(qy >= py && qy >= ry)
			maxHeight = qy + 30;
		else
			maxHeight = ry + 30;
	}

	std::vector<float> newShape;
	newShape.push_back(SHAPE_TRIANGLE); //for drawing TRIANGLE //shapes[i][0]
	newShape.push_back(px); //shapes[i][px] 1
	newShape.push_back(py); //shapes[i][py] 2
	newShape.push_back(qx); //shapes[i][qx] 3
	newShape.push_back(qy); //shapes[i][qy] 4
	newShape.push_back(rx); //shapes[i][rx] 5
	newShape.push_back(ry); //shapes[i][ry] 6
	newShape.push_back(isFilled); //to get whether the circle is filled //shapes[i][7] 0/1 = false/true

	newShape.push_back(color.r); //shapes[i][r] 8
	newShape.push_back(color.g); //shapes[i][g] 9
	newShape.push_back(color.b); //shapes[i][b] 10
	newShape.push_back(color.a); //shapes[i][a] 11

	shapes.push_back(newShape);
	resetColorAndBorder();
}

void gGUISurface::addImage(float x, float y, float w, float h, gImage* image) {
	if(y >= maxHeight)
		maxHeight = y + h + 30;

	std::vector<float> newShape;
	newShape.push_back(SHAPE_IMAGE); //for drawing IMAGE //shapes[i][0]
	newShape.push_back(x); //shapes[i][x] 1
	newShape.push_back(y); //shapes[i][y] 2
	newShape.push_back(w); //shapes[i][w] 3
	newShape.push_back(h); //shapes[i][h] 4
	newShape.push_back(imageNum);
	imageNum++;

	//paths.push_back(pathOfImage);
	images.push_back(image); //

	shapes.push_back(newShape);
	resetColorAndBorder();
}
void gGUISurface::addText(std::string text, float x, float y, int fontFace, int fontType, gColor color) {
	std::vector<float> newShape;
	newShape.push_back(SHAPE_TEXT); //for drawing IMAGE //shapes[i][0]
	newShape.push_back(x); //shapes[i][x] 1
	newShape.push_back(y); //shapes[i][y] 2
	newShape.push_back(fontFace); //shapes[i][fontFace] 3
	newShape.push_back(fontType); //shapes[i][h] 4
	newShape.push_back(textnum);
	textnum++;
	newShape.push_back(color.r); //shapes[i][r] 6
	newShape.push_back(color.g); //shapes[i][g] 7
	newShape.push_back(color.b); //shapes[i][b] 8
	newShape.push_back(color.a); //shapes[i][a] 9

	//paths.push_back(pathOfImage);
	texts.push_back(text); //

	shapes.push_back(newShape);
	resetColorAndBorder();
}

void gGUISurface::clear() {
	images.clear();
	texts.clear();
	shapes.clear();
	resetColorAndBorder();
	imageNum = 0;
	textnum = 0;
}

void gGUISurface::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	if(!isPointInsideVerticalScrollbar(x, y, true)) {
		mousepressedonlist = true;
	}
	root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_MOUSEPRESSED, gToStr(x), gToStr(y));
}

void gGUISurface::mouseReleased(int x, int y, int button) {
	gGUIScrollable::mouseReleased(x, y, button);
	if(mousepressedonlist) {
		mousepressedonlist = false;
		verticalscroll = y;
	}
	root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_MOUSERELEASED, gToStr(x), gToStr(y));
}

