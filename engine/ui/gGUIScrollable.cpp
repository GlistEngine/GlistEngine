/*
 * gGUIScrollable.cpp
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#include "gGUIScrollable.h"
#include "gAppManager.h"


gGUIScrollable::gGUIScrollable() {
	boxw = width;
	boxh = height;
	totalw = boxw;
	totalh = boxh;
	scrollamount = 8;
	enableverticalscroll = false;
	enablehorizontalscroll = false;
	barbackgroundcolor = middlegroundcolor;
	barforegroundcolor = backgroundcolor;
	titlex = left;
	titley = top + font->getStringHeight("AE");
	titleheight = font->getSize() * 1.8f;
	boxfbo = new gFbo();
	setTitleOn(false);
	toolbarw = 0;
	toolbarh = 0;
}

gGUIScrollable::~gGUIScrollable() {
	delete boxfbo;
}

void gGUIScrollable::enableScrollbars(bool isVerticalEnabled, bool isHorizontalEnabled) {
	enableverticalscroll = isVerticalEnabled;
	enablehorizontalscroll = isHorizontalEnabled;
	setDimensions(width, height);
}

void gGUIScrollable::setDimensions(int newWidth, int newHeight) {
	height = newHeight;
	width = newWidth;

	boxw = width;
	if (enableverticalscroll) {
		boxw -= barsize;
	}
	boxw -= toolbarw;

	boxh = height;
	if (enablehorizontalscroll) {
		boxh -= barsize;
	}
	boxh -= toolbarh;

//	totalw = boxw;
//	totalh = boxh + barsize;

	titlex = left + font->getStringWidth("i");
	titley = top + font->getStringHeight("AE");

	boxfbo->allocate(renderer->getScreenWidth(), renderer->getScreenHeight());
}


void gGUIScrollable::updateScrollbar() {
	float deltat = appmanager->getElapsedTime();
	if (verticalscrollclickedtime > 0) {
		verticalscrollclickedtime -= deltat;
		if (verticalscrollclickedtime < 0) {
			verticalscrollclickedtime = 0;
		}
	}
	if (horizontalscrollclickedtime > 0) {
		horizontalscrollclickedtime -= deltat;
		if (horizontalscrollclickedtime < 0) {
			horizontalscrollclickedtime = 0;
		}
	}

	// update scroll bar
	// vertical bar
	int scrollableheight = totalh - boxh;
	if (scrollableheight > 0) {
		verticalscroll = gClamp(verticalscroll, 0, scrollableheight);
	} else {
		verticalscroll = 0;
	}

	scrollbarverticalsize = ((float) boxh / totalh) * boxh;
	if (scrollbarverticalsize < barsize) {
		scrollbarverticalsize = barsize;
	}
	if (scrollableheight > 0) {
		// Calculate the position of the scrollbar thumb within the viewport
		verticalscrollbarpos = ((float) verticalscroll / scrollableheight) * (boxh - scrollbarverticalsize);
	} else {
		verticalscrollbarpos = 0; // Set scrollbar position to the top if no scrolling is needed
	}

	// horizontal bar
	int scrollablewidth = totalw - boxw;
	if (scrollablewidth > 0) {
		horizontalscroll = gClamp(horizontalscroll, 0, scrollablewidth);
	} else {
		horizontalscroll = 0;
	}

	scrollbarhorizontalsize = ((float) boxw / totalw) * boxw;
	if (scrollbarhorizontalsize < barsize) {
		scrollbarhorizontalsize = barsize;
	}
	if (scrollablewidth > 0) {
		// Calculate the position of the scrollbar thumb within the viewport
		horizontalscrollbarpos = ((float) horizontalscroll / scrollablewidth) * (boxw - scrollbarhorizontalsize);
	} else {
		horizontalscrollbarpos = 0; // Set scrollbar position to the top if no scrolling is needed
	}
}

void gGUIScrollable::draw() {
	updateScrollbar();

	bool isalpha = renderer->isAlphaBlendingEnabled();
	bool isalphatest = renderer->isAlphaTestEnabled();
	if(isalpha) {
		renderer->disableAlphaBlending();
	}
	if (isalphatest) {
		renderer->disableAlphaTest();
	}
	renderer->setColor(fontcolor);
	if(istitleon) font->drawText(title, titlex, titley);
	renderer->setColor(0, 0, 0);
	boxfbo->bind();
	renderer->clearColor(0, 0, 0, 0);
	drawContent();
	drawScrollbars();
	boxfbo->unbind();
	renderer->setColor(255, 255, 255);
	boxfbo->drawSub(left, top + titleheight,
					width, height,
					0, renderer->unscaleY(renderer->getHeight() - height),
					renderer->unscaleX(width), renderer->unscaleY(height));
	renderer->setColor(foregroundcolor);
	gDrawRectangle(left, top + titleheight, width, height, false);
	if(isalpha) {
		renderer->enableAlphaBlending();
	}
	if (isalphatest) {
		renderer->enableAlphaTest();
	}
}

void gGUIScrollable::drawContent() {
//	gLogi("Listbox") << "l:" << left << ", t:" << top << ", w:" << boxw << ", h:" << boxh;
//	renderer->setColor(textbackgroundcolor);
//	gDrawRectangle(0, 0, boxw, boxh, true);
}

void gGUIScrollable::drawScrollbars() {
	// render
	gColor* oldcolor = renderer->getColor();
	if(enableverticalscroll) {
		renderer->setColor(&barbackgroundcolor);
		gDrawRectangle(boxw, toolbarh, barsize, boxh, true);

		renderer->setColor(&barforegroundcolor);
		gDrawRectangle(boxw, verticalscrollbarpos, barsize, scrollbarverticalsize, true);
	}

	if(enablehorizontalscroll) {
		renderer->setColor(&barbackgroundcolor);
		gDrawRectangle(toolbarw, boxh, boxw, barsize, true);

		renderer->setColor(&barforegroundcolor);
		gDrawRectangle(toolbarw + horizontalscrollbarpos, boxh, scrollbarhorizontalsize, barsize, true);
	}

	renderer->setColor(foregroundcolor);
	gDrawRectangle(boxw + toolbarw, boxh + toolbarh, barsize, barsize, true);

	// reset color back to before
	renderer->setColor(oldcolor);
}

void gGUIScrollable::mouseMoved(int x, int y) {
}

void gGUIScrollable::mousePressed(int x, int y, int button) {
	isdraggingverticalscroll = isPointInsideVerticalScrollbar(x, y);
	isdragginghorizontalscroll = isPointInsideHorizontalScrollbar(x, y);
	// double click behavior
	if (!isdragginghorizontalscroll && isPointInsideHorizontalScrollbar(x, y, true)  && horizontalscrollclickedtime > 0.2f) {
		verticalscrolldragstart = 0;
		isdragginghorizontalscroll = true;
		mouseDragged(x, y, button);
		isdragginghorizontalscroll = false;
	} else if (isdraggingverticalscroll) {
		verticalscrolldragstart = y;
	}
	horizontalscrollclickedtime = 0.4f;
	if (!isdraggingverticalscroll && isPointInsideVerticalScrollbar(x, y, true) && verticalscrollclickedtime > 0.2f) {
		horizontalscrolldragstart = 0;
		isdraggingverticalscroll = true;
		mouseDragged(x, y, button);
		isdraggingverticalscroll = false;
	} else if (isdragginghorizontalscroll) {
		horizontalscrolldragstart = x;
	}
	verticalscrollclickedtime = 0.4f;
}

void gGUIScrollable::mouseDragged(int x, int y, int button) {
	if(isdraggingverticalscroll && totalh > boxh) {
		int pos = y - verticalscrolldragstart;
		int diff = (float)pos / boxh * totalh;
		verticalscroll = gClamp(verticalscroll + diff, 0, totalh - boxh);
		verticalscrolldragstart = y;
	}
	if(isdragginghorizontalscroll && totalw > boxw) {
		int pos = x - horizontalscrolldragstart;
		int diff = (float)pos / boxw * totalw;
		horizontalscroll = gClamp(horizontalscroll + diff, 0, totalw - boxw);
		horizontalscrolldragstart = x;
	}
}

void gGUIScrollable::mouseReleased(int x, int y, int button) {
	isdraggingverticalscroll = false;
	isdragginghorizontalscroll = false;
}

void gGUIScrollable::mouseScrolled(int x, int y) {
	if(enableverticalscroll && totalh > boxh) {
		int diff = -y * scrollamount;
		verticalscroll = gClamp(verticalscroll + diff, 0, totalh - boxh);
		verticalscrolldragstart = y;
	}
	if(enablehorizontalscroll && totalw > boxw) {
		int diff = -x * scrollamount;
		horizontalscroll = gClamp(horizontalscroll + diff, 0, totalw - boxw);
		horizontalscrolldragstart = x;
	}
}

int gGUIScrollable::getVerticalScroll(){
	return verticalscroll;
}

void gGUIScrollable::windowResized(int w, int h) {
	delete boxfbo;
	boxfbo = new gFbo();
	setDimensions(width, height);
	gGUIControl::windowResized(w, h);
}

gFbo* gGUIScrollable::getFbo() {
	return boxfbo;
}

int gGUIScrollable::getTitleTop() {
	return titleheight;
}

void gGUIScrollable::setToolbarSpace(int toolbarW, int toolbarH) {
	toolbarw = toolbarW;
	toolbarh = toolbarH;
}

bool gGUIScrollable::isPointInsideVerticalScrollbar(int x, int y, bool checkFullSize) {
	int scrollbarsize = checkFullSize ? boxh : scrollbarverticalsize;
	int scrollbarpos = checkFullSize ? 0 : verticalscrollbarpos;
	int startx = left + boxw;
	int starty = top + scrollbarpos;
	int endx = startx + barsize;
	int endy = starty + scrollbarsize;

	return x >= startx && x < endx && y >= starty && y < endy;
}

bool gGUIScrollable::isPointInsideHorizontalScrollbar(int x, int y, bool checkFullSize) {
	int scrollbarsize = checkFullSize ? boxw : scrollbarhorizontalsize;
	int scrollbarpos = checkFullSize ? 0 : horizontalscrollbarpos;
	int startx = left + scrollbarpos;
	int starty = top + boxh;
	int endx = startx + scrollbarsize;
	int endy = starty + barsize;

	return x >= startx && x < endx && y >= starty && y < endy;
}
