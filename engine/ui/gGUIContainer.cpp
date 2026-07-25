/*
 * gGUIContainer.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUIContainer.h"

#include <algorithm>

// GUI coordinates are already normalized by gAppManager on mobile.  Six units
// was small enough for ordinary finger jitter to turn a tap into a scroll.  A
// twelve-unit slop keeps taps reliable without making an intentional drag feel
// delayed.  This applies only to containers which explicitly enable scrolling.
const int gGUIContainer::CONTENT_DRAG_SLOP = 12;


gGUIContainer::gGUIContainer() {
	iscontainer = true;
	topbarh = 0;
	contentwidth = 0;
	contentheight = 0;
	contentscrollingenabled = false;
	iscontentdragging = false;
	iscontentdragmoved = false;
	guisizer = nullptr;
	temporaryemptysizer.setSize(1, 1);
	setSizer(&temporaryemptysizer);
}

gGUIContainer::~gGUIContainer() {
}

void gGUIContainer::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	if(contentscrollingenabled) {
		totalw = contentwidth > 0 ? contentwidth : w;
		totalh = contentheight > 0 ? contentheight : h;
	} else {
		// Preserve the original container contract. Legacy desktop controls use
		// absolute sizer coordinates and must not be moved into an FBO-local space.
		totalh = h;
	}
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
	if(contentscrollingenabled) {
		// Scroll-enabled mobile containers need their viewport refreshed during
		// relayout. Legacy containers retain their original lightweight behavior.
		gGUIScrollable::setDimensions(w, h);
	}
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
	contentscrollingenabled = true;
	contentwidth = std::max(contentWidth, 1);
	contentheight = std::max(contentHeight, 1);
	totalw = contentwidth;
	totalh = contentheight;
}

void gGUIContainer::enableContentScrolling(bool enabled) {
	contentscrollingenabled = enabled;
	if(!enabled) {
		horizontalscroll = 0;
		verticalscroll = 0;
		iscontentdragging = false;
		iscontentdragmoved = false;
	}
}

bool gGUIContainer::isContentScrollingEnabled() const {
	return contentscrollingenabled;
}

void gGUIContainer::update() {
	if(guisizer) guisizer->update();
}

void gGUIContainer::draw() {
	if(contentscrollingenabled) {
		gGUIScrollable::draw();
	} else if(guisizer) {
		guisizer->draw();
	}
}

void gGUIContainer::drawContent() {
	if(!guisizer) return;
	// gGUIScrollable renders content into its local framebuffer.  Keep the
	// sizer in that local coordinate system and translate only this container's
	// children by its own scroll offsets.
	const int layoutwidth = isHorizontalScrollEnabled() ? totalw : boxw;
	guisizer->set(-horizontalscroll, topbarh - verticalscroll,
			std::max(0, layoutwidth), std::max(0, totalh - topbarh));
	guisizer->draw();
}

int gGUIContainer::getCursor(int x, int y) {
	if(!contentscrollingenabled) return guisizer->getCursor(x, y);
	// drawContent() has already positioned the child controls in viewport-local
	// coordinates. Convert screen to viewport coordinates exactly once here.
	return guisizer->getCursor(x - left, y - top);
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
	if(!contentscrollingenabled) {
		guisizer->mouseMoved(x, y);
		return;
	}
	guisizer->mouseMoved(x - left, y - top);
}

void gGUIContainer::mousePressed(int x, int y, int button) {
	if(!contentscrollingenabled) {
		guisizer->mousePressed(x, y, button);
		return;
	}
	gGUIScrollable::mousePressed(x, y, button);
	const bool onverticalbar = isVerticalScrollEnabled() && isPointInsideVerticalScrollbar(x, y, true);
	const bool onhorizontalbar = isHorizontalScrollEnabled() && isPointInsideHorizontalScrollbar(x, y, true);
	iscontentdragging = false;
	iscontentdragmoved = false;
	if(!onverticalbar && !onhorizontalbar) {
		iscontentdragging = hasVerticalOverflow() || hasHorizontalOverflow();
		contentdragstartx = x;
		contentdragstarty = y;
		contentdragscrollx = horizontalscroll;
		contentdragscrolly = verticalscroll;
		guisizer->mousePressed(x - left, y - top, button);
	}
}

void gGUIContainer::mouseDragged(int x, int y, int button) {
	if(!contentscrollingenabled) {
		guisizer->mouseDragged(x, y, button);
		return;
	}
	gGUIScrollable::mouseDragged(x, y, button);
	if(iscontentdragging) {
		const int dx = x - contentdragstartx;
		const int dy = y - contentdragstarty;
		const int scrolldx = isHorizontalScrollEnabled() ? dx : 0;
		const int scrolldy = isVerticalScrollEnabled() ? dy : 0;
		if(!iscontentdragmoved
				&& scrolldx * scrolldx + scrolldy * scrolldy
				> CONTENT_DRAG_SLOP * CONTENT_DRAG_SLOP) {
			iscontentdragmoved = true;
			// Cancel the child press as soon as this gesture becomes a drag. This
			// clears its visual/interaction state even if Android later reports the
			// release outside the viewport.
			guisizer->mouseReleased(-100000, -100000, button);
		}
		// Do not move content during the tap-candidate phase. Apart from matching
		// native scroll views, this keeps press and release hit coordinates equal.
		if(!iscontentdragmoved) return;
		horizontalscroll = isHorizontalScrollEnabled()
				? gClamp(contentdragscrollx - dx, 0, std::max(0, totalw - boxw)) : 0;
		verticalscroll = isVerticalScrollEnabled()
				? gClamp(contentdragscrolly - dy, 0, std::max(0, totalh - boxh)) : 0;
	} else if(!isdraggingverticalscroll && !isdragginghorizontalscroll) {
		guisizer->mouseDragged(x - left, y - top, button);
	}
}

void gGUIContainer::mouseReleased(int x, int y, int button) {
	if(!contentscrollingenabled) {
		guisizer->mouseReleased(x, y, button);
		return;
	}
	gGUIScrollable::mouseReleased(x, y, button);
	if(iscontentdragging && iscontentdragmoved) {
		// A drag started on a button must cancel its press instead of becoming a
		// click when the finger leaves the screen.
		guisizer->mouseReleased(-100000, -100000, button);
	} else {
		guisizer->mouseReleased(x - left, y - top, button);
	}
	iscontentdragging = false;
	iscontentdragmoved = false;
}

void gGUIContainer::mouseScrolled(int x, int y) {
	if(!contentscrollingenabled) {
		guisizer->mouseScrolled(x, y);
	} else if(hasVerticalOverflow() || hasHorizontalOverflow()) {
		gGUIScrollable::mouseScrolled(x, y);
	} else {
		guisizer->mouseScrolled(x, y);
	}
}

void gGUIContainer::windowResized(int w, int h) {
	if(contentscrollingenabled) gGUIScrollable::setDimensions(width, height);
	if(guisizer) guisizer->windowResized(w, h);
}

void gGUIContainer::setCursorOn(bool isOn) {
	gBaseGUIObject::setCursorOn(isOn);
	guisizer->setCursorOn(isOn);
}
