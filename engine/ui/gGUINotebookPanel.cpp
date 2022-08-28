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
	fbo.bind();
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
	renderer->setColor(tablinecolor);
	gDrawLine(left, top + tablinetop, right, top + tablinetop);
	gDrawLine(left, top + topbarh - 1, right, top + topbarh - 1);
	if(guisizer) {
		int x;
		int tabsize;
		if(width > tablabelssizew) {
			drawfromlefttoright = true;
			indexleft = 0;
		}
		if(drawfromlefttoright == true) {
			x = left;
			if(indexleft != 0) {
				drawSlideButtons(false);
				x += slidebuttonw;
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
				x -= slidebuttonw;
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
		//renderer->setColor(color.YELLOW);
		//gDrawCross(left + width / 2 - 150, tablinetop + height / 2 - 150, 120.0f, 160.0f, 40.0f, true);////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		renderer->setColor(color);
		fbo.unbind();
		fbo.drawSub(left, top, left, top, width, height);
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
	if(((isslidingleft && x < left + slidebuttonw - 1) || (isslidingright && x > left + width - slidebuttonw + 1)) && y <= top + topbarh && y >= top + tablinetop) {
		slideButtonPressed(x, y);
	}
	else if(drawfromlefttoright) {
		xleft = left + 1;
		for(int i = indexleft; i < guisizers.size(); i++) {
			tabsize = fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
			if (x >= xleft - spaceforright + 7 + isslidingleft * (slidebuttonw - 1) + tabsize && x <= xleft - spaceforright + 7 + isslidingleft * (slidebuttonw - 1) + tabsize + 10 && y >= tablinetop + 6 && y <= tablinetop + 17) {
				//gLogi("gGUINotebookPanel") << "Close button pressed for-> " << quisizerlabels.at(i);
				closeTab(i);
				break;
			}
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
			if (x >= xleft - spaceforright + 9 - isslidingright * (slidebuttonw - 1) && x <= xleft - spaceforright + 9 - isslidingright * (slidebuttonw - 1) + 10 && y >= tablinetop + 6 && y <= tablinetop + 17) {
				//gLogi("gGUINotebookPanel") << "Close button pressed for-> " << quisizerlabels.at(i);
				closeTab(i);
				break;
			}
			tabsize = fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
			if(x >= xleft - tabsize && x <= xleft && y <= top + topbarh && y >= top + tablinetop) {
				setSizerFromDeque(i);
				break;
			}
			xleft = xleft - tabsize;
		}
	}
}

void gGUINotebookPanel::mouseMoved(int x, int y) {
	//gLogi("gGUINotebookPanel") << "x -> " << x << ",  y -> " << y;
	indexcursoroncross = -1;
	colorreductionr = false;
	colorreductionl = false;
	guisizer->mouseMoved(x, y);
	int xleft = 0;
	int tabsize = 0;
	if ((isslidingleft && x <= left + slidebuttonw - 1) && y <= top + topbarh && y >= top + tablinetop) {
		colorreductionl = true;
	}
	else if ((isslidingright && x >= left + width - slidebuttonw + 1) && y <= top + topbarh && y >= top + tablinetop) {
		colorreductionr = true;
	}
	else if(drawfromlefttoright) {
		xleft = left - spaceforright + 8 + isslidingleft * (slidebuttonw - 1);
		for(int i = indexleft; i < guisizers.size(); i++) {
			tabsize = fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
			if(x >= xleft + tabsize && x <= xleft + tabsize + 10 && y >= tablinetop + 6 && y <= tablinetop + 17) {
				indexcursoroncross = i;
				break;
			}
			xleft = xleft + tabsize;
		}
	}
	else {
		xleft = left + width - spaceforright + 8 - isslidingright * (slidebuttonw - 1);
		for(int i = indexright; i >= 0; i--) {
			if(x >= xleft && x <= xleft + 10 && y >= tablinetop + 6 && y <= tablinetop + 17) {
				indexcursoroncross = i;
				break;
			}
			tabsize = fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
			xleft = xleft - tabsize;
		}
	}
}

void gGUINotebookPanel::slideButtonPressed(int x, int y) {
	if(x > left + width - slidebuttonw + 1) {
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
	if(drawFromLeftToRight == false) {
		sign = -1;
		if(indexcursoroncross == index) {
			renderer->setColor(cursoroncrosscolor);
			gDrawCross(x - spaceforright + 11, tablinetop + 7, 10, 10, 2, true);
		}
		renderer->setColor(crossoutlinecolor);
		gDrawCross(x - spaceforright + 11, tablinetop + 7, 10, 10, 2, false);
	}
	else {
		if(indexcursoroncross == index) {
			renderer->setColor(cursoroncrosscolor);
			gDrawCross(x + tabSize - spaceforright + 11, tablinetop + 7, 10, 10, 2, true);
		}
		renderer->setColor(crossoutlinecolor);
		gDrawCross(x + tabSize - spaceforright + 11, tablinetop + 7, 10, 10, 2, false);
	}
	renderer->setColor(tablinecolor);
	gDrawLine(x, top + tablinetop + 1, x, top + topbarh);
	gDrawLine(x + sign * tabSize, top + tablinetop + 1, x + sign * tabSize, top + topbarh);
	renderer->setColor(fontcolor);
	fontfortabs.drawText(quisizerlabels.at(index), x - (!drawFromLeftToRight * tabSize) + spaceforleft, top + topbarh - tabsizersh / 2.45f);
}

void gGUINotebookPanel::drawSlideButtons(bool isRightButton) {
	int signsize = fontfortabs.getStringWidth(">");
	if(isRightButton) {
		if(colorreductionr)
			renderer->setColor(colorReductionOnCursor(slidebuttoncolor));
		else
			renderer->setColor(slidebuttoncolor);
		gDrawRectangle(left + width - slidebuttonw, top + tablinetop + 1, slidebuttonw, tabsizersh - 1, true);
		renderer->setColor(fontcolor);
		fontfortabs.drawText(">", left + width - (slidebuttonw / 2) - (signsize / 2) - 2, top + topbarh - tabsizersh / 2);
	}
	else {
		if(colorreductionl)
			renderer->setColor(colorReductionOnCursor(slidebuttoncolor));
		else
			renderer->setColor(slidebuttoncolor);
		gDrawRectangle(left - 1, top + tablinetop + 1, slidebuttonw, tabsizersh - 1, true);
		renderer->setColor(fontcolor);
		fontfortabs.drawText("<", left + (slidebuttonw / 2) - (signsize / 2) - 2, top + topbarh - tabsizersh / 2);
	}
}

void gGUINotebookPanel::closeTab(int index) {
	if(drawfromlefttoright) {
		if(index < activesizerindex) {
			activesizerindex--;
		}
		else if (index == activesizerindex) {
			setSizer(&notebookpanelsizer);
			activesizerindex = -1;
		}
		if(indexleft != 0)
			indexleft--;
	}
	else {
		if(index < activesizerindex) {
			activesizerindex--;
		}
		else if (index == activesizerindex) {
			setSizer(&notebookpanelsizer);
			activesizerindex = -1;
		}
		if(indexright != 0)
			indexright--;
	}
	guisizers.erase(guisizers.begin() + index);
	quisizerlabels.erase(quisizerlabels.begin() + index);
	tablabelssizew = getWidthOfTabLabels();
	indexcursoroncross = -1;
}

gColor gGUINotebookPanel::colorReductionOnCursor(gColor color) {
		color.set(color.r + oncursorcolorreduction, color.g + oncursorcolorreduction, color.b + oncursorcolorreduction);
		return color;
}

int gGUINotebookPanel::getWidthOfTabLabels() {
	int tabsize = 0;
	for(int i = 0; i < guisizers.size(); i++) {
		tabsize += fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
	}
	return tabsize;
}
