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
	totalh = 0;
	maxHeight = bottom;
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
	renderer->setColor(gColor(0.0f, 0.0f, 0.0f, 0.0f));
	title = "Surface";
	font->drawText(title + ":", titlex, titley);

	drawBackground();
	drawShapes();
}

void gGUISurface::drawShapes() {
	for(int i = 0; i < shapes.size(); i++) {

		//rectangle = 0
		if((int) shapes[i][0] == 0) {//for drawing RECTANGLE first index decides to the shape type

			if((int) shapes[i][5] == 0) {//isFilled = false
				if((int) shapes[i][6] == 0) {//no thickness
					renderer->setColor(gColor(shapes[i][7], shapes[i][8], shapes[i][9], shapes[i][10]));
					gDrawRectangle(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4], false);
				}
				else
				{
					renderer->setColor(gColor(shapes[i][9], shapes[i][10], shapes[i][11], shapes[i][12]));
					gDrawRectangle(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4], false);
//					gDrawRectangle(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4], false, shapes[i][7], shapes[i][8]);
				}
			}
			else { //isFilled = true
				if((int) shapes[i][6] == 0) {//no thickness
					renderer->setColor(gColor(shapes[i][7], shapes[i][8], shapes[i][9], shapes[i][10]));
					gDrawRectangle(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4], true);
				}
				else
				{
					renderer->setColor(gColor(shapes[i][9], shapes[i][10], shapes[i][11], shapes[i][12]));
					gDrawRectangle(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4], true);
//					gDrawRectangle(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4], true, shapes[i][7], shapes[i][8]);
				}
			}
		}
		//circle = 1
		else if((int) shapes[i][0] == 1) {//for drawing CIRCLE first index decides to the shape type
			if((int) shapes[i][4] == 0) {//isFilled = false
				renderer->setColor(gColor(shapes[i][6], shapes[i][7], shapes[i][8], shapes[i][9]));
				gDrawCircle(shapes[i][1], shapes[i][2]  - firsty, shapes[i][3], false, shapes[i][5]);
			}
			else { //isFilled = true
				renderer->setColor(gColor(shapes[i][6], shapes[i][7], shapes[i][8], shapes[i][9]));
				gDrawCircle(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], true, shapes[i][5]);
			}
		}
		//2d line = 2
		else if((int) shapes[i][0] == 2) {//for drawing 2D LINE first index decides to the shape type
			renderer->setColor(gColor(shapes[i][5], shapes[i][6], shapes[i][7], shapes[i][8]));
			gDrawLine(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4] - firsty);
		}
		//arrow = 3
		else if((int) shapes[i][0] == 3) {//for drawing 3D LINE first index decides to the shape type
			renderer->setColor(gColor(shapes[i][7], shapes[i][8], shapes[i][9], shapes[i][10]));
			gDrawArrow(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4], shapes[i][5], shapes[i][6]);
		}
		//triangle = 4
		else if((int) shapes[i][0] == 4) {//for drawing 3D LINE first index decides to the shape type
			if((int) shapes[i][7] == 0) {//isFilled = false
				renderer->setColor(gColor(shapes[i][8], shapes[i][9], shapes[i][10], shapes[i][11]));
				gDrawTriangle(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4] - firsty, shapes[i][5], shapes[i][6] - firsty, false);
			}
			else { //isFilled = true
				renderer->setColor(gColor(shapes[i][8], shapes[i][9], shapes[i][10], shapes[i][11]));
				gDrawTriangle(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4] - firsty,shapes[i][5], shapes[i][6] - firsty, true);
			}
		}
		//image = 5
		else if((int) shapes[i][0] == 5) {//for drawing 3D LINE first index decides to the shape type
			//images[shapes[i][5]]->loadImage(paths[shapes[i][5]]);
			renderer->setColor(gColor(1.0f,	1.0f, 1.0f, 1.0f));
			images[shapes[i][5]]->draw(shapes[i][1], shapes[i][2] - firsty, shapes[i][3], shapes[i][4]);
			//images[1].draw(shapes[i+1][1] + left, shapes[i+1][2] + top, shapes[i+1][3], shapes[i+1][4]);
		}
	}
}

void gGUISurface::drawBackground() {
	renderer->setColor(gColor(1.0f, 	1.0f, 	1.0f, 	1.0f));//white
	gDrawRectangle(0, 0, right, maxHeight, true);
	totalh= maxHeight;
}

void gGUISurface::setBorder(float thickness, float borderposition) {
	this->thickness = thickness;
	this->borderposition = borderposition;
}

void gGUISurface::addRectangle(float x, float y, float w, float h, bool isFilled, gColor color) {
	if(y >= maxHeight)
		maxHeight = y + h + 30;

	std::vector<float> newShape;
	newShape.push_back(0); //for drawing rectangle //shapes[i][0]
	newShape.push_back(x); //shapes[i][x] 1
	newShape.push_back(y); //shapes[i][y] 2
	newShape.push_back(w); //shapes[i][w] 3
	newShape.push_back(h); //shapes[i][h] 4
	if(isFilled)
		newShape.push_back(1); //to get whether the rectagnle is filled //shapes[i][5] 0/1 = false/true
	else
		newShape.push_back(0); //to get whether the rectagnle is filled //shapes[i][5] 0/1 = false/true
	if(thickness != 0) {
		newShape.push_back(1); //shapes[i][1] 6 to understand that this rectangle has thickness and borderposition
		newShape.push_back(thickness); //shapes[i][thickness] 7
		newShape.push_back(borderposition); //shapes[i][borderposition] 8
	}
	else {
		newShape.push_back(0); //shapes[i][0] 6 to understand that this rectangle has thickness and borderposition
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
	newShape.push_back(1); //for drawing CIRCLE //shapes[i][0]
	newShape.push_back(xCenter); //shapes[i][xCenter] 1
	newShape.push_back(yCenter); //shapes[i][yCenter] 2
	newShape.push_back(radius); //shapes[i][radius] 3

	if(isFilled)
		newShape.push_back(1); //to get whether the circle is filled //shapes[i][4] 0/1 = false/true
	else
		newShape.push_back(0); //to get whether the circle is filled //shapes[i][4] 0/1 = false/true
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
	newShape.push_back(2); //for drawing 2D LINE //shapes[i][0]
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
	newShape.push_back(3); //for drawing ARROW //shapes[i][0]
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
	newShape.push_back(4); //for drawing TRIANGLE //shapes[i][0]
	newShape.push_back(px); //shapes[i][px] 1
	newShape.push_back(py); //shapes[i][py] 2
	newShape.push_back(qx); //shapes[i][qx] 3
	newShape.push_back(qy); //shapes[i][qy] 4
	newShape.push_back(rx); //shapes[i][rx] 5
	newShape.push_back(ry); //shapes[i][ry] 6

	if(isFilled)
		newShape.push_back(1); //to get whether the circle is filled //shapes[i][7] 0/1 = false/true
	else
		newShape.push_back(0); //to get whether the circle is filled //shapes[i][7] 0/1 = false/true

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
	newShape.push_back(5); //for drawing IMAGE //shapes[i][0]
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

void gGUISurface::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	if(x >= (boxw - vsbw) && x < vsbw && y >= 0 && y < vsbh) {
		mousepressedonlist = true;
	}
}

void gGUISurface::mouseReleased(int x, int y, int button) {
	gGUIScrollable::mouseReleased(x, y, button);
	if(mousepressedonlist) {
		mousepressedonlist = false;
		firsty = y;
	}
}

//void gGUISurface::drawLine(float x1, float y1, float z1, float x2, float y2, float z2) {
//	std::vector<float> newShape;
//	newShape.push_back(6); //for drawing 3D LINE //shapes[i][0]
//	newShape.push_back(x1); //shapes[i][x1] 1
//	newShape.push_back(y1); //shapes[i][y1] 2
//	newShape.push_back(z1); //shapes[i][z1] 3
//	newShape.push_back(x2); //shapes[i][x2] 4
//	newShape.push_back(y2); //shapes[i][y2] 5
//	newShape.push_back(z2); //shapes[i][z2] 6
//
//	newShape.push_back(r); //shapes[i][r] 7
//	newShape.push_back(g); //shapes[i][g] 8
//	newShape.push_back(b); //shapes[i][b] 9
//	newShape.push_back(a); //shapes[i][a] 10
//
//	shapes.push_back(newShape);
//	resetColorAndBorder();
//}

//void gGUISurface::drawBox(float x, float y, float z, float w, float h, float d, bool isFilled) {
//	this->x = x;
//	this->y = y;
//	this->z = z;
//	this->w = w;
//	this->h = h;
//	this->d = d;
//	this->isFilled = isFilled;
//	renderer->setColor(gColor(r, g, b, a));
//	gDrawBox(x + left, y + top, z, w, h, d, isFilled);
//}
////void gGUISurface::drawBox(glm::mat4 transformationMatrix, bool isFilled = true);
//void gGUISurface::drawSphere(float xPos, float yPos, float zPos, int xSegmentNum, int ySegmentNum, float scale, bool isFilled) {
//	this->xPos = xPos;
//	this->yPos = yPos;
//	this->zPos = zPos;
//	this->isFilled = isFilled;
//	renderer->setColor(gColor(r, g, b, a));
//	gDrawSphere(xPos + left, yPos + top, zPos, xSegmentNum, ySegmentNum, scale, isFilled);
//}

