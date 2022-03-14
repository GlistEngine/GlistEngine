/*
 * gGUIScrollable.cpp
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#include "gGUIScrollable.h"


gGUIScrollable::gGUIScrollable() {
	vsbw = 8;
	vsbh = height;
	hsbw = width;
	hsbh = 8;
	vsbx = right - vsbw;
	vsby = top;
	hsbx = left;
	hsby = bottom - hsbh;
	boxw = width;
	boxh = height;
	totalh = boxh;
	scrolldiff = 8;
	vsbenabled = false;
	hsbenabled = false;
	sbenabled = false;
	vsbactive = true;
	hsbactive = true;
	iscursoronvsb = false;
	iscursoronhsb = false;
	sbbgcolor = middlegroundcolor;
	sbfgcolor = backgroundcolor;
//	sbfgcolor = gColor(0.1f, 0.45f, 0.87f);
	sbdragcolor.set(0, 0, 255, 255);
	vsbalpha = 1.0f;
	hsbalpha = 1.0f;
	sbalphaboxlimit = 1.0f;
//	sbalphaboxlimit = 0.2f;
	sbalphasblimit = 1.0f;
	alphablending = false;
	vrw = vsbw;
	vrh = vsbh;
	vrx = vsbx;
	vry = vsby;
	hrw = hsbw;
	hrh = hsbh;
	hrx = hsbx;
	hry = hsby;
	firstx = 0;
	firsty = 0;
	vsbmy = -1;
	boxfbo = new gFbo();
}

gGUIScrollable::~gGUIScrollable() {
	delete boxfbo;
}

void gGUIScrollable::enableScrollbars(bool isVerticalEnabled, bool isHorizontalEnabled) {
	vsbenabled = isVerticalEnabled;
	hsbenabled = isHorizontalEnabled;
	if(vsbenabled || hsbenabled) sbenabled = true;
	else sbenabled = false;
}

void gGUIScrollable::setDimensions(int width, int height) {
	boxw = width;
	boxh = height;
	vsbh = height;
	hsbw = width;
	vsbx = left + boxw - vsbw;
	vsby = top;
	hsbx = left;
	hsby = top + boxh - hsbh;

	vrw = vsbw;
	vrh = vsbh * boxh / totalh;
	vrx = vsbx;
	vry = vsby;
	hrw = hsbw;
	hrh = hsbh;
	hrx = hsbx;
	hry = hsby;

	boxfbo->allocate(boxw, boxh);
}

void gGUIScrollable::draw() {
	boxfbo->bind();
	renderer->clearColor(0, 0, 0, 0);
	drawContent();
	drawScrollbars();
	boxfbo->unbind();
	renderer->setColor(255, 255, 255);
	boxfbo->draw(left, top);
}

void gGUIScrollable::drawContent() {
//	gLogi("Listbox") << "l:" << left << ", t:" << top << ", w:" << boxw << ", h:" << boxh;
	renderer->setColor(255, 255, 255);
	gDrawRectangle(left, top, boxw, boxh, true);
}

void gGUIScrollable::drawScrollbars() {
	if(sbenabled) {
		gColor* oldcolor = renderer->getColor();
		alphablending = renderer->isAlphaBlendingEnabled();
		if(!alphablending) {
//			renderer->enableAlphaBlending();
//			renderer->enableAlphaTest();
		}

		if(vsbenabled) {
/*
			vsbalpha += 0.01f;
			if(!iscursoronvsb) {
				if(vsbalpha > sbalphaboxlimit) vsbalpha = sbalphaboxlimit;
			} else {
				vsbalpha += 0.04f;
				if(vsbalpha > sbalphasblimit) vsbalpha = sbalphasblimit;
			}
*/
//			sbbgcolor.a = vsbalpha;
			renderer->setColor(&sbbgcolor);
			gDrawRectangle(left + boxw - vsbw, top, vsbw, vsbh, true);

//			sbfgcolor.a = vsbalpha;
			renderer->setColor(&sbfgcolor);
			gDrawRectangle(vrx, vry, vrw, vrh, true);
		}

		if(hsbenabled) {
/*
			hsbalpha += 0.01f;
			if(!iscursoronhsb) {
				if(hsbalpha > sbalphaboxlimit) hsbalpha = sbalphaboxlimit;
			} else {
				hsbalpha += 0.04f;
				if(hsbalpha > sbalphasblimit) hsbalpha = sbalphasblimit;
			}
*/
			sbbgcolor.a = hsbalpha;
			renderer->setColor(&sbbgcolor);
			gDrawRectangle(left, top + boxh - hsbh, hsbw, hsbh, true);

			sbfgcolor.a = hsbalpha;
			renderer->setColor(&sbfgcolor);
			gDrawRectangle(hrx, hry, hrw, hrh, true);
		}

		if(!alphablending) {
//			renderer->disableAlphaTest();
//			renderer->disableAlphaBlending();
		}
		renderer->setColor(oldcolor);
	} else {
//		vsbalpha = 0.0f;
//		hsbalpha = 0.0f;
	}
}

void gGUIScrollable::mouseMoved(int x, int y) {
	iscursoronvsb = false;
	iscursoronhsb = false;
	if(x >= vsbx && x < vsbx + vsbw && y >= vsby && y < vsby + vsbh) iscursoronvsb = true;
	if(x >= hsbx && x < hsbx + hsbw && y >= hsby && y < hsby + hsbh) iscursoronhsb = true;
}

void gGUIScrollable::mousePressed(int x, int y, int button) {
	if(vsbenabled && x >= vrx && x < vrx + vrw && y >= vry && y < vry + vrh) {
		vsbmy = y;
	}
}

void gGUIScrollable::mouseDragged(int x, int y, int button) {
	if(vsbmy > -1) {
		vry += y - vsbmy;
		if(vry < top) vry = top;
		if(vry > top + boxh - vrh) vry = top + boxh - vrh;

		firsty += y - vsbmy;
		if(firsty < 0) firsty = 0;
		if(firsty > totalh - boxh) firsty = totalh - boxh;

		vsbmy = y;
	}
}

void gGUIScrollable::mouseReleased(int x, int y, int button) {
	vsbmy = -1;
}

void gGUIScrollable::mouseScrolled(int x, int y) {
	firsty -= y * scrolldiff;
	if(firsty < 0) firsty = 0;
	if(firsty > totalh - boxh) firsty = totalh - boxh;
	if(vsbenabled) vry = top + firsty * boxh / totalh;

	firstx -= x * scrolldiff;
	if(firstx < 0) firstx = 0;
//	gLogi("Scrollable") << "t:" << top << ", y:" << vry << ", h:" << vrh;
}


