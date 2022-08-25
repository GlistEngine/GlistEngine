/*
 * gGUINotebookPanel.cpp
 *
 *  Created on: 10 Aug 2022
 *      Author: burakmeydan
 */

#include "gGUINotebookPanel.h"

gGUINotebookPanel::gGUINotebookPanel() {
	loadFont();
	topbarh = tabsizersh + 1;
	setSizer(&notebookpanelsizer);
}

gGUINotebookPanel::~gGUINotebookPanel() {

}

void gGUINotebookPanel::draw() {
	//gLogi("gGUINotebookPanel") << "draw";
	isslidingright = false;
	isslidingleft = false;
	if(istitlevisible) {
		topbarh = titleh + tabsizersh + 1;
		renderer->setColor(foregroundcolor);
		gDrawRectangle(left, top, width, titleh, true);
		renderer->setColor(fontcolor);
		font->drawText(title, left + 2, top + 12);
		tablinetop = titleh;
	}
	renderer->setColor(middlegroundcolor);
	gDrawRectangle(left, top + tablinetop, width, tabsizersh, true);
	renderer->setColor(foregroundcolor);
	gDrawRectangle(left, top + topbarh - 1, width, height - topbarh - 1, true);
	fbo.drawSub(left, tablinetop, left, top, width, height - tablinetop);
	fbo.bind();
	if(guisizer) {
		int x;
		int tabsize;
		if(width > tablabelssizew) {
			drawfromlefttoright = true;
			indexleft = 0;
		}
		renderer->setColor(middlegroundcolor);
		gDrawRectangle(left, top + tablinetop, width, tabsizersh, true);
		renderer->setColor(tablinecolor);
		gDrawLine(left, top + tablinetop, right, top + tablinetop);
		gDrawLine(left, top + topbarh - 1, right, top + topbarh - 1);
		if(drawfromlefttoright == true) {
			x = left;
			if(indexleft != 0) {
				drawSlideButtons(false);
				x += 11;
				isslidingleft = true;
			}
			for(int i = indexleft; i < guisizers.size(); i++) {
				tabsize = fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
				drawTabs(x, tabsize, i, drawfromlefttoright);
				if(x + tabsize > left + width) {
					//gLogi("gGUINotebookPanel") << "Overflowing panel -> " << quisizerlabels.at(i);
					isslidingright = true;
					drawSlideButtons(true);
					indexright = i - 1;
					break;
				}
				x = x + tabsize;
			}
		}
		else {
			x = left + width;
			if(indexright != guisizers.size() - 1) {
				drawSlideButtons(true);
				x -= 11;
				isslidingright = true;
			}
			for(int i = indexright; i >= 0; i--) {
				tabsize = fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;;
				drawTabs(x, tabsize, i, drawfromlefttoright);
				if(x - tabsize < left) {
					//gLogi("gGUINotebookPanel") << "Overflowing panel from left-> " << quisizerlabels.at(i);
					isslidingleft = true;
					drawSlideButtons(false);
					indexleft = i + 1;
					break;
				}
				x = x - tabsize;
			}
		}
		fbo.unbind();
		guisizer->draw();
	}
}

void gGUINotebookPanel::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(w, h);
	guisizer->set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y + topbarh, w, h - topbarh);
	fbo.allocate(getScreenWidth(), getScreenHeight(), false);
	if(!quisizerlabels.empty()) {
		tablabelssizew = getWidthOfTabLabels();
	}
	else {
		tablabelssizew = width;
	}
	//gLogi("gGUINotebookPanel") << tablabelssizew;
}

void gGUINotebookPanel::loadFont() {
	fontfortabs.load(font->getPath(), tabfontsize, font->isAntialised(), font->getDpi());
}

void gGUINotebookPanel::setTitleVisibility(bool isVisible) {
	istitlevisible = isVisible;
	topbarh = tabsizersh + 1;
}

void gGUINotebookPanel::mousePressed(int x, int y, int button) {
	//gLogi("gGUINotebookPanel") << "x -> " << x << ",  y -> " << y << ", button -> " << button;
	guisizer->mousePressed(x, y, button);
	int xleft;
	int tabsize;
	if(((isslidingleft && x < left + 10) || (isslidingright && x > left + width - 10)) && y <= top + topbarh && y >= top + tablinetop) {
		slideButtonPressed(x, y);
	}
	else if(drawfromlefttoright) {
		xleft = left + 1;
		for(int i = indexleft; i < guisizers.size(); i++) {
			tabsize = fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
			if(x <= xleft + tabsize && x >= xleft && y <= top + topbarh && y >= top + tablinetop) {
				setSizerFromDeque(i);
				break;
			}
			xleft = xleft + tabsize;
		}
	}
	else {
		xleft = left + width - 1;
		for(int i = indexright; i >= 0; i--) {
			tabsize = fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
			if(x >= xleft - tabsize && x <= xleft && y <= top + topbarh && y >= top + tablinetop) {
				setSizerFromDeque(i);
				break;
			}
			xleft = xleft - tabsize;
		}
	}
}

void gGUINotebookPanel::slideButtonPressed(int x, int y) {
	if(x > left + width - 10) {
		drawfromlefttoright = false;
		indexright++;
		//gLogi("gGUINotebookPanel") << "Right slide button pressed, indexright-> " << indexright;
	}
	else {
		drawfromlefttoright = true;
		indexleft--;
		//gLogi("gGUINotebookPanel") << "Left slide button pressed, indexleft-> " << indexleft;
	}
}

void gGUINotebookPanel::drawTabs(int x, int tabSize, int index, bool drawFromLeftToRight) {
	int sign = 1;
	if(activesizerindex == index) {
		renderer->setColor(foregroundcolor);
		gDrawRectangle(x - !drawFromLeftToRight * (tabSize), top + tablinetop + 1, tabSize, tabsizersh, true);
	}
	renderer->setColor(tablinecolor);
	if(drawFromLeftToRight == false)
		sign = -1;
	gDrawLine(x, top + tablinetop + 1, x, top + topbarh);
	gDrawLine(x + sign * tabSize, top + tablinetop + 1, x + sign * tabSize, top + topbarh);
	renderer->setColor(fontcolor);
	fontfortabs.drawText(quisizerlabels.at(index), x - (!drawFromLeftToRight * tabSize) + spaceforleft, top + topbarh - tabsizersh / 2.45f - 1);
}

void gGUINotebookPanel::drawSlideButtons(bool isRightButton) {
	if(isRightButton) {
		renderer->setColor(slidebuttoncolor);
		gDrawRectangle(left + width - 11, top + tablinetop + 1, 11, tabsizersh - 1, true);
		renderer->setColor(fontcolor);
		fontfortabs.drawText(">", left + width - 12, top + topbarh - tabsizersh / 2);
	}
	else {
		renderer->setColor(slidebuttoncolor);
		gDrawRectangle(left - 1, top + tablinetop + 1, 11, tabsizersh - 1, true);
		renderer->setColor(fontcolor);
		fontfortabs.drawText("<", left - 1, top + topbarh - tabsizersh / 2);
	}
}

int gGUINotebookPanel::getWidthOfTabLabels() {
	int tabsize = 0;
	for(int i = 0; i < guisizers.size(); i++) {
		tabsize += fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
	}
	return tabsize;
}
