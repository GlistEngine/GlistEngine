/*
 * gGUIScrollable.cpp
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#include "gGUIScrollable.h"
#include "gAppManager.h"

#if GLIST_ANDROID || GLIST_IOS
#include <cmath>

gGUIScrollable* gGUIScrollable::contentdragowner = nullptr;
gGUIScrollable* gGUIScrollable::contentflingowner = nullptr;
gGUIScrollable* gGUIScrollable::contentoverscrollowner = nullptr;

namespace {
// How far the finger has to travel before the drag counts as scrolling the
// content rather than pressing what is under it. Deliberately below the page's
// own threshold in gAppManager, so that a control gets the chance to claim the
// gesture before the page starts considering it.
const int contentdragthreshold = 6;

// The content coasts on the same terms as the page does, so that a list and the
// page it sits on feel like one surface. See the matching block in gAppManager
// for what each of these means.
constexpr float contentflingdecayrate = 3.0f;
constexpr float contentflingminstartspeed = 60.0f;
constexpr float contentflingstopspeed = 20.0f;
constexpr float contentflingmaxstep = 0.1f;
constexpr float contentvelocitymininterval = 0.001f;
constexpr float contentvelocitymaxinterval = 0.2f;
constexpr float contentvelocitysmoothing = 0.7f;

// The rubber band follows the page's model too, but stretches less: a control is
// a small window and a long pull inside it looks like the drawing has come loose.
// See the matching block in gAppManager for what each of these means.
constexpr float contentoverscrollmax = 55.0f;
constexpr float contentoverscrollreturnrate = 12.0f;
constexpr float contentoverscrollstopdistance = 0.5f;
constexpr float contentoverscrollbouncetime = 0.12f;

float resistContentOverscroll(float distance) {
	float magnitude = std::fabs(distance);
	float resisted = contentoverscrollmax * magnitude / (magnitude + contentoverscrollmax);
	return distance < 0.0f ? -resisted : resisted;
}
}

void gGUIScrollable::enableContentOverscroll(bool isEnabled) {
	iscontentoverscrollenabled = isEnabled;
	if(!isEnabled) setContentOverscroll(0.0f, 0.0f);
}

bool gGUIScrollable::isContentOverscrollEnabled() const {
	return iscontentoverscrollenabled;
}

void gGUIScrollable::setContentOverscroll(float overscrollX, float overscrollY) {
	contentoverscrollx = overscrollX;
	contentoverscrolly = overscrollY;
	if(overscrollX == 0.0f && overscrollY == 0.0f) {
		if(contentoverscrollowner == this) contentoverscrollowner = nullptr;
		return;
	}
	// Taking ownership drops whatever was stretched before. Two controls cannot be
	// pulled at once, and a band left behind by an earlier gesture would sit there
	// stretched with nothing to spring it back.
	if(contentoverscrollowner && contentoverscrollowner != this) {
		contentoverscrollowner->contentoverscrollx = 0.0f;
		contentoverscrollowner->contentoverscrolly = 0.0f;
	}
	contentoverscrollowner = this;
}

bool gGUIScrollable::isContentDragActive() {
	return contentdragowner != nullptr;
}

void gGUIScrollable::cancelContentFling() {
	contentflingowner = nullptr;
	// A stretch left mid spring goes with it: either a finger has landed and is
	// about to set its own, or the gesture was withdrawn and there is nothing left
	// to spring back from.
	if(contentoverscrollowner) contentoverscrollowner->setContentOverscroll(0.0f, 0.0f);
}

void gGUIScrollable::trackContentVelocity() {
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	if(hascontentdragvelocity) {
		float interval = std::chrono::duration<float>(now - lastcontentdragtime).count();
		if(interval > contentvelocitymininterval && interval < contentvelocitymaxinterval) {
			// Measured from the scroll that actually took effect rather than from
			// the finger, so a drag already pinned against an end builds up no
			// speed and cannot fling away from it once released.
			float instantx = (float)(getTouchScrollX() - lastcontentdraghorizontalscroll) / interval;
			float instanty = (float)(getTouchScrollY() - lastcontentdragverticalscroll) / interval;
			contentflingvelocityx = contentflingvelocityx * (1.0f - contentvelocitysmoothing) + instantx * contentvelocitysmoothing;
			contentflingvelocityy = contentflingvelocityy * (1.0f - contentvelocitysmoothing) + instanty * contentvelocitysmoothing;
		}
	}
	lastcontentdragtime = now;
	lastcontentdraghorizontalscroll = getTouchScrollX();
	lastcontentdragverticalscroll = getTouchScrollY();
	hascontentdragvelocity = true;
}

void gGUIScrollable::updateContentOverscroll() {
	gGUIScrollable* control = contentoverscrollowner;
	if(!control) return;
	// A finger still dragging is holding the stretch out on purpose, and sets it
	// itself every move.
	if(contentdragowner == control && control->iscontentdragging) return;
	float step = (float)appmanager->getElapsedTime();
	if(step <= 0.0f) return;
	if(step > contentflingmaxstep) step = contentflingmaxstep;

	float decay = std::exp(-contentoverscrollreturnrate * step);
	float newx = control->contentoverscrollx * decay;
	float newy = control->contentoverscrolly * decay;
	if(std::fabs(newx) < contentoverscrollstopdistance) newx = 0.0f;
	if(std::fabs(newy) < contentoverscrollstopdistance) newy = 0.0f;
	control->setContentOverscroll(newx, newy);
}

void gGUIScrollable::updateContentFling() {
	updateContentOverscroll();
	gGUIScrollable* control = contentflingowner;
	if(!control) return;
	float step = (float)appmanager->getElapsedTime();
	if(step <= 0.0f) return;
	if(step > contentflingmaxstep) step = contentflingmaxstep;

	control->contentflingpositionx += control->contentflingvelocityx * step;
	control->contentflingpositiony += control->contentflingvelocityy * step;
	float decay = std::exp(-contentflingdecayrate * step);
	control->contentflingvelocityx *= decay;
	control->contentflingvelocityy *= decay;

	// Reaching an end stops that axis and pulls the coasting position back onto
	// the clamped value, so no unseen speed piles up beyond the content.
	int maxy = control->getTouchScrollMaxY();
	if(maxy > 0) {
		int target = (int)std::lround(control->contentflingpositiony);
		control->setTouchScrollY(gClamp(target, 0, maxy));
		if(target != control->getTouchScrollY()) {
			// What is left of the throw becomes the bounce rather than being
			// dropped, so reaching the end fast looks different from reaching it
			// slow. Only when nothing is stretched yet, or a fling grinding along
			// the end would keep topping it up and the spring would never finish.
			if(control->iscontentoverscrollenabled && control->contentoverscrolly == 0.0f) {
				control->setContentOverscroll(control->contentoverscrollx,
						resistContentOverscroll(control->contentflingvelocityy * contentoverscrollbouncetime));
			}
			control->contentflingvelocityy = 0.0f;
			control->contentflingpositiony = (float)control->getTouchScrollY();
		}
	} else {
		control->contentflingvelocityy = 0.0f;
	}
	int maxx = control->getTouchScrollMaxX();
	if(maxx > 0) {
		int target = (int)std::lround(control->contentflingpositionx);
		control->setTouchScrollX(gClamp(target, 0, maxx));
		if(target != control->getTouchScrollX()) {
			if(control->iscontentoverscrollenabled && control->contentoverscrollx == 0.0f) {
				control->setContentOverscroll(
						resistContentOverscroll(control->contentflingvelocityx * contentoverscrollbouncetime),
						control->contentoverscrolly);
			}
			control->contentflingvelocityx = 0.0f;
			control->contentflingpositionx = (float)control->getTouchScrollX();
		}
	} else {
		control->contentflingvelocityx = 0.0f;
	}

	if(std::fabs(control->contentflingvelocityx) < contentflingstopspeed &&
			std::fabs(control->contentflingvelocityy) < contentflingstopspeed) {
		contentflingowner = nullptr;
	}
}
#endif


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
#if GLIST_ANDROID || GLIST_IOS
	// The three statics below name whichever control is being dragged, coasting or
	// stretched. Nothing else clears them, so a control destroyed while it holds
	// one leaves a pointer to freed memory that updateContentFling() then writes
	// through, every frame, forever. Closing a tab or swapping a page is enough to
	// do it.
	if(contentdragowner == this) contentdragowner = nullptr;
	if(contentflingowner == this) contentflingowner = nullptr;
	if(contentoverscrollowner == this) contentoverscrollowner = nullptr;
#endif
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

	if (renderer->getScreenWidth() != boxfbo->getWidth() || renderer->getScreenHeight() != boxfbo->getHeight()) {
		boxfbo->allocate(renderer->getScreenWidth(), renderer->getScreenHeight());
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
	int scrollableheight = totalh - boxh;
	if (scrollableheight > 0) {
		verticalscroll = gClamp(verticalscroll, 0, scrollableheight);
	} else {
		verticalscroll = 0;
	}

	// totalh is 0 until a subclass says how tall its content is, and a float
	// division by it yields infinity - which is undefined behaviour on the way
	// back to an int, not merely a large number.
	scrollbarverticalsize = totalh > 0 ? (int)(((float) boxh / totalh) * boxh) : boxh;
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

	scrollbarhorizontalsize = totalw > 0 ? (int)(((float) boxw / totalw) * boxw) : boxw;
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
	// The content is rendered into a screen sized buffer starting at its top
	// left corner, so it must not inherit the scroll offset of the page this
	// control sits on. The offset is restored before the buffer is drawn back,
	// since that drawing does belong to the page.
	int pagescrollx = gRenderer::getScrollX();
	int pagescrolly = gRenderer::getScrollY();
	int pageoverscrollx = gRenderer::getOverscrollX();
	int pageoverscrolly = gRenderer::getOverscrollY();
	gRenderer::setScrollX(0);
	gRenderer::setScrollY(0);
	gRenderer::setOverscroll(0, 0);
#if GLIST_ANDROID || GLIST_IOS
	// The box can be wider or taller than the on-screen band once the page
	// scrolls, and drawn through the band's projection whatever falls outside the
	// band would be clipped and the buffer read below would stretch the clipped
	// edge across the gap. The buffer is sized to the box and the content is drawn
	// through the box's own projection, so all of it is captured whatever the band
	// is. See gRenderer::setContentProjection().
	int bufferwidth = renderer->unscaleX(width);
	int bufferheight = renderer->unscaleY(height);
	if (bufferwidth < 1) bufferwidth = 1;
	if (bufferheight < 1) bufferheight = 1;
	if (bufferwidth != boxfbo->getWidth() || bufferheight != boxfbo->getHeight()) {
		boxfbo->allocate(bufferwidth, bufferheight);
	}
	gRenderer::setContentProjection(width, height);
#endif
	boxfbo->bind();
	renderer->clearColor(0, 0, 0, 0);
#if GLIST_ANDROID || GLIST_IOS
	// The control's own rubber band is drawn the same way the page's is: by
	// shifting the band the content is rendered through. Nothing the content
	// itself does has to change, and no subclass sees a scroll position outside
	// its range - which matters, since several of them turn that position into a
	// row index and would read outside their data with a negative one.
	gRenderer::setOverscroll((int)std::lround(contentoverscrollx), (int)std::lround(contentoverscrolly));
	drawContent();
	// The bars sit against the edge of the box and belong to it, not to the
	// content, so they stay put while the content stretches away.
	gRenderer::setOverscroll(0, 0);
#else
	drawContent();
#endif
	drawScrollbars();
	boxfbo->unbind();
#if GLIST_ANDROID || GLIST_IOS
	gRenderer::clearContentProjection();
#endif
	gRenderer::setScrollX(pagescrollx);
	gRenderer::setScrollY(pagescrolly);
	gRenderer::setOverscroll(pageoverscrollx, pageoverscrolly);
	renderer->setColor(255, 255, 255);
#if GLIST_ANDROID || GLIST_IOS
	// The buffer was drawn through the box's own projection (setContentProjection),
	// so it holds the whole box and nothing else - the full buffer is the source.
	boxfbo->drawSub(left, top + titleheight,
					width, height,
					0, 0,
					boxfbo->getWidth(), boxfbo->getHeight());
#else
	boxfbo->drawSub(left, top + titleheight,
					width, height,
					0, renderer->getScreenHeight() - renderer->unscaleY(height),
					renderer->unscaleX(width), renderer->unscaleY(height));
#endif
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

#if GLIST_ANDROID || GLIST_IOS
	// A content drag is only possible where the content actually is: inside the
	// control, and not on either scrollbar, which have their own drag.
	// A finger on the glass takes the content over, which is what lets a coasting
	// list be stopped mid flight.
	if(contentflingowner == this) contentflingowner = nullptr;
	contentflingvelocityx = 0.0f;
	contentflingvelocityy = 0.0f;
	hascontentdragvelocity = false;
	iscontentdragging = false;
	iscontentdragarmed = !isdraggingverticalscroll && !isdragginghorizontalscroll &&
			isInsideTouchScrollArea(x, y);
	contentdragstartx = x;
	contentdragstarty = y;
	contentdragstartverticalscroll = getTouchScrollY();
	contentdragstarthorizontalscroll = getTouchScrollX();
	if(contentdragowner == this) contentdragowner = nullptr;
#endif
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

#if GLIST_ANDROID || GLIST_IOS
	if(isdraggingverticalscroll || isdragginghorizontalscroll || !iscontentdragarmed) return;

	// The content follows the finger one to one, unlike the thumb drag above
	// which is scaled: here the finger is on the content itself.
	int draggedx = contentdragstartx - x;
	int draggedy = contentdragstarty - y;
	int maxy = getTouchScrollMaxY();
	int maxx = getTouchScrollMaxX();
	bool canscrollvertically = maxy > 0;
	bool canscrollhorizontally = maxx > 0;
	if(!iscontentdragging) {
		bool passedvertically = canscrollvertically &&
				(draggedy >= contentdragthreshold || draggedy <= -contentdragthreshold);
		bool passedhorizontally = canscrollhorizontally &&
				(draggedx >= contentdragthreshold || draggedx <= -contentdragthreshold);
		if(!passedvertically && !passedhorizontally) return;
		iscontentdragging = true;
	}

	// Measured against the position the drag started from rather than the last
	// one seen, so that rounding cannot accumulate over a long drag.
	bool hasroomleft = false;
	float overx = 0.0f;
	float overy = 0.0f;
	if(canscrollvertically) {
		int target = contentdragstartverticalscroll + draggedy;
		setTouchScrollY(gClamp(target, 0, maxy));
		if(target == getTouchScrollY()) hasroomleft = true;
		// Whatever the clamp refused is how far past the end the finger is asking
		// to go, and that is what the rubber band shows.
		else overy = resistContentOverscroll((float)(target - getTouchScrollY()));
	}
	if(canscrollhorizontally) {
		int target = contentdragstarthorizontalscroll + draggedx;
		setTouchScrollX(gClamp(target, 0, maxx));
		if(target == getTouchScrollX()) hasroomleft = true;
		else overx = resistContentOverscroll((float)(target - getTouchScrollX()));
	}
	if(iscontentoverscrollenabled) {
		// Only where there is genuinely nowhere left to go. A page that can still
		// scroll is about to take the gesture over, and stretching the control for
		// the frame before that happens reads as a twitch rather than a limit.
		if(gRenderer::getMaxScrollX() > 0) overx = 0.0f;
		if(gRenderer::getMaxScrollY() > 0) overy = 0.0f;
		setContentOverscroll(overx, overy);
	}
	// The gesture is held only while the content still has somewhere to go.
	// Being pinned against the end releases it, which is the moment the page
	// behind is allowed to start scrolling instead. Note this is about the drag
	// running past the end, not about the finger pausing: a still finger keeps
	// asking for a reachable position and so keeps the gesture.
	contentdragowner = hasroomleft ? this : nullptr;
	trackContentVelocity();
#endif
}

void gGUIScrollable::mouseReleased(int x, int y, int button) {
	isdraggingverticalscroll = false;
	isdragginghorizontalscroll = false;
#if GLIST_ANDROID || GLIST_IOS
	// Only a drag that still owned the gesture may coast. Having lost it means
	// either the content was already pinned against its end, or the page has
	// taken the gesture over - in both cases a fling here would fight what the
	// user is actually doing.
	if(contentdragowner == this && iscontentdragging &&
			(std::fabs(contentflingvelocityx) >= contentflingminstartspeed ||
			 std::fabs(contentflingvelocityy) >= contentflingminstartspeed)) {
		contentflingowner = this;
		contentflingpositionx = (float)getTouchScrollX();
		contentflingpositiony = (float)getTouchScrollY();
	}
	iscontentdragarmed = false;
	iscontentdragging = false;
	if(contentdragowner == this) contentdragowner = nullptr;
#endif
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
