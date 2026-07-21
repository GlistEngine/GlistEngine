/****************************************************************************
 * Copyright (c) 2014 Nitra Games Ltd., Istanbul, Turkey                    *
 *                                                                          *
 * Permission is hereby granted, free of charge, to any person obtaining a  *
 * copy of this software and associated documentation files (the            *
 * "Software"), to deal in the Software without restriction, including      *
 * without limitation the rights to use, copy, modify, merge, publish,      *
 * distribute, distribute with modifications, sublicense, and/or sell       *
 * copies of the Software, and to permit persons to whom the Software is    *
 * furnished to do so, subject to the following conditions:                 *
 *                                                                          *
 * The above copyright notice and this permission notice should not be      *
 * deleted from the source form of the Software.                            *
 *                                                                          *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS  *
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF               *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.   *
 * IN NO EVENT SHALL THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR    *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR    *
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                          *
 * Except as contained in this notice, the name(s) of the above copyright   *
 * holders shall not be used in advertising or otherwise to promote the     *
 * sale, use or other dealings in this Software without prior written       *
 * authorization.                                                           *
 ****************************************************************************/

/****************************************************************************
 * Author: Noyan Culum, Sevval Bulburu, Aynur Dogan 2022-08-19              *
 ****************************************************************************/

/*
 * gGUIScrollable.h
 *
 *  Created on: Feb 5, 2022
 *      Author: noyan
 */

#ifndef UI_GGUISCROLLABLE_H_
#define UI_GGUISCROLLABLE_H_

#include "gGUIControl.h"
#include "gFbo.h"

#include <chrono>

/*
 * This class is a child class of gGUIControl. Scrollable class provide scrollable
 * function on objects. When the box object's width and height is smaller than
 * it's content, developers can use this class.
 *
 * For using this class, firstly a set function must be wrote in the class that
 * used Scrollalble class. In this set function, set function must be called
 * from gGUIControl class. Secondly, setDimensions() function must be called
 * from Scrollable class. setDimensions() function updates some variables for
 * using draw window and scroll bars. Because of that parameters should be width
 * and height. Except for this two function, developers can call any function
 * which should be setted when added object to panel.
 *
 * After written set function, enableScrollbars function must be called. This
 * function makes scroll bars visible. For using scrollable function at least one
 * of the parameters must be active.
 *
 * If a class is generated from Scrollable class with inheritance, new class must
 * overrided drawContent() function.
 *
 */

class gGUIScrollable: public gGUIControl {
public:
	gGUIScrollable();
	virtual ~gGUIScrollable();

	/*
	 * Makes scrollbars visible or invisible. For using scrollable function in
	 * other gui objects, this function must be used. First parameter is for the
	 * verticle scroll bar and the second one is horizontal scroll bar. Developer
	 * can use both of the equalities or one of them. bool 'true' makes the bars
	 * visible. When a bar is visible, that means that scrollable function is
	 * active for the given direction.
	 *
	 * @param isVerticalEnabled is a bool value that makes vertical scroll bar
	 * active or inactive. 'true ' for activation, 'false ' for inactivation.
	 *
	 * @param isHorizontalEnabled is a bool value that makes horizontal scroll bar
	 * active or inactive. 'true ' for activation, 'false ' for inactivation.
	 */
	void enableScrollbars(bool isVerticalEnabled, bool isHorizontalEnabled);

	void updateScrollbar();

	void draw();

	/*
	 * Draws contents of class. But it is empty for now, must be overrided in
	 * child classes from this parent class.
	 */
	virtual void drawContent();

	/*
	 *	Draws scroll bars according to if vertical or horizontal movement is
	 *	enabled.
	 */
	virtual void drawScrollbars();

	/*
	 *	Updates the width and height of the window with given parameters. This
	 *	function is overrided from gGUIControl class.
	 *
	 *	@param w is the new width value.
	 *
	 *	@param h is the new height value.
	 */
	virtual void windowResized(int w, int h);

	virtual void mouseMoved(int x, int y);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseDragged(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);

	/*
	 * Makes the scroll movement according to coordinate of the mouse. When mouse
	 * used for scroll, it changed some values of box that we draw.
	 *
	 */
	virtual void mouseScrolled(int x, int y);

	int getVerticalScroll();

#if GLIST_ANDROID || GLIST_IOS
	/*
	 * True while a finger is dragging some control's content and that content
	 * still has somewhere to go.
	 *
	 * On a touch screen the scrollbar thumb is too thin to hit, so scrolling has
	 * to be done by dragging the content itself. That puts the control in
	 * competition with the page it sits on, which scrolls by the same gesture.
	 * gAppManager reads this to settle the question: while it is true the finger
	 * belongs to the control, and the page keeps still.
	 *
	 * It goes false as soon as the drag is pinned against the control's own end,
	 * which is what lets the page take the gesture over from there.
	 */
	static bool isContentDragActive();

	/*
	 * Advances the content fling of whichever control is still coasting after a
	 * finger let go of it.
	 *
	 * Driven from gAppManager rather than from update() or draw(), because
	 * several subclasses override both without calling the base and would
	 * silently never coast. Only one control can be flung at a time - it takes a
	 * finger to start one - so a single call carries all of them.
	 */
	static void updateContentFling();

	// Ends any content fling at once, without letting it come to rest.
	static void cancelContentFling();
#endif

#if GLIST_ANDROID || GLIST_IOS
	/*
	 * Turns the rubber band at the ends of a control's own scroll on or off.
	 *
	 * On by default. Worth turning off for a control whose content is not a plain
	 * list of things - one where the edge is a hard boundary rather than the end
	 * of a run - since there the stretch reads as the drawing coming loose.
	 */
	void enableContentOverscroll(bool isEnabled);
	bool isContentOverscrollEnabled() const;
#endif


	gFbo* getFbo();

	int getTitleTop();

	void setToolbarSpace(int toolbarW, int toolbarH);

protected:
	bool isPointInsideVerticalScrollbar(int x, int y, bool checkFullSize = false);
	bool isPointInsideHorizontalScrollbar(int x, int y, bool checkFullSize = false);

	/*
	 *	Updates the values of variables that we use to draw window and scrollbars
	 *	with the given parameters.
	 *
	 *	@param w is the new width value.
	 *
	 *	@param h is the new height value.
	 */
	void setDimensions(int width, int height);

protected:
	const int barsize = 12;

	int scrollamount = 8;

	int boxw = 0, boxh = 0; // viewport size
	int totalw = 0, totalh = 0; // content size
	int verticalscroll = 0, horizontalscroll = 0; // content scroll amounts

	int verticalscrollbarpos = 0, horizontalscrollbarpos = 0; // scaled scrollbar position for rendering
	int scrollbarverticalsize = 0, scrollbarhorizontalsize = 0; // scrollbar size for rendering

	// for dragging the bars
	bool isdraggingverticalscroll = false;
	int verticalscrolldragstart = 0;
	float verticalscrollclickedtime = 0.0f;
	bool isdragginghorizontalscroll = false;
	int horizontalscrolldragstart = 0;
	float horizontalscrollclickedtime = 0.0f;

#if GLIST_ANDROID || GLIST_IOS
	// for dragging the content itself, which is how a finger scrolls
	bool iscontentdragarmed = false;
	bool iscontentdragging = false;
	int contentdragstartx = 0, contentdragstarty = 0;
	int contentdragstartverticalscroll = 0, contentdragstarthorizontalscroll = 0;

	// Speed of the drag, kept so the content can carry on when the finger lifts.
	float contentflingvelocityx = 0.0f, contentflingvelocityy = 0.0f;
	// The coasting position at sub unit precision, so a slow glide is not
	// rounded away frame after frame.
	float contentflingpositionx = 0.0f, contentflingpositiony = 0.0f;
	bool hascontentdragvelocity = false;
	std::chrono::steady_clock::time_point lastcontentdragtime;
	int lastcontentdragverticalscroll = 0, lastcontentdraghorizontalscroll = 0;

	// How far the content is drawn past its end, in unit space, either sign. A
	// drawing offset only - the scroll position stays clamped, which is what keeps
	// subclasses that read it as a row index safe.
	float contentoverscrollx = 0.0f, contentoverscrolly = 0.0f;
	bool iscontentoverscrollenabled = true;

	void trackContentVelocity();
	void setContentOverscroll(float overscrollX, float overscrollY);
	// Springs whichever control is stretched back towards its end. Static for the
	// same reason the fling is: only one control can be stretched at a time.
	static void updateContentOverscroll();

	/*
	 * What the finger moves, and how far it may move it.
	 *
	 * By default this is the content scroll every scrollable already has. A
	 * control whose finger scrollable region is something else - gGUINotebook
	 * drags its tab strip, which is neither the content nor a scrollbar -
	 * overrides these, and gets the drag, the hand over to the page and the
	 * fling without repeating any of it.
	 *
	 * A maximum of zero means that axis does not scroll, which is also how the
	 * page is told it may have the gesture.
	 */
	virtual int getTouchScrollX() const { return horizontalscroll; }
	virtual int getTouchScrollY() const { return verticalscroll; }
	virtual void setTouchScrollX(int value) { horizontalscroll = value; }
	virtual void setTouchScrollY(int value) { verticalscroll = value; }
	virtual int getTouchScrollMaxX() const { return totalw > boxw ? totalw - boxw : 0; }
	virtual int getTouchScrollMaxY() const { return totalh > boxh ? totalh - boxh : 0; }

	// Where a finger has to land for the drag to be about scrolling at all.
	virtual bool isInsideTouchScrollArea(int x, int y) const {
		return x >= left && x < left + boxw && y >= top && y < top + height;
	}
#endif
private:
#if GLIST_ANDROID || GLIST_IOS
	// The control currently moving its content under a finger, if any. Only one
	// can hold the gesture, so a single pointer answers isContentDragActive()
	// without every control having to be asked.
	static gGUIScrollable* contentdragowner;
	// The control still coasting after its finger left, if any.
	static gGUIScrollable* contentflingowner;
	// The control currently stretched past one of its ends, if any. Separate from
	// the two above because a rubber band outlives both: it is still springing
	// back after the finger has gone and the fling has stopped.
	static gGUIScrollable* contentoverscrollowner;
#endif
	gFbo* boxfbo;

	bool enableverticalscroll, enablehorizontalscroll;
	gColor barbackgroundcolor, barforegroundcolor;
	int toolbarw, toolbarh;
};

#endif /* UI_GGUISCROLLABLE_H_ */
