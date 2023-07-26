/*
 * gGUINotebook.cpp
 *
 *  Created on: 10 Aug 2022
 *      Author: burakmeydan
 */

#include <gGUINotebook.h>
gGUINotebook::gGUINotebook() {
	loadFont();
	topbarh = tabsizersh + 1;
	setSizer(&notebooksizer);
}

gGUINotebook::~gGUINotebook() {

}

void gGUINotebook::draw() {
	//gLogi("gGUINotebook") << "draw";
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
					//gLogi("gGUINotebook") << "Overflowing panel -> " << quisizerlabels.at(i);
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
					//gLogi("gGUINotebook") << "Overflowing panel from left-> " << quisizerlabels.at(i);
					isslidingleft = true;
					drawSlideButtons(false);
					indexleft = i + 1;
					break;
				}
				x = x - tabsize;
			}
		}
		guisizer->draw();
	}

	renderer->setColor(color);
	fbo.unbind();
//	fbo.drawSub(left, top, 0, 0, width, height);
	fbo.draw(0, 0);
}

void gGUINotebook::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
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
	//gLogi("gGUINotebook") << tablabelssizew;
}

void gGUINotebook::loadFont() {
	fontfortabs.load(font->getPath(), tabfontsize, font->isAntialised(), font->getDpi());
}

void gGUINotebook::mousePressed(int x, int y, int button) {
	//gLogi("gGUINotebook") << "x -> " << x << ",  y -> " << y << ", button -> " << button;
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
			if (x >= xleft - spaceforright + 7 + isslidingleft * (slidebuttonw - 1) + tabsize && x <= xleft - spaceforright + 7 + isslidingleft * (slidebuttonw - 1) + tabsize + 10
					&& y >= tablinetop + 6 && y <= tablinetop + 17 && isclosabletab) {
				//gLogi("gGUINotebook") << "Close button pressed for-> " << quisizerlabels.at(i);
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
			if (x >= xleft - spaceforright + 9 - isslidingright * (slidebuttonw - 1) && x <= xleft - spaceforright + 9 - isslidingright * (slidebuttonw - 1) + 10
					&& y >= tablinetop + 6 && y <= tablinetop + 17 && isclosabletab) {
				//gLogi("gGUINotebook") << "Close button pressed for-> " << quisizerlabels.at(i);
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

void gGUINotebook::mouseMoved(int x, int y) {
	//gLogi("gGUINotebook") << "x -> " << x << ",  y -> " << y;
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

void gGUINotebook::slideButtonPressed(int x, int y) {
	if(x > left + width - slidebuttonw + 1) {
		drawfromlefttoright = false;
		indexright++;
		//gLogi("gGUINotebook") << "Right slide button pressed, indexright-> " << indexright;
	}
	else {
		drawfromlefttoright = true;
		indexleft--;
		//gLogi("gGUINotebook") << "Left slide button pressed, indexleft-> " << indexleft;
	}
}

void gGUINotebook::drawTabs(int x, int tabSize, int index, bool drawFromLeftToRight) {
	int sign = 1;
	if(activesizerindex == index) {
		renderer->setColor(foregroundcolor);
		gDrawRectangle(x - !drawFromLeftToRight * (tabSize), top + tablinetop + 1, tabSize, tabsizersh, true);
	}
	if(drawFromLeftToRight == false) {
		sign = -1;
		if(indexcursoroncross == index && isclosabletab) {
			renderer->setColor(cursoroncrosscolor);
			gDrawCross(x - spaceforright + 11, tablinetop + 7, 10, 10, 2, true);
		}
		if(isclosabletab) {
			renderer->setColor(crossoutlinecolor);
			gDrawCross(x - spaceforright + 11, tablinetop + 7, 10, 10, 2, false);
		}
	}
	else {
		if(indexcursoroncross == index && isclosabletab) {
			renderer->setColor(cursoroncrosscolor);
			gDrawCross(x + tabSize - spaceforright + 11, tablinetop + 7, 10, 10, 2, true);
		}
		if(isclosabletab) {
			renderer->setColor(crossoutlinecolor);
			gDrawCross(x + tabSize - spaceforright + 11, tablinetop + 7, 10, 10, 2, false);
		}
	}
	renderer->setColor(tablinecolor);
	gDrawLine(x, top + tablinetop + 1, x, top + topbarh);
	gDrawLine(x + sign * tabSize, top + tablinetop + 1, x + sign * tabSize, top + topbarh);
	renderer->setColor(fontcolor);
	fontfortabs.drawText(quisizerlabels.at(index), x - (!drawFromLeftToRight * tabSize) + spaceforleft, top + topbarh - tabsizersh / 2.45f);
}

void gGUINotebook::drawSlideButtons(bool isRightButton) {
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

void gGUINotebook::closeTab(int index) {
	if(drawfromlefttoright) {
		if(index < activesizerindex) {
			activesizerindex--;
		}
		else if (index == activesizerindex) {
			setSizer(&notebooksizer);
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
			setSizer(&notebooksizer);
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

gColor gGUINotebook::colorReductionOnCursor(gColor color) {
		color.set(color.r + oncursorcolorreduction, color.g + oncursorcolorreduction, color.b + oncursorcolorreduction);
		return color;
}

int gGUINotebook::getWidthOfTabLabels() {
	int tabsize = 0;
	for(int i = 0; i < guisizers.size(); i++) {
		tabsize += fontfortabs.getStringWidth(quisizerlabels.at(i)) + spaceforleft + spaceforright;
	}
	return tabsize;
}

void gGUINotebook::setTitleVisibility(bool isVisible) {
	istitlevisible = isVisible;
	topbarh = tabsizersh + 1;
}

void gGUINotebook::setClosableTab(bool isClosableTab) {
	isclosabletab = isClosableTab;
}

void gGUINotebook::setSizerFromDeque(int guiSizersIndex) {
	activesizerindex = guiSizersIndex;
	guisizer = guisizers.at(guiSizersIndex);
	guisizer->setTopParent(topparent);
	guisizer->setParent(this);
	guisizer->setParentSlotNo(0, 0);
	guisizer->setRootApp(root);
	guisizer->set(left, topbarh, width, height - topbarh);
	if(!sizerrescaling) guisizer->enableRescaling(false);
}

void gGUINotebook::addSizerToDeque(gGUISizer* guiSizer, std::string sizerLabel) {
	guisizers.push_back(guiSizer);
	quisizerlabels.push_back(sizerLabel);
}

void gGUINotebook::setActiveTab(int index) {
    if (index >= 0 && index < guisizers.size()) {
        setSizerFromDeque(index);
    }
}

