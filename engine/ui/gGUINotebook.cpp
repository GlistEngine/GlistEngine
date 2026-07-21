/*
 * gGUINotebook.cpp
 *
 *  Created on: 30 Apr 2024
 *      Author: Metehan Gezer
 */

#include <gGUINotebook.h>
#include <algorithm>
#include <cmath>

gGUINotebook::gGUINotebook() {
	tabposition = TabPosition::TOP;
	titlefont = font;
	titlepadding = 8;
	tabgap = 5;
	tabscroll = 0;
	headerheight = titlefont->getStringHeight("A") + titlepadding * 2;
	scrollbuttonwidth = headerheight;
	closebuttonsize = 8;
	tabvisibility = true;
	activetab = -1;
	setSizer(&notebooksizer);
}

gGUINotebook::~gGUINotebook() {

}

void gGUINotebook::set(gBaseApp* root, gBaseGUIObject* topParent, gBaseGUIObject* parent, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIScrollable::set(root, topParent, parent, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(w, h);
	fbo.allocate(getScreenWidth(), getScreenHeight());
	setTabVisibility(tabvisibility);
}


void gGUINotebook::draw() {
	gColor* oldcolor = renderer->getColor();

	if (tabvisibility) {
		drawHeader();
	} else {
		headerbox.enabled = false;
	}
	// box outline
	renderer->setColor(backgroundcolor);
	gDrawRectangle(left, top, width, height, false);

	renderer->setColor(255, 255, 255);
	guisizer->draw();
	renderer->setColor(oldcolor);
}

void gGUINotebook::setTabPosition(gGUINotebook::TabPosition position) {
	tabposition = position;
	updateSizer();
}

gGUINotebook::TabPosition gGUINotebook::getTabPosition() const {
	return tabposition;
}

void gGUINotebook::setTabVisibility(bool visible) {
	tabvisibility = visible;
	updateSizer();
}

void gGUINotebook::setActiveTab(int index) {
	if (index == activetab) {
		return;
	}
	Tab* oldtab = getTab(activetab);
	if (oldtab != nullptr && oldtab->autoclose) {
		int tmp = activetab;
		// we set the active tab to -1, so it won't try to call
		// setActiveTab again to open the previous tab
		activetab = -1;
		closeTab(tmp);
	}
	Tab* newtab = getTab(index);
	if (newtab == nullptr) {
		// set tab to inactive
		activetab = -1;
		guisizer->removeControl(0, 0);
		return;
	}
	// set new tab as active
	activetab = index;
	guisizer->setControl(0, 0, newtab->sizer);
}

int gGUINotebook::getActiveTab() const {
	return activetab;
}

void gGUINotebook::setTabClosable(int index, bool isClosable) {
	Tab* tab = getTab(index);
	if (tab == nullptr) {
		return;
	}
	tab->closable = isClosable;
}

bool gGUINotebook::isTabClosable(int index) {
	Tab* tab = getTab(index);
	if (tab == nullptr) {
		return false;
	}
	return tab->closable;
}

void gGUINotebook::setTabAutoClose(int index, bool autoClose) {
	Tab* tab = getTab(index);
	if (tab == nullptr) {
		return;
	}
	tab->autoclose = autoClose;
}

bool gGUINotebook::isTabAutoClose(int index) {
	Tab* tab = getTab(index);
	if (tab == nullptr) {
		return false;
	}
	return tab->autoclose;
}

int gGUINotebook::addTab(gGUISizer* sizer, std::string title, bool closable) {
	tabs.emplace_back(sizer, title, closable, (int) titlefont->getStringWidth(title), (int) titlefont->getStringHeight(title));
	if (activetab < 0) {
		setActiveTab(tabs.size() - 1);
	}
	return tabs.size() - 1;
}

void gGUINotebook::closeTab(int index) {
	if (index < 0 || index >= tabs.size()) {
		return;
	}
	if (index == activetab) {
		setActiveTab(activetab - 1); // sets the previous tab as active
	}
	tabs.erase(tabs.begin() + index);
}

gGUISizer* gGUINotebook::getTabSizer(int index) {
	Tab* tab = getTab(index);
	if (tab != nullptr) {
		return tab->sizer;
	}
	return nullptr;
}

gGUISizer* gGUINotebook::getTabSizerByTitle(const std::string& title) {
	return getTabSizer(findIndexByTitle(title));
}

gGUINotebook::Tab* gGUINotebook::getTab(int index) {
	if (index < 0 || index >= tabs.size()) {
		return nullptr;
	}
	return &tabs[index];
}

int gGUINotebook::findIndexByTitle(const std::string& title) {
	int i = 0;
	for (const auto& item : tabs) {
		if (item.title == title) {
			return i;
		}
		i++;
	}
	return -1;
}

void gGUINotebook::drawHeader() {
	notebookbox.enabled = true;
	notebookbox.ox = left;
	notebookbox.oy = top;
	notebookbox.x = 0;
	notebookbox.y = 0;
	notebookbox.w = width;
	notebookbox.h = height;

	headerbox.enabled = true;
	headerbox.ox = left;
	headerbox.oy = top;

	if (tabposition == TabPosition::TOP) {
		headerbox.x = 0;
		headerbox.y = 0;
		headerbox.w = width;
		headerbox.h = headerheight;
	} else if (tabposition == TabPosition::LEFT) {
		notebookbox.flipXY();
		headerbox.x = 0;
		headerbox.y = 0;
		headerbox.w = headerheight;
		headerbox.h = height;
	} else if (tabposition == TabPosition::BOTTOM) {
		headerbox.x = 0;
		headerbox.y = height - headerheight;
		headerbox.w = width;
		headerbox.h = headerheight;
	} else if (tabposition == TabPosition::RIGHT) {
		notebookbox.flipXY();
		headerbox.x = width - headerheight;
		headerbox.y = 0;
		headerbox.w = headerheight;
		headerbox.h = height;
	}
	bool fliptextvertically = tabposition == TabPosition::BOTTOM;
	bool fliptexthorizontally = tabposition == TabPosition::LEFT;

	int totalbarsize = 0;
	// prepare tabs and calculate total width
	for (int i = 0; i < tabs.size(); ++i) {
		Tab& tab = tabs[i];
		tab.tabwidth = tab.titlewidth + titlepadding * 2;
		if (tab.closable) {
			// close button and its padding
			tab.tabwidth += 8 + titlepadding;
		}
		// width calculation
		totalbarsize += tab.tabwidth + tabgap;
	}

	// limit scroll
	int availablespace = notebookbox.w;
	bool showscrollbuttons = false;
	if (totalbarsize > width) {
		availablespace -= scrollbuttonwidth * 2;
		showscrollbuttons = true;
	}
	int scrollmax = totalbarsize - availablespace;
	scrollmax = std::max(0, scrollmax);
	tabscrollmax = scrollmax;
	tabscroll = std::min(tabscroll, scrollmax);
	tabscroll = std::max(tabscroll, 0);

	renderer->setColor(0, 0, 0);
	// The strip is rendered into a screen sized buffer starting at its top left
	// corner, so it must not inherit the scroll offset of the page this notebook
	// sits on - with the offset left in, scrolling the page walks the strip out
	// of the buffer and it simply vanishes. The offset is put back before the
	// buffer is drawn to the screen, since that drawing does belong to the page.
	int pagescrollx = gRenderer::getScrollX();
	int pagescrolly = gRenderer::getScrollY();
	int pageoverscrollx = gRenderer::getOverscrollX();
	int pageoverscrolly = gRenderer::getOverscrollY();
	gRenderer::setScrollX(0);
	gRenderer::setScrollY(0);
	// The page's rubber band would otherwise stretch the strip along with it,
	// which is not what is being pulled.
	gRenderer::setOverscroll(0, 0);
#if GLIST_ANDROID || GLIST_IOS
	// The strip spans the whole notebook width, which can be wider than the
	// on-screen band once the page scrolls sideways. Drawn through the band's
	// projection its far part would be clipped, and the buffer read at the bottom
	// of this function would then stretch the clipped edge across the gap -
	// straight black streaks past the last visible tab. The buffer is sized to the
	// strip and the strip is drawn through its own box, so all of it is captured
	// whatever the band is. See gRenderer::setContentProjection().
	int bufferwidth = renderer->unscaleX(notebookbox.w);
	int bufferheight = renderer->unscaleY(notebookbox.h);
	if (bufferwidth < 1) bufferwidth = 1;
	if (bufferheight < 1) bufferheight = 1;
	if (fbo.getWidth() != bufferwidth || fbo.getHeight() != bufferheight) {
		fbo.allocate(bufferwidth, bufferheight);
	}
	gRenderer::setContentProjection(notebookbox.w, notebookbox.h);
#endif
	fbo.bind();
	renderer->clearColor(0, 0, 0, 0);
	drawHeaderBackground();

	int initialstart = 0;
	if (showscrollbuttons) {
		initialstart = scrollbuttonwidth;
	}
#if GLIST_ANDROID || GLIST_IOS
	// The strip is always laid out left to right in the buffer and rotated on the
	// way out, so its rubber band is always along x here whichever side the tabs
	// are on. Applied to the tabs alone: the background and the scroll buttons
	// belong to the box, not to the strip sliding inside it.
	gRenderer::setOverscroll((int)std::lround(isVerticalTabStrip() ? contentoverscrolly : contentoverscrollx), 0);
#endif
	int start = initialstart;
	for (int i = 0; i < tabs.size(); ++i) {
		Tab& tab = tabs[i];
		int relativestartx = start - tabscroll;
		int tabwidth = tab.tabwidth;
		// The strip runs from initialstart to initialstart + availablespace, so its
		// far edge is the sum of the two. Testing against availablespace alone cut
		// the strip short by the width of one scroll button: a tab starting inside
		// that band was dropped instead of being drawn half in view, which left
		// grey where it should have been and made it appear all at once after the
		// smallest drag.
		if (relativestartx + tabwidth < initialstart || relativestartx > initialstart + availablespace) {
			start += tabwidth + tabgap;
			tab.tabbox.enabled = false;
			tab.closebox.enabled = false;
			continue;
		}
		renderer->setColor(foregroundcolor);
		gDrawRectangle(relativestartx, 0, tabwidth, headerheight, true);
		if (activetab != i) {
			renderer->setColor(middlegroundcolor);
			gDrawRectangle(relativestartx, headerheight - 3, tabwidth, 3, true);
			renderer->setColor(backgroundcolor);
			gDrawRectangle(relativestartx, headerheight - 1, tabwidth, 1, true);
		}
		renderer->setColor(fontcolor);
		if (fliptextvertically) {
			titlefont->drawTextVerticallyFlipped(tab.title, relativestartx + titlepadding - 3, titlepadding + (headerheight - tab.titleheight) / 2 - 4);
		} else if (fliptexthorizontally) {
			titlefont->drawTextHorizontallyFlipped(tab.title, relativestartx + titlepadding + 3 + tab.titlewidth, titlepadding + (headerheight - tab.titleheight) / 2);
		} else {
			titlefont->drawText(tab.title, relativestartx + titlepadding - 3, titlepadding + (headerheight - tab.titleheight) / 2);
		}
		if (tab.closable) {
			int ax = relativestartx + tabwidth - titlepadding - closebuttonsize;
			int ay = titlepadding + 1;
			int bx = ax + closebuttonsize;
			int by = ay + closebuttonsize;
			int cx = ax + closebuttonsize;
			int cy = ay;
			int dx = ax;
			int dy = ay + closebuttonsize;
			gDrawLine(ax, ay, bx, by);
			gDrawLine(cx, cy, dx, dy);

			tab.closebox.ox = left - closebuttonsize;
			tab.closebox.oy = top - closebuttonsize;
			tab.closebox.x = ax + closebuttonsize;
			tab.closebox.y = ay + closebuttonsize;
			tab.closebox.w = closebuttonsize;
			tab.closebox.h = closebuttonsize;
			tab.closebox.enabled = true;
		}

		// update render data
		tab.tabbox.ox = left;
		tab.tabbox.oy = top;
		tab.tabbox.x = relativestartx;
		tab.tabbox.y = 0;
		tab.tabbox.w = tabwidth;
		tab.tabbox.h = headerheight;
		tab.tabbox.enabled = true;
		start += tabwidth + tabgap;
	}
#if GLIST_ANDROID || GLIST_IOS
	gRenderer::setOverscroll(0, 0);
#endif
	if (showscrollbuttons) {
		renderer->setColor(middlegroundcolor);
		gDrawRectangle(0, -1, scrollbuttonwidth, headerheight + 1, true);
		gDrawRectangle(notebookbox.w - scrollbuttonwidth, -1, scrollbuttonwidth, headerheight + 1, true);
		renderer->setColor(backgroundcolor);
		gDrawRectangle(0, -1, scrollbuttonwidth, headerheight + 1, false);
		gDrawRectangle(notebookbox.w - scrollbuttonwidth, -1, scrollbuttonwidth, headerheight + 1, false);

		// left arrow
		if (tabscroll > 0) {
			renderer->setColor(fontcolor);
		} else {
			renderer->setColor(backgroundcolor);
		}
		int offset = 3;
		gDrawLine(scrollbuttonwidth / 2 - offset, headerheight / 2,
				  scrollbuttonwidth / 2 + scrollbuttonwidth / 4 - offset, headerheight / 2 - scrollbuttonwidth / 4);
		gDrawLine(scrollbuttonwidth / 2 - offset, headerheight / 2,
				  scrollbuttonwidth / 2 + scrollbuttonwidth / 4 - offset, headerheight / 2 + scrollbuttonwidth / 4);

		// right arrow
		if (tabscroll < scrollmax) {
			renderer->setColor(fontcolor);
		} else {
			renderer->setColor(backgroundcolor);
		}
		offset = 2;
		gDrawLine(notebookbox.w - scrollbuttonwidth / 2 + offset, headerheight / 2,
				  notebookbox.w - scrollbuttonwidth / 2 - scrollbuttonwidth / 4 + offset, headerheight / 2 - scrollbuttonwidth / 4);
		gDrawLine(notebookbox.w - scrollbuttonwidth / 2 + offset, headerheight / 2,
				  notebookbox.w - scrollbuttonwidth / 2 - scrollbuttonwidth / 4 + offset, headerheight / 2 + scrollbuttonwidth / 4);

		// update data so clicking works
		scrollpreviousbutton.enabled = true;
		scrollpreviousbutton.ox = left;
		scrollpreviousbutton.oy = top;
		scrollpreviousbutton.x = 0;
		scrollpreviousbutton.y = 0;
		scrollpreviousbutton.w = scrollbuttonwidth;
		scrollpreviousbutton.h = scrollbuttonwidth;

		scrollnextbutton.enabled = true;
		scrollnextbutton.ox = left;
		scrollnextbutton.oy = top;
		scrollnextbutton.x = notebookbox.w - scrollbuttonwidth;
		scrollnextbutton.y = 0;
		scrollnextbutton.w = scrollbuttonwidth;
		scrollnextbutton.h = scrollbuttonwidth;
	} else {
		scrollpreviousbutton.enabled = false;
		scrollnextbutton.enabled = false;
	}

	fbo.unbind();
#if GLIST_ANDROID || GLIST_IOS
	gRenderer::clearContentProjection();
#endif
	gRenderer::setScrollX(pagescrollx);
	gRenderer::setScrollY(pagescrolly);
	gRenderer::setOverscroll(pageoverscrollx, pageoverscrolly);
	renderer->setColor(255, 255, 255, 255);
	float rotate = 0;
	int shiftx = 0;
	int shifty = 0;
	int scalex = 1;
	int scaley = 1;
	if (tabposition == TabPosition::RIGHT) {
		rotate = 90;
		shiftx = width;
	} else if (tabposition == TabPosition::LEFT) {
		rotate = 90;
		scaley = -1;
	} else if (tabposition == TabPosition::BOTTOM) {
		shifty = height;
		scaley = -1;
	}
#if GLIST_ANDROID || GLIST_IOS
	// The buffer was drawn through the strip's own box (setContentProjection), so
	// it holds the whole strip and nothing else - the full buffer is the source.
	fbo.getTexture().drawSub(left + shiftx, top + shifty, notebookbox.w * scalex, notebookbox.h * scaley, 0, 0, fbo.getWidth(), fbo.getHeight(), 0, 0, rotate);
#else
	fbo.getTexture().drawSub(left + shiftx, top + shifty, notebookbox.w * scalex, notebookbox.h * scaley, 0, renderer->getHeight() - notebookbox.h, notebookbox.w, notebookbox.h, 0, 0, rotate);
#endif

	// visualise hitboxes for debugging
	renderer->setColor(255, 0, 0);
	/*for (const auto& item : tabs) {
		if (!item.tabbox.enabled) {
			continue;
		}
		item.tabbox.rotate(tabposition, width, height).render();
		item.closebox.rotate(tabposition, width, height).render();
	}
	headerbox.render();
	notebookbox.rotate(tabposition, width, height).render();
	scrollpreviousbutton.rotate(tabposition, width, height).render();
	scrollnextbutton.rotate(tabposition, width, height).render();*/
}

void gGUINotebook::drawHeaderBackground() {
	// header background
	renderer->setColor(middlegroundcolor);
	gDrawRectangle(0, 0, notebookbox.w, headerheight, true);

	renderer->setColor(backgroundcolor);
	// header outline
	gDrawRectangle(0, 0, notebookbox.w, headerheight, false);
}

void gGUINotebook::updateSizer() {
	if (!tabvisibility) {
		guisizer->set(root, topparent, parent, parentslotlineno, parentslotcolumnno,
					  left, top + topbarh,
					  width, height - topbarh);
		return;
	}
	switch (tabposition) {
	case TabPosition::TOP:
		guisizer->set(root, topparent, parent, parentslotlineno, parentslotcolumnno,
			  left, top + topbarh + headerheight,
			  width, height - topbarh - headerheight);
		break;
	case TabPosition::BOTTOM:
		guisizer->set(root, topparent, parent, parentslotlineno, parentslotcolumnno,
					  left, top + topbarh,
					  width, height - topbarh - headerheight);
		break;
	case TabPosition::LEFT:
		guisizer->set(root, topparent, parent, parentslotlineno, parentslotcolumnno,
					  left + headerheight, top + topbarh,
					  width - headerheight, height - topbarh);
		break;
	case TabPosition::RIGHT:
		guisizer->set(root, topparent, parent, parentslotlineno, parentslotcolumnno,
					  left, top + topbarh,
					  width - headerheight, height - topbarh);
		break;
	}
}

#if GLIST_ANDROID || GLIST_IOS
int gGUINotebook::getNaturalHeight() {
	int stripheight = tabvisibility ? headerheight : 0;
	int bodyheight = guisizer ? guisizer->getNaturalHeight() : 0;
	return stripheight + bodyheight;
}

void gGUINotebook::setReferenceHeight(int referenceHeight) {
	if(!guisizer) return;
	int stripheight = tabvisibility ? headerheight : 0;
	int bodyheight = referenceHeight - stripheight;
	if(bodyheight < 0) bodyheight = 0;
	guisizer->setReferenceHeight(bodyheight);
}
#endif

bool gGUINotebook::handleHeaderPress(int x, int y) {
	if (scrollpreviousbutton.rotate(tabposition, width, height).isPointInside(x, y)) {
		tabscroll -= 100;
		if (tabscroll < 0) tabscroll = 0;
		return true;
	}
	if (scrollnextbutton.rotate(tabposition, width, height).isPointInside(x, y)) {
		tabscroll += 100;
		return true;
	}
	for (int i = 0; i < tabs.size(); ++i) {
		Tab& tab = tabs[i];
		if (tab.closebox.rotate(tabposition, width, height).isPointInside(x, y)) {
			closeTab(i);
			return true;
		}
		if (tab.tabbox.rotate(tabposition, width, height).isPointInside(x, y)) {
			setActiveTab(i);
			return true;
		}
	}
	return false;
}

void gGUINotebook::mousePressed(int x, int y, int button) {
#if GLIST_ANDROID || GLIST_IOS
	// Arms the tab strip drag, the strip being this control's touch scroll area.
	gGUIScrollable::mousePressed(x, y, button);
	if (isInsideTouchScrollArea(x, y)) {
		// A finger on the strip is either tapping a tab or starting to drag
		// across it, and which one it is is not known yet. Acting at release
		// instead is what stops a drag from switching tabs on its way past.
		return;
	}
#else
	if (handleHeaderPress(x, y)) {
		return;
	}
#endif
	gGUIContainer::mousePressed(x, y, button);
}

void gGUINotebook::mouseDragged(int x, int y, int button) {
#if GLIST_ANDROID || GLIST_IOS
	if (iscontentdragarmed) {
		gGUIScrollable::mouseDragged(x, y, button);
		return;
	}
#endif
	gGUIContainer::mouseDragged(x, y, button);
}

void gGUINotebook::mouseReleased(int x, int y, int button) {
#if GLIST_ANDROID || GLIST_IOS
	bool wasonstrip = iscontentdragarmed;
	bool wasdragging = iscontentdragging;
	// Clears the drag and starts the fling if the strip was thrown.
	gGUIScrollable::mouseReleased(x, y, button);
	if (wasonstrip) {
		// A gesture that never became a drag is a tap, and only then does the
		// tab under it get picked. A withdrawn gesture arrives from a point no
		// tab occupies, so it lands on nothing either way.
		if (!wasdragging) handleHeaderPress(x, y);
		return;
	}
#endif
	gGUIContainer::mouseReleased(x, y, button);
}

void gGUINotebook::mouseMoved(int x, int y) {
	lastmousex = x;
	lastmousey = y;
	// Controls living inside a tab are reachable only through here: gGUISizer
	// routes presses and drags by iscursoron, and nothing but mouseMoved sets it.
	gGUIContainer::mouseMoved(x, y);
}

void gGUINotebook::mouseScrolled(int x, int y) {
	if (headerbox.isPointInside(lastmousex, lastmousey)) {
		if (tabposition == TabPosition::LEFT || tabposition == TabPosition::RIGHT) {
			tabscroll -= y * 20;
		} else if (tabposition == TabPosition::TOP || tabposition == TabPosition::BOTTOM) {
			tabscroll -= x * 20;
		}
	}
}