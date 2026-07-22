/*
 * gGUIScrollable.cpp
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#include "gGUIScrollable.h"
#include "gAppManager.h"

#include <algorithm>


gGUIScrollable::gGUIScrollable() {
	boxw = width;
	boxh = height;
	totalw = boxw;
	totalh = boxh;
	scrollamount = 8;
	enableverticalscroll = false;
	enablehorizontalscroll = false;
	#if defined(ANDROID) || defined(IOS)
	barbackgroundcolor = backgroundcolor;
	barforegroundcolor = middlegroundcolor;
	#else
	// Keep the established desktop theme for existing applications.
	barbackgroundcolor = middlegroundcolor;
	barforegroundcolor = backgroundcolor;
	#endif
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
	boxw = std::max(0, boxw);

	boxh = height;
	if (enablehorizontalscroll) {
		boxh -= barsize;
	}
	boxh -= toolbarh;
	boxh = std::max(0, boxh);

//	totalw = boxw;
//	totalh = boxh + barsize;

	titlex = left + font->getStringWidth("i");
	titley = top + font->getStringHeight("AE");

	const int screenwidth = renderer->getScreenWidth();
	const int screenheight = renderer->getScreenHeight();
	if(screenwidth > 0 && screenheight > 0
			&& (screenwidth != boxfbo->getWidth() || screenheight != boxfbo->getHeight())) {
		boxfbo->allocate(screenwidth, screenheight);
	}
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
	int scrollableheight = enableverticalscroll ? totalh - boxh : 0;
	if (enableverticalscroll && scrollableheight > 0) {
		verticalscroll = gClamp(verticalscroll, 0, scrollableheight);
	} else {
		verticalscroll = 0;
	}

	scrollbarverticalsize = totalh > 0 ? ((float) boxh / totalh) * boxh : boxh;
	scrollbarverticalsize = gClamp(scrollbarverticalsize, std::min(barsize, boxh), boxh);
	if (scrollableheight > 0) {
		// Calculate the position of the scrollbar thumb within the viewport
		verticalscrollbarpos = ((float) verticalscroll / scrollableheight) * (boxh - scrollbarverticalsize);
	} else {
		verticalscrollbarpos = 0; // Set scrollbar position to the top if no scrolling is needed
	}
	verticalscrollbarpos = gClamp(verticalscrollbarpos, 0, std::max(0, boxh - scrollbarverticalsize));

	// horizontal bar
	int scrollablewidth = enablehorizontalscroll ? totalw - boxw : 0;
	if (enablehorizontalscroll && scrollablewidth > 0) {
		horizontalscroll = gClamp(horizontalscroll, 0, scrollablewidth);
	} else {
		horizontalscroll = 0;
	}

	scrollbarhorizontalsize = totalw > 0 ? ((float) boxw / totalw) * boxw : boxw;
	scrollbarhorizontalsize = gClamp(scrollbarhorizontalsize, std::min(barsize, boxw), boxw);
	if (scrollablewidth > 0) {
		// Calculate the position of the scrollbar thumb within the viewport
		horizontalscrollbarpos = ((float) horizontalscroll / scrollablewidth) * (boxw - scrollbarhorizontalsize);
	} else {
		horizontalscrollbarpos = 0; // Set scrollbar position to the top if no scrolling is needed
	}
	horizontalscrollbarpos = gClamp(horizontalscrollbarpos, 0, std::max(0, boxw - scrollbarhorizontalsize));
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
	const bool drawvertical = hasVerticalOverflow();
	const bool drawhorizontal = hasHorizontalOverflow();
	if(drawvertical) {
		renderer->setColor(&barbackgroundcolor);
		gDrawRectangle(boxw, toolbarh, barsize, boxh, true);

		renderer->setColor(&barforegroundcolor);
		gDrawRectangle(boxw, verticalscrollbarpos, barsize, scrollbarverticalsize, true);
	}

	if(drawhorizontal) {
		renderer->setColor(&barbackgroundcolor);
		gDrawRectangle(toolbarw, boxh, boxw, barsize, true);

		renderer->setColor(&barforegroundcolor);
		gDrawRectangle(toolbarw + horizontalscrollbarpos, boxh, scrollbarhorizontalsize, barsize, true);
	}

	if(drawvertical && drawhorizontal) {
		renderer->setColor(foregroundcolor);
		gDrawRectangle(boxw + toolbarw, boxh + toolbarh, barsize, barsize, true);
	}

	// reset color back to before
	renderer->setColor(oldcolor);
}

void gGUIScrollable::mouseMoved(int x, int y) {
}

void gGUIScrollable::mousePressed(int x, int y, int button) {
	isdraggingverticalscroll = hasVerticalOverflow() && isPointInsideVerticalScrollbar(x, y);
	isdragginghorizontalscroll = hasHorizontalOverflow() && isPointInsideHorizontalScrollbar(x, y);
	// double click behavior
	if (hasHorizontalOverflow() && !isdragginghorizontalscroll && isPointInsideHorizontalScrollbar(x, y, true)  && horizontalscrollclickedtime > 0.2f) {
		horizontalscrolldragstart = 0;
		isdragginghorizontalscroll = true;
		mouseDragged(x, y, button);
		isdragginghorizontalscroll = false;
	} else if (isdraggingverticalscroll) {
		verticalscrolldragstart = y;
	}
	horizontalscrollclickedtime = 0.4f;
	if (hasVerticalOverflow() && !isdraggingverticalscroll && isPointInsideVerticalScrollbar(x, y, true) && verticalscrollclickedtime > 0.2f) {
		verticalscrolldragstart = 0;
		isdraggingverticalscroll = true;
		mouseDragged(x, y, button);
		isdraggingverticalscroll = false;
	} else if (isdragginghorizontalscroll) {
		horizontalscrolldragstart = x;
	}
	verticalscrollclickedtime = 0.4f;
}

void gGUIScrollable::mouseDragged(int x, int y, int button) {
	if(isdraggingverticalscroll && boxh > 0 && totalh > boxh) {
		int pos = y - verticalscrolldragstart;
		int diff = (float)pos / boxh * totalh;
		verticalscroll = gClamp(verticalscroll + diff, 0, totalh - boxh);
		verticalscrolldragstart = y;
	}
	if(isdragginghorizontalscroll && boxw > 0 && totalw > boxw) {
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
	setDimensions(width, height);
	gGUIControl::windowResized(w, h);
}

gFbo* gGUIScrollable::getFbo() {
	return boxfbo;
}

int gGUIScrollable::getTitleTop() {
	return titleheight;
}

bool gGUIScrollable::isVerticalScrollEnabled() const {
	return enableverticalscroll;
}

bool gGUIScrollable::isHorizontalScrollEnabled() const {
	return enablehorizontalscroll;
}

bool gGUIScrollable::hasVerticalOverflow() const {
	return enableverticalscroll && totalh > boxh;
}

bool gGUIScrollable::hasHorizontalOverflow() const {
	return enablehorizontalscroll && totalw > boxw;
}

void gGUIScrollable::setToolbarSpace(int toolbarW, int toolbarH) {
	toolbarw = toolbarW;
	toolbarh = toolbarH;
}

bool gGUIScrollable::isPointInsideVerticalScrollbar(int x, int y, bool checkFullSize) {
	if(!hasVerticalOverflow()) return false;
	int scrollbarsize = checkFullSize ? boxh : scrollbarverticalsize;
	int scrollbarpos = checkFullSize ? 0 : verticalscrollbarpos;
	int startx = left + boxw;
	int starty = top + scrollbarpos;
	int endx = startx + barsize;
	int endy = starty + scrollbarsize;

	return x >= startx && x < endx && y >= starty && y < endy;
}

bool gGUIScrollable::isPointInsideHorizontalScrollbar(int x, int y, bool checkFullSize) {
	if(!hasHorizontalOverflow()) return false;
	int scrollbarsize = checkFullSize ? boxw : scrollbarhorizontalsize;
	int scrollbarpos = checkFullSize ? 0 : horizontalscrollbarpos;
	int startx = left + scrollbarpos;
	int starty = top + boxh;
	int endx = startx + scrollbarsize;
	int endy = starty + barsize;

	return x >= startx && x < endx && y >= starty && y < endy;
}
