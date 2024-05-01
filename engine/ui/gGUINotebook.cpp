/*
 * gGUINotebook.cpp
 *
 *  Created on: 30 Apr 2024
 *      Author: Metehan Gezer
 */

#include <gGUINotebook.h>
#include <algorithm>

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
	tabscroll = std::min(tabscroll, scrollmax);
	tabscroll = std::max(tabscroll, 0);

	renderer->setColor(0, 0, 0);
	fbo.bind();
	renderer->clearColor(0, 0, 0, 0);
	drawHeaderBackground();

	int initialstart = 0;
	if (showscrollbuttons) {
		initialstart = scrollbuttonwidth;
	}
	int start = initialstart;
	for (int i = 0; i < tabs.size(); ++i) {
		Tab& tab = tabs[i];
		int relativestartx = start - tabscroll;
		int tabwidth = tab.tabwidth;
		if (relativestartx + tabwidth < initialstart || relativestartx > availablespace) {
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
	fbo.getTexture().drawSub(left + shiftx, top + shifty, notebookbox.w * scalex, notebookbox.h * scaley, 0, renderer->getHeight() - notebookbox.h, notebookbox.w, notebookbox.h, 0, 0, rotate);

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

void gGUINotebook::mousePressed(int x, int y, int button) {
	if (scrollpreviousbutton.rotate(tabposition, width, height).isPointInside(x, y)) {
		tabscroll -= 100;
		return;
	}
	if (scrollnextbutton.rotate(tabposition, width, height).isPointInside(x, y)) {
		tabscroll += 100;
		return;
	}
	for (int i = 0; i < tabs.size(); ++i) {
		Tab& tab = tabs[i];
		if (tab.closebox.rotate(tabposition, width, height).isPointInside(x, y)) {
			closeTab(i);
			break;
		}
		if (tab.tabbox.rotate(tabposition, width, height).isPointInside(x, y)) {
			setActiveTab(i);
			break;
		}
	}
}

void gGUINotebook::mouseMoved(int x, int y) {
	lastmousex = x;
	lastmousey = y;
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