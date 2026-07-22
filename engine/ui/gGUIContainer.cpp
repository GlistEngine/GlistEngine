/*
 * gGUIContainer.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIContainer.h"

#include <algorithm>


gGUIContainer::gGUIContainer() {
	iscontainer = true;
	topbarh = 0;
	contentwidth = 0;
	contentheight = 0;
	iscontentdragging = false;
	iscontentdragmoved = false;
	guisizer = nullptr;
	temporaryemptysizer.setSize(1, 1);
	setSizer(&temporaryemptysizer);
}

gGUIContainer::~gGUIContainer() {
}

void gGUIContainer::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalw = contentwidth > 0 ? contentwidth : w;
	totalh = contentheight > 0 ? contentheight : h;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(w, h);
	guisizer->set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y + topbarh, w, h - topbarh);
}

void gGUIContainer::set(int x, int y, int w, int h) {
	left = x;
	top = y;
	right = x + w;
	bottom = y + h;
	width = w;
	height = h;
	// Parent sizers use this lightweight overload during relayout.  Refresh the
	// scroll viewport here too, so rotation cannot leave stale scroll bounds.
	gGUIScrollable::setDimensions(w, h);
	guisizer->set(x, y + topbarh, w, h - topbarh);
}

void gGUIContainer::setSizer(gGUISizer* guiSizer) {
	guisizer = guiSizer;
	guisizer->setTopParent(topparent);
	guisizer->setParent(this);
	guisizer->setParentSlotNo(0, 0);
	guisizer->setRootApp(root);
//	guisizer->set(left, top, width, height);
	guisizer->left = left;
	guisizer->top = top + topbarh;
	guisizer->right = right;
	guisizer->bottom = bottom;
	guisizer->width = width;
	guisizer->height = height - topbarh;
}

gGUISizer* gGUIContainer::getSizer() {
	return guisizer;
}

void gGUIContainer::setContentSize(int contentWidth, int contentHeight) {
	contentwidth = std::max(contentWidth, 1);
	contentheight = std::max(contentHeight, 1);
	totalw = contentwidth;
	totalh = contentheight;
}

void gGUIContainer::update() {
	if(guisizer) guisizer->update();
}

void gGUIContainer::draw() {
	gGUIScrollable::draw();
}

void gGUIContainer::drawContent() {
	if(!guisizer) return;
	// gGUIScrollable renders content into its local framebuffer.  Keep the
	// sizer in that local coordinate system and translate only this container's
	// children by its own scroll offsets.
	const int layoutwidth = isHorizontalScrollEnabled() ? totalw : boxw;
	guisizer->set(-horizontalscroll, topbarh - verticalscroll, layoutwidth, totalh - topbarh);
	guisizer->draw();
}

int gGUIContainer::getCursor(int x, int y) {
	return guisizer->getCursor(x - left + horizontalscroll, y - top - topbarh + verticalscroll);
}

void gGUIContainer::keyPressed(int key) {
	guisizer->keyPressed(key);
}

void gGUIContainer::keyReleased(int key) {
	guisizer->keyReleased(key);
}

void gGUIContainer::charPressed(unsigned int codepoint) {
	guisizer->charPressed(codepoint);
}

void gGUIContainer::mouseMoved(int x, int y) {
	guisizer->mouseMoved(x - left + horizontalscroll, y - top - topbarh + verticalscroll);
}

void gGUIContainer::mousePressed(int x, int y, int button) {
	gGUIScrollable::mousePressed(x, y, button);
	const bool onverticalbar = isVerticalScrollEnabled() && isPointInsideVerticalScrollbar(x, y, true);
	const bool onhorizontalbar = isHorizontalScrollEnabled() && isPointInsideHorizontalScrollbar(x, y, true);
	if(!onverticalbar && !onhorizontalbar) {
		iscontentdragging = true;
		iscontentdragmoved = false;
		contentdragstartx = x;
		contentdragstarty = y;
		contentdragscrollx = horizontalscroll;
		contentdragscrolly = verticalscroll;
		guisizer->mousePressed(x - left + horizontalscroll, y - top - topbarh + verticalscroll, button);
	}
}

void gGUIContainer::mouseDragged(int x, int y, int button) {
	gGUIScrollable::mouseDragged(x, y, button);
	if(iscontentdragging) {
		const int dx = x - contentdragstartx;
		const int dy = y - contentdragstarty;
		const int scrolldx = isHorizontalScrollEnabled() ? dx : 0;
		const int scrolldy = isVerticalScrollEnabled() ? dy : 0;
		if(scrolldx * scrolldx + scrolldy * scrolldy > 36) iscontentdragmoved = true;
		horizontalscroll = isHorizontalScrollEnabled()
				? gClamp(contentdragscrollx - dx, 0, std::max(0, totalw - boxw)) : 0;
		verticalscroll = isVerticalScrollEnabled()
				? gClamp(contentdragscrolly - dy, 0, std::max(0, totalh - boxh)) : 0;
	} else if(!isdraggingverticalscroll && !isdragginghorizontalscroll) {
		guisizer->mouseDragged(x - left + horizontalscroll, y - top - topbarh + verticalscroll, button);
	}
}

void gGUIContainer::mouseReleased(int x, int y, int button) {
	gGUIScrollable::mouseReleased(x, y, button);
	if(iscontentdragging && iscontentdragmoved) {
		// A drag started on a button must cancel its press instead of becoming a
		// click when the finger leaves the screen.
		guisizer->mouseReleased(-100000, -100000, button);
	} else {
		guisizer->mouseReleased(x - left + horizontalscroll, y - top - topbarh + verticalscroll, button);
	}
	iscontentdragging = false;
	iscontentdragmoved = false;
}

void gGUIContainer::mouseScrolled(int x, int y) {
	gGUIScrollable::mouseScrolled(x, y);
}

void gGUIContainer::windowResized(int w, int h) {
	gGUIScrollable::setDimensions(width, height);
	if(guisizer) guisizer->windowResized(w, h);
}

void gGUIContainer::setCursorOn(bool isOn) {
	gBaseGUIObject::setCursorOn(isOn);
	guisizer->setCursorOn(isOn);
}
