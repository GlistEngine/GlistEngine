/*
 * gGUISizer.cpp
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#include "gGUISizer.h"
#include "gGUIControl.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"


gGUISizer::gGUISizer() {
	sizertype = SIZERTYPE_HORIZONTAL;
	bordersenabled = false;
	linenum = 1;
	columnnum = 1;
	lineprs = new float[1];
	lineprs[0] = 1.0f;
	columnprs = new float[1];
	columnprs[0] = 1.0f;
	linetprs = new float[1];
	linetprs[0] = 0.0f;
	columntprs = new float[1];
	columntprs[0] = 0.0f;
	resizable = false;
	resizecolumn = 0;
	resizeline = 0;
	fillbackground = false;
	slotpadding = 2;
}

gGUISizer::~gGUISizer() {
}

void gGUISizer::set(int x, int y, int w, int h) {
	gLogi("Sizer") << "set";
	left = x;
	top = y;
	right = x + w;
	bottom = y + h;
	width = w;
	height = h;

	for (int i = 0; i < linenum; i++) {
		for (int j = 0; j < columnnum; j++) {
			if(iscontrolset[i][j]) {
				guicontrol[i][j]->set(
					root,
					this,
					i,
					j,
					left + (width * columntprs[j]),
					top + (height * linetprs[i]),
					width * (columntprs[j + 1] - columntprs[j]),
					height * (linetprs[i + 1] - linetprs[i])
				);
			}
		}
	}
}

int gGUISizer::getSizerType() {
	return sizertype;
}

void gGUISizer::setSize(int lineNum, int columnNum) {
	linenum = lineNum;
	columnnum = columnNum;
	sizertype = detectSizerType();
	guicontrol = new gGUIControl**[linenum];
	for(int i = 0; i < linenum; i++) guicontrol[i] = new gGUIControl*[columnNum];
	iscontrolset = new bool*[linenum];
	for(int i = 0; i < linenum; i++) iscontrolset[i] = new bool[columnNum];
	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			iscontrolset[i][j] = false;
		}
	}

	delete lineprs;
	lineprs = new float[linenum];
	for(int i = 0; i < linenum; i++) lineprs[i] = 1.0f / linenum;

	delete columnprs;
	columnprs = new float[columnnum];
	for(int i = 0; i < columnnum; i++) columnprs[i] = 1.0f / columnnum;

	delete linetprs;
	linetprs = new float[linenum + 1];
	linetprs[0] = 0.0f;
	linetprs[linenum] = 1.0f;
	for(int i = 1; i < linenum; i++) {
		linetprs[i] = 0.0f;
		for(int j = 0; j < i; j++) linetprs[i] += lineprs[j];
	}

	delete columntprs;
	columntprs = new float[columnnum + 1];
	columntprs[0] = 0.0f;
	columntprs[columnnum] = 1.0f;
	for(int i = 1; i < columnnum; i++) {
		columntprs[i] = 0.0f;
		for(int j = 0; j < i; j++) columntprs[i] += columnprs[j];
	}
}

int gGUISizer::getLineNum() {
	return linenum;
}

int gGUISizer::getColumnNum() {
	return columnnum;
}

void gGUISizer::setLineProportions(float* proportions) {
	delete lineprs;
	lineprs = new float[linenum];
	for(int i = 0; i < linenum; i++) lineprs[i] = proportions[i];

	delete linetprs;
	linetprs = new float[linenum + 1];
	linetprs[0] = 0.0f;
	linetprs[linenum] = 1.0f;
	for(int i = 1; i < linenum; i++) {
		linetprs[i] = 0.0f;
		for(int j = 0; j < i; j++) linetprs[i] += lineprs[j];
	}
}

void gGUISizer::setColumnProportions(float* proportions) {
	delete columnprs;
	columnprs = new float[columnnum];
	for(int i = 0; i < columnnum; i++) columnprs[i] = proportions[i];

	delete columntprs;
	columntprs = new float[columnnum + 1];
	columntprs[0] = 0.0f;
	columntprs[columnnum] = 1.0f;
	for(int i = 1; i < columnnum; i++) {
		columntprs[i] = 0.0f;
		for(int j = 0; j < i; j++) columntprs[i] += columnprs[j];
	}
}

void gGUISizer::setSlotPadding(int padding) {
	slotpadding = padding;
}

void gGUISizer::setControl(int lineNo, int columnNo, gGUIControl* guiControl) {
	guicontrol[lineNo][columnNo] = guiControl;
//	guicontrol[lineNo][columnNo]->setParent(this);
//	guicontrol[lineNo][columnNo]->setParentSlotNo(lineNo, columnNo);
//	guicontrol[lineNo][columnNo]->setRootApp(root);
	guicontrol[lineNo][columnNo]->set(root, this, lineNo, columnNo,
			left + (width * columntprs[columnNo]) + slotpadding,
			top + (height * linetprs[lineNo]) + slotpadding,
			(left + (width * columntprs[columnNo + 1])) - (left + (width * columntprs[columnNo])) - (2 * slotpadding),
			(top + (height * linetprs[lineNo + 1])) - (top + (height * linetprs[lineNo])) - (2 * slotpadding)
	);
	iscontrolset[lineNo][columnNo] = true;
//	guicontrol[lineNo][columnNo]->left = left + (width * columntprs[columnNo]);
//	guicontrol[lineNo][columnNo]->top = top + (height * linetprs[lineNo]);
//	guicontrol[lineNo][columnNo]->right = left + (width * columntprs[columnNo + 1]);
//	guicontrol[lineNo][columnNo]->bottom = top + (height * linetprs[lineNo + 1]);
//	guicontrol[lineNo][columnNo]->width = guicontrol[lineNo][columnNo]->right - guicontrol[lineNo][columnNo]->left;
//	guicontrol[lineNo][columnNo]->height = guicontrol[lineNo][columnNo]->bottom - guicontrol[lineNo][columnNo]->top;
}

gGUIControl* gGUISizer::getControl(int lineNo, int columnNo) {
	return guicontrol[lineNo][columnNo];
}

int gGUISizer::getCursor(int x, int y) {
	if(resizable) {
		for(int i = 1; i < columnnum; i++) {
			if(x >= left + (width * columntprs[i]) - 1 && x <= left + (width * columntprs[i]) + 1) return CURSOR_HRESIZE;
		}
		for(int i = 1; i < linenum; i++) {
			if(y >= top + (height * linetprs[i]) - 1 && y <= top + (height * linetprs[i]) + 1) return CURSOR_VRESIZE;
		}
	}

	int cc = -1, cl = -1;
	for(int i = 1; i < linenum + 1; i++) {
		if(y < top + (height * linetprs[i])) {
			cl = i - 1;
			break;
		}
	}
	for(int i = 1; i < columnnum + 1; i++) {
		if(x < left + (width * columntprs[i])) {
			cc = i - 1;
			break;
		}
	}

	if(cl == -1 || cc == -1) return CURSOR_ARROW;

	if(iscontrolset[cl][cc]) {
		int gc = guicontrol[cl][cc]->getCursor(x, y);
		return gc;

	}
	return CURSOR_ARROW;
}

int gGUISizer::detectSizerType() {
	if(linenum == 1) return SIZERTYPE_HORIZONTAL;
	else if(columnnum == 1) return SIZERTYPE_VERTICAL;
	return SIZERTYPE_GRID;
}

void gGUISizer::enableBorders(bool isEnabled) {
	bordersenabled = isEnabled;
}

void gGUISizer::enableResizing(bool isEnabled) {
	resizable = isEnabled;
}

void gGUISizer::enableBackgroundFill(bool isEnabled) {
	fillbackground = isEnabled;
}

void gGUISizer::update() {
//	gLogi("gGUISizer") << "update, c0pr:" << linetprs[1];
//	root->getAppManager()->setCursor(3);
	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(iscontrolset[i][j]) guicontrol[i][j]->update();
		}
	}
}

void gGUISizer::draw() {
//	gLogi("gGUISizer") << "draw";
	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(fillbackground) {
				gColor oldcolor = *renderer->getColor();
				renderer->setColor(*backgroundcolor);
				gDrawRectangle(left + (width * columntprs[j]), top + (height * linetprs[i]), width * columnprs[j], height * lineprs[i], true);
				renderer->setColor(oldcolor);
			}

			if(iscontrolset[i][j]) guicontrol[i][j]->draw();
		}
	}
	if(bordersenabled) {
		gColor oldcolor = *renderer->getColor();
		for(int i = 0; i < linenum; i++) {
			for(int j = 0; j < columnnum; j++) {
				if(j == 0) continue;
				for (int k = 0; k < 3; k++) {
					float lc = foregroundcolor->r - (std::fabs(k - 1) * 0.05f);
					renderer->setColor(gColor(lc, lc, lc));
					int l = left + (width * columntprs[j]) + k - 1;
					gDrawLine(l, top, l, bottom);
				}
			}
			if(i == 0) continue;
			for (int k = 0; k < 3; k++) {
				float lc = foregroundcolor->r - (std::fabs(k - 1) * 0.05f);
				renderer->setColor(gColor(lc, lc, lc));
				int t = top + (height * linetprs[i]) + k - 1;
				gDrawLine(left, t, right, t);
			}
		}
		renderer->setColor(oldcolor);
	}
}

void gGUISizer::keyPressed(int key) {
	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(iscontrolset[i][j] && guicontrol[i][j]->isfocused) guicontrol[i][j]->keyPressed(key);
		}
	}
}

void gGUISizer::keyReleased(int key) {
	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(iscontrolset[i][j] && guicontrol[i][j]->isfocused) guicontrol[i][j]->keyReleased(key);
		}
	}
}

void gGUISizer::charPressed(unsigned int codepoint) {
	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(iscontrolset[i][j] && guicontrol[i][j]->isfocused) guicontrol[i][j]->charPressed(codepoint);
		}
	}
}

void gGUISizer::mouseMoved(int x, int y) {
	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(iscontrolset[i][j]) {
				bool iscursoronold = guicontrol[i][j]->iscursoron;
				if(x >= guicontrol[i][j]->left && x < guicontrol[i][j]->right && y >= guicontrol[i][j]->top && y < guicontrol[i][j]->bottom) {
					guicontrol[i][j]->iscursoron = true;
					if(!iscursoronold) guicontrol[i][j]->mouseEntered();
				} else {
					if (guicontrol[i][j]->iscursoron) {
						guicontrol[i][j]->iscursoron = false;
						if(!iscursoronold) guicontrol[i][j]->mouseExited();
					}
				}
				if(guicontrol[i][j]->iscursoron) guicontrol[i][j]->mouseMoved(x, y);
			}
		}
	}
}

void gGUISizer::mousePressed(int x, int y, int button) {
	if(iscursoron && resizable) {
		for(int i = 1; i < columnnum; i++) {
			if(x >= left + (width * columntprs[i]) - 1 && x <= left + (width * columntprs[i]) + 1) {
				resizecolumn = i;
				resizex = x;
				break;
			}
		}
		if (!resizecolumn) {
			for(int i = 1; i < linenum; i++) {
				if(y >= top + (height * linetprs[i]) - 1 && y <= top + (height * linetprs[i]) + 1) {
					resizeline = i;
					resizey = y;
					break;
				}
			}
		}
	}

	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(iscontrolset[i][j]) {
				bool focusold = guicontrol[i][j]->isfocused;
				guicontrol[i][j]->isfocused = false;
				if(guicontrol[i][j]->iscursoron) {
					guicontrol[i][j]->isfocused = true;
					if(!focusold) root->getCurrentCanvas()->onGuiEvent(id, GUIEVENT_FOCUSED);
					guicontrol[i][j]->mousePressed(x, y, button);
				}
				if(focusold && !guicontrol[i][j]->isfocused) root->getCurrentCanvas()->onGuiEvent(id, GUIEVENT_UNFOCUSED);
			}
		}
	}
}

void gGUISizer::mouseDragged(int x, int y, int button) {
	if(iscursoron && resizecolumn > 0) {
		float prdiff = (float)(x - resizex) / (float)width;
		if(columntprs[resizecolumn] + prdiff < columntprs[resizecolumn - 1] + 0.02f) prdiff = (columntprs[resizecolumn - 1] + 0.02f) - columntprs[resizecolumn];
		if(columntprs[resizecolumn] + prdiff > columntprs[resizecolumn + 1] - 0.02f) prdiff = (columntprs[resizecolumn + 1] - 0.02f) - columntprs[resizecolumn];
		columnprs[resizecolumn] -= prdiff;
		columnprs[resizecolumn - 1] += prdiff;
		for(int i = 1; i < columnnum; i++) {
			columntprs[i] = 0.0f;
			for(int j = 0; j < i; j++) columntprs[i] += columnprs[j];
		}

		for (int i = resizecolumn - 1; i <= resizecolumn; i++) {
			for (int j = 0; j < linenum; j++) {
				if(iscontrolset[j][i]) {
					guicontrol[j][i]->set(
						root,
						this,
						i,
						j,
						left + (width * columntprs[i]),
						top + (height * linetprs[j]),
						width * (columntprs[i + 1] - columntprs[i]),
						height * (linetprs[j + 1] - linetprs[j])
					);
				}
			}
		}

		resizex = x;
		return;
	}

	if(iscursoron && resizeline > 0) {
		float prdiff = (float)(y - resizey) / (float)height;
		if(linetprs[resizeline] + prdiff < linetprs[resizeline - 1] + 0.02f) prdiff = (linetprs[resizeline - 1] + 0.02f) - linetprs[resizeline];
		if(linetprs[resizeline] + prdiff > linetprs[resizeline + 1] - 0.02f) prdiff = (linetprs[resizeline + 1] - 0.02f) - linetprs[resizeline];
		lineprs[resizeline] -= prdiff;
		lineprs[resizeline - 1] += prdiff;
		for(int i = 1; i < linenum; i++) {
			linetprs[i] = 0.0f;
			for(int j = 0; j < i; j++) linetprs[i] += lineprs[j];
		}

		for (int i = resizeline - 1; i <= resizeline; i++) {
			for (int j = 0; j < columnnum; j++) {
				if(iscontrolset[i][j]) {
					guicontrol[i][j]->set(
						root,
						this,
						i,
						j,
						left + (width * columntprs[j]),
						top + (height * linetprs[i]),
						width * (columntprs[j + 1] - columntprs[j]),
						height * (linetprs[i + 1] - linetprs[i])
					);
				}
			}
		}

		resizey = y;
		return;
	}

	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(iscontrolset[i][j] && guicontrol[i][j]->iscursoron) guicontrol[i][j]->mouseDragged(x, y, button);
		}
	}
}

void gGUISizer::mouseReleased(int x, int y, int button) {
	if(iscursoron && resizecolumn > 0) {
		float prdiff = (float)(x - resizex) / (float)width;
		if(columntprs[resizecolumn] + prdiff < columntprs[resizecolumn - 1] + 0.02f) prdiff = (columntprs[resizecolumn - 1] + 0.02f) - columntprs[resizecolumn];
		if(columntprs[resizecolumn] + prdiff > columntprs[resizecolumn + 1] - 0.02f) prdiff = (columntprs[resizecolumn + 1] - 0.02f) - columntprs[resizecolumn];
		columnprs[resizecolumn] -= prdiff;
		columnprs[resizecolumn - 1] += prdiff;
		for(int i = 1; i < columnnum; i++) {
			columntprs[i] = 0.0f;
			for(int j = 0; j < i; j++) columntprs[i] += columnprs[j];
		}

		for (int i = resizecolumn - 1; i <= resizecolumn; i++) {
			for (int j = 0; j < linenum; j++) {
				if(iscontrolset[j][i]) {
					guicontrol[j][i]->set(
						root,
						this,
						i,
						j,
						left + (width * columntprs[i]),
						top + (height * linetprs[j]),
						width * (columntprs[i + 1] - columntprs[i]),
						height * (linetprs[j + 1] - linetprs[j])
					);
				}
			}
		}

		resizecolumn = 0;
		resizex = 0;
		return;
	}

	if(iscursoron && resizeline > 0) {
		float prdiff = (float)(y - resizey) / (float)height;
		if(linetprs[resizeline] + prdiff < linetprs[resizeline - 1] + 0.02f) prdiff = (linetprs[resizeline - 1] + 0.02f) - linetprs[resizeline];
		if(linetprs[resizeline] + prdiff > linetprs[resizeline + 1] - 0.02f) prdiff = (linetprs[resizeline + 1] - 0.02f) - linetprs[resizeline];
		lineprs[resizeline] -= prdiff;
		lineprs[resizeline - 1] += prdiff;
		for(int i = 1; i < linenum; i++) {
			linetprs[i] = 0.0f;
			for(int j = 0; j < i; j++) linetprs[i] += lineprs[j];
		}

		for (int i = resizeline - 1; i <= resizeline; i++) {
			for (int j = 0; j < columnnum; j++) {
				if(iscontrolset[i][j]) {
					guicontrol[i][j]->set(
						root,
						this,
						i,
						j,
						left + (width * columntprs[j]),
						top + (height * linetprs[i]),
						width * (columntprs[j + 1] - columntprs[j]),
						height * (linetprs[i + 1] - linetprs[i])
					);
				}
			}
		}

		resizeline = 0;
		resizey = 0;
		return;
	}

	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			if(iscontrolset[i][j] && guicontrol[i][j]->iscursoron) guicontrol[i][j]->mouseReleased(x, y, button);
		}
	}
}

void gGUISizer::mouseScrolled(int x, int y) {
	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			for(int j = 0; j < columnnum; j++) {
				if(iscontrolset[i][j] && guicontrol[i][j]->iscursoron) guicontrol[i][j]->mouseScrolled(x, y);
			}
		}
	}
}

void gGUISizer::mouseEntered() {

}

void gGUISizer::mouseExited() {
	for(int i = 0; i < linenum; i++) {
		for(int j = 0; j < columnnum; j++) {
			for(int j = 0; j < columnnum; j++) {
				if(iscontrolset[i][j] && guicontrol[i][j]->iscursoron) {
					guicontrol[i][j]->iscursoron = false;
					guicontrol[i][j]->mouseExited();
				}
			}
		}
	}
}

