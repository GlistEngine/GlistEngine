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
	issizer = true;
	sizertype = SIZERTYPE_HORIZONTAL;
	bordersenabled = false;
	resizable = false;
	resizecolumn = 0;
	resizeline = 0;
	fillbackground = false;
	slotpadding = 2;
	slotheightpadding = 0;
	alignvertically = false;
	lineprs = nullptr;
	columnprs = nullptr;
	linetprs = nullptr;
	columntprs = nullptr;
	setSize(1, 1);
}

gGUISizer::~gGUISizer() {
}

void gGUISizer::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	left = x;
	top = y;
	right = x + w;
	bottom = y + h;
	width = w;
	height = h;
//	gLogi("Sizer") << "id:" << id  << ", l:" << left << ", t:" << top << ", w:" << w << ", h:" << h;
	reloadControls();
}

void gGUISizer::set(int x, int y, int w, int h) {
	left = x;
	top = y;
	right = x + w;
	bottom = y + h;
	oldwidth = width;
	if(oldwidth == 0) oldwidth = w;
	oldheight = height;
	if(oldheight == 0) oldheight = h;
	width = w;
	height = h;
//	gLogi("Sizer") << "id:" << id  << ", l:" << left << ", t:" << top << ", w:" << w << ", h:" << h;
	reloadControls();
}

int gGUISizer::getSlotWidth(int lineNo, int columnNo) { //
	return width * columnprs[columnNo];
}

int gGUISizer::getSlotHeight(int lineNo, int columnNo) { //
	return height * lineprs[lineNo];
}

int gGUISizer::getSlotX(int lineNo, int columnNo) {
	int totalwidth = left;

	for(int col = 0; col < columnNo; col++) {
		int colwidth = getSlotWidth(lineNo, col);
		totalwidth += colwidth;
	}

	return totalwidth;
}

int gGUISizer::getSlotY(int lineNo, int columnNo) {
	int totalheight = top;

	for(int row = 0; row < lineNo; row++) {
		int rowheight = getSlotHeight(row, columnNo);
		totalheight += rowheight;
	}

	return totalheight;
}

int gGUISizer::getSizerType() {
	return sizertype;
}

void gGUISizer::setSize(int lineNum, int columnNum) {
	linenum = lineNum;
	columnnum = columnNum;
	sizertype = detectSizerType();

	guicontrols.clear();
	for (int i = 0; i < linenum * columnnum; ++i) {
		guicontrols.push_back({nullptr, false});
	}

	delete lineprs;
	lineprs = new float[linenum];
	for(int i = 0; i < linenum; i++) {
		lineprs[i] = 1.0f / (float)linenum;
	}

	delete columnprs;
	columnprs = new float[columnnum];
	for(int i = 0; i < columnnum; i++) {
		columnprs[i] = 1.0f / (float)columnnum;
	}

	delete linetprs;
	linetprs = new float[linenum + 1];
	linetprs[0] = 0.0f;
	linetprs[linenum] = 1.0f;
	for(int i = 1; i < linenum; i++) {
		linetprs[i] = 0.0f;
		for(int j = 0; j < i; j++) {
			linetprs[i] += lineprs[j];
		}
	}

	delete columntprs;
	columntprs = new float[columnnum + 1];
	columntprs[0] = 0.0f;
	columntprs[columnnum] = 1.0f;
	for(int i = 1; i < columnnum; i++) {
		columntprs[i] = 0.0f;
		for(int j = 0; j < i; j++) {
			columntprs[i] += columnprs[j];
		}
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
	for(int i = 0; i < linenum; i++) {
		lineprs[i] = proportions[i];
	}

	delete linetprs;
	linetprs = new float[linenum + 1];
	linetprs[0] = 0.0f;
	linetprs[linenum] = 1.0f;
	for(int i = 1; i < linenum; i++) {
		linetprs[i] = 0.0f;
		for(int j = 0; j < i; j++) {
			linetprs[i] += lineprs[j];
		}
	}
}

void gGUISizer::setColumnProportions(float* proportions) {
	delete columnprs;
	columnprs = new float[columnnum];
	for(int i = 0; i < columnnum; i++) {
		columnprs[i] = proportions[i];
	}

	delete columntprs;
	columntprs = new float[columnnum + 1];
	columntprs[0] = 0.0f;
	columntprs[columnnum] = 1.0f;
	for(int i = 1; i < columnnum; i++) {
		columntprs[i] = 0.0f;
		for(int j = 0; j < i; j++) {
			columntprs[i] += columnprs[j];
		}
	}
}

void gGUISizer::setSlotPadding(int padding, int height) {
	slotpadding = padding;
	slotheightpadding = height;
}

void gGUISizer::setControl(int line, int column, gGUIControl* control) {
	Entry& entry = guicontrols[indexOf(line, column)];
	entry.control = control;
	entry.isset = true;
	reloadControl(*control, line, column);
}

void gGUISizer::removeControl(int line, int column) {
	guicontrols[indexOf(line, column)].isset = false;
}

gGUIControl* gGUISizer::getControl(int line, int column) {
	Entry& entry = guicontrols[indexOf(line, column)];
	if (!entry.isset) {
		return nullptr;
	}
	return entry.control;
}

int gGUISizer::getCursor(int x, int y) {
	if(resizable) {
		for(int i = 1; i < columnnum; i++) {
			if(x >= left + (width * columntprs[i]) - 1 && x <= left + (width * columntprs[i]) + 1) {
				return CURSOR_HRESIZE;
			}
		}
		for(int i = 1; i < linenum; i++) {
			if(y >= top + (height * linetprs[i]) - 1 && y <= top + (height * linetprs[i]) + 1) {
				return CURSOR_VRESIZE;
			}
		}
	}

	int column = -1;
	int line = -1;
	for(int i = 1; i < linenum + 1; i++) {
		if(y < top + (height * linetprs[i])) {
			line = i - 1;
			break;
		}
	}
	for(int i = 1; i < columnnum + 1; i++) {
		if(x < left + (width * columntprs[i])) {
			column = i - 1;
			break;
		}
	}
	if(line == -1 || column == -1) {
		return CURSOR_ARROW;
	}

	gGUIControl* control = getControl(line, column);
	if(control != nullptr) {
		return control->getCursor(x, y);

	}
	return CURSOR_ARROW;
}

int gGUISizer::detectSizerType() {
	if(linenum == 1) return SIZERTYPE_HORIZONTAL;
	else if(columnnum == 1) return SIZERTYPE_VERTICAL;
	return SIZERTYPE_GRID;
}

void gGUISizer::checkSpaces() {
	std::vector<int> spacelineno;
	std::vector<float> spacelineprs;
	float totalspacelineprs = 0.0f;
	std::vector<int> spacecolumnno;
	std::vector<float> spacecolumnprs;
	float totalspacecolumnprs = 0.0f;
	for (int line = 0; line < linenum; line++) {
		for (int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);

			if (control != nullptr && control->countAsSpace()) {
				// Line check
				if(linenum > 1) {
					bool spacelinealreadyfound = false;
					if(!spacelineno.empty()) {
						for(int spi = 0; spi < spacelineno.size(); spi++) {
							if(spacelineno[spi] == line) {
								spacelinealreadyfound = true;
								break;
							}
						}
					}
					if(!spacelinealreadyfound) {
						spacelineno.push_back(line);
						spacelineprs.push_back(lineprs[line]);
						totalspacelineprs += lineprs[line];
					}
				}

				// Column check
				if(columnnum > 1) {
					bool spacecolumnalreadyfound = false;
					if(!spacecolumnno.empty()) {
						for(int spi = 0; spi < spacecolumnno.size(); spi++) {
							if(spacecolumnno[spi] == column) {
								spacecolumnalreadyfound = true;
								break;
							}
						}
					}
					if(!spacecolumnalreadyfound) {
						spacecolumnno.push_back(column);
						spacecolumnprs.push_back(columnprs[column]);
						totalspacecolumnprs += columnprs[column];
					}
				}
			}
		}
	}

	if(!spacelineno.empty()) {
		int lspacenum = spacelineno.size();
		float ldiffratio = (float)oldheight / (float)height;
		float newtotalnospaceprs = (1.0f - totalspacelineprs) * ldiffratio;
		float newtotalspaceprs = 1.0f - newtotalnospaceprs;
		float lspacediffratio = newtotalspaceprs / totalspacelineprs;

		for (int li = 0; li < linenum; li++) {
			bool isspace = false;
			for (int si = 0; si < lspacenum; si++) {
				if(li == spacelineno[si]) {
					lineprs[li] *= lspacediffratio;
					isspace = true;
					break;
				}
			}
			if(!isspace) lineprs[li] *= ldiffratio;
		}
		setLineProportions(lineprs);
	}

	if(!spacecolumnno.empty()) {
		int cspacenum = spacecolumnno.size();
		float cdiffratio = (float)oldwidth / (float)width;
		float newtotalnospaceprs = (1.0f - totalspacecolumnprs) * cdiffratio;
		float newtotalspaceprs = 1.0f - newtotalnospaceprs;
		float cspacediffratio = newtotalspaceprs / totalspacecolumnprs;

		for (int li = 0; li < columnnum; li++) {
			bool isspace = false;
			for (int si = 0; si < cspacenum; si++) {
				if(li == spacecolumnno[si]) {
					columnprs[li] *= cspacediffratio;
					isspace = true;
					break;
				}
			}
			if(!isspace) columnprs[li] *= cdiffratio;
		}
		setColumnProportions(columnprs);
	}
}

void gGUISizer::reloadControls() {
	if(oldwidth != width || oldheight != height) checkSpaces();
	for (int line = 0; line < linenum; line++) {
		for (int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if (control != nullptr) {
				reloadControl(*control, line, column);
			}
		}
	}
}

void gGUISizer::reloadControl(gGUIControl& control) {
	int column = control.getParentSlotColumnNo();
	int line = control.getParentSlotLineNo();
	reloadControl(control, line, column);
}

void gGUISizer::reloadControl(gGUIControl& control, int line, int column) {
	int x = left + (width * columntprs[column]) + slotpadding;
	int y = top + (height * linetprs[line]) + slotheightpadding;
	int w = width * (columntprs[column + 1] - columntprs[column]) - slotpadding * 2;
	int h = height * (linetprs[line + 1] - linetprs[line]) - slotheightpadding;
	if (alignvertically) {
		int contentheight = control.calculateContentHeight();
		if (contentheight > 0) {
			int diff = (h - contentheight) / 2;
			y += diff;
			h -= diff;
		}
	}
	control.set(root, topparent, this, line, column, x, y, w, h);
}

int gGUISizer::indexOf(int line, int column) const {
	return (line * columnnum) + column;
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

void gGUISizer::setAlignContentVertically(bool enabled) {
	if (alignvertically == enabled) {
		return;
	}
	alignvertically = enabled;
	reloadControls();
}

bool gGUISizer::isAlignContentVertically() {
	return alignvertically;
}

bool gGUISizer::isControlSet(int line, int column) {
	return guicontrols[indexOf(line, column)].isset;
}

void gGUISizer::update() {
//	gLogi("gGUISizer") << "update, c0pr:" << linetprs[1];
//	appmanager->setCursor(3);
	for(int line = 0; line < linenum; line++) {
		for(int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr) {
				control->update();
			}
		}
	}
}

void gGUISizer::draw() {
//	gLogi("gGUISizer") << "draw";
//	gLogi("gGUISizer") << "l:" << left << ", t:" << top << ", w:" << width << ", h:" << height << ", ln:" << linenum << ", cn:" << columnnum;
	for(int line = 0; line < linenum; line++) {
		for(int column = 0; column < columnnum; column++) {
			if(fillbackground) {
				gColor oldcolor = *renderer->getColor();
				renderer->setColor(backgroundcolor);
				gDrawRectangle(left + (width * columntprs[column]), top + (height * linetprs[line]), width * columnprs[column], height * lineprs[line], true);
				renderer->setColor(&oldcolor);
			}

			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled()) {
				control->draw();
			}
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
		renderer->setColor(&oldcolor);
	}
}

void gGUISizer::keyPressed(int key) {
	for(int line = 0; line < linenum; line++) {
		for(int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled() && control->isfocused) {
				control->keyPressed(key);
			}
		}
	}
}

void gGUISizer::keyReleased(int key) {
	for(int line = 0; line < linenum; line++) {
		for(int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled() && control->isfocused) {
				control->keyReleased(key);
			}
		}
	}
}

void gGUISizer::charPressed(unsigned int codepoint) {
	for(int line = 0; line < linenum; line++) {
		for(int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled() && control->isfocused) {
				control->charPressed(codepoint);
			}
		}
	}
}

void gGUISizer::mouseMoved(int x, int y) {
	for(int line = 0; line < linenum; line++) {
		for(int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled()) {
				bool iscursoronold = control->iscursoron;
				if(x >= control->left && x < control->right && y >= control->top && y < control->bottom) {
					control->setCursorOn(true);
					if(!iscursoronold) {
						control->mouseEntered();
					}
				} else {
					if (control->iscursoron) {
						control->setCursorOn(false);
						if(iscursoronold) control->mouseExited();
					}
				}
				if(control->iscursoron) {
					control->mouseMoved(x, y);
				}
			}
		}
	}
}

void gGUISizer::mousePressed(int x, int y, int button) {
//	gLogi("Sizer") << "mousePressed 1, t:" << title;
	if(iscursoron && resizable) {
		for(int column = 1; column < columnnum; column++) {
			if(x >= left + (width * columntprs[column]) - 1 && x <= left + (width * columntprs[column]) + 1) {
				resizecolumn = column;
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

	for(int line = 0; line < linenum; line++) {
		for(int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled()) {
//				bool focusold = control->isfocused;
				control->isfocused = false;
//				gLogi("Sizer") << "mousePressed 21, i:" << i << ", j:" << j << ", x:" << x << ", y:" << y << ", l:" << control->left << ", t:" << control->top << ", r:" << control->right << ", b:" << control->bottom;
				if(control->iscursoron) {
					control->isfocused = true;
					if(!control->iscontainer && !control->issizer) {
						previousfocusid = focusid;
						focusid = control->getId();
					}
					control->mousePressed(x, y, button);
//					if(!focusold) root->getCurrentCanvas()->onGuiEvent(id, GUIEVENT_FOCUSED);
				}
//				if(focusold && !control->isfocused) root->getCurrentCanvas()->onGuiEvent(id, GUIEVENT_UNFOCUSED);
			}
		}
	}
}

void gGUISizer::mouseDragged(int x, int y, int button) {
	if(iscursoron && resizecolumn > 0) {
		float prdiff = (float)(x - resizex) / (float)width;
		if(columntprs[resizecolumn] + prdiff < columntprs[resizecolumn - 1] + 0.02f) {
			prdiff = (columntprs[resizecolumn - 1] + 0.02f) - columntprs[resizecolumn];
		}
		if(columntprs[resizecolumn] + prdiff > columntprs[resizecolumn + 1] - 0.02f) {
			prdiff = (columntprs[resizecolumn + 1] - 0.02f) - columntprs[resizecolumn];
		}
		columnprs[resizecolumn] -= prdiff;
		columnprs[resizecolumn - 1] += prdiff;
		for(int i = 1; i < columnnum; i++) {
			columntprs[i] = 0.0f;
			for(int j = 0; j < i; j++) {
				columntprs[i] += columnprs[j];
			}
		}


		for (int column = resizecolumn - 1; column <= resizecolumn; column++) {
			for (int line = 0; line < linenum; line++) {
				gGUIControl* control = getControl(line, column);
				if(control != nullptr) {
					reloadControl(*control, line, column);
				}
			}
		}

		resizex = x;
		return;
	}

	if(iscursoron && resizeline > 0) {
		float prdiff = (float)(y - resizey) / (float)height;
		if(linetprs[resizeline] + prdiff < linetprs[resizeline - 1] + 0.02f) {
			prdiff = (linetprs[resizeline - 1] + 0.02f) - linetprs[resizeline];
		}
		if(linetprs[resizeline] + prdiff > linetprs[resizeline + 1] - 0.02f) {
			prdiff = (linetprs[resizeline + 1] - 0.02f) - linetprs[resizeline];
		}
		lineprs[resizeline] -= prdiff;
		lineprs[resizeline - 1] += prdiff;
		for(int i = 1; i < linenum; i++) {
			linetprs[i] = 0.0f;
			for(int j = 0; j < i; j++) linetprs[i] += lineprs[j];
		}

		for (int line = resizeline - 1; line <= resizeline; line++) {
			for (int column = 0; column < columnnum; column++) {
				gGUIControl* control = getControl(line, column);
				if(control != nullptr) {
					reloadControl(*control, line, column);
				}
			}
		}

		resizey = y;
		return;
	}


	for (int line = 0; line < linenum; line++) {
		for (int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled() && control->iscursoron) {
				control->mouseDragged(x, y, button);
			}
		}
	}
}

void gGUISizer::mouseReleased(int x, int y, int button) {
	if(iscursoron && resizecolumn > 0) {
		float prdiff = (float)(x - resizex) / (float)width;
		if(columntprs[resizecolumn] + prdiff < columntprs[resizecolumn - 1] + 0.02f) {
			prdiff = (columntprs[resizecolumn - 1] + 0.02f) - columntprs[resizecolumn];
		}
		if(columntprs[resizecolumn] + prdiff > columntprs[resizecolumn + 1] - 0.02f) {
			prdiff = (columntprs[resizecolumn + 1] - 0.02f) - columntprs[resizecolumn];
		}
		columnprs[resizecolumn] -= prdiff;
		columnprs[resizecolumn - 1] += prdiff;
		for(int i = 1; i < columnnum; i++) {
			columntprs[i] = 0.0f;
			for(int j = 0; j < i; j++) {
				columntprs[i] += columnprs[j];
			}
		}

		for (int column = resizecolumn - 1; column <= resizecolumn; column++) {
			for (int line = 0; line < linenum; line++) {
				gGUIControl* control = getControl(line, column);
				if(control != nullptr) {
					reloadControl(*control, line, column);
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

		for (int line = resizeline - 1; line <= resizeline; line++) {
			for (int column = 0; column < columnnum; column++) {
				gGUIControl* control = getControl(line, column);
				if(control != nullptr) {
					reloadControl(*control, line, column);
				}
			}
		}

		resizeline = 0;
		resizey = 0;
		return;
	}

	for (int line = 0; line < linenum; line++) {
		for (int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled()) {
				control->mouseReleased(x, y, button);
			}
		}
	}
}

void gGUISizer::mouseScrolled(int x, int y) {
	for (int line = 0; line < linenum; line++) {
		for (int column = 0; column < columnnum; column++) {
			gGUIControl* control =  getControl(line, column);
			if(control != nullptr && control->isEnabled() && control->iscursoron) {
				control->mouseScrolled(x, y);
			}
		}
	}
}

void gGUISizer::mouseEntered() {

}

void gGUISizer::mouseExited() {
	for (int line = 0; line < linenum; line++) {
		for (int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled() && control->iscursoron) {
				control->setCursorOn(false);
				control->mouseExited();
			}
		}
	}
}

void gGUISizer::windowResized(int w, int h) {
	for (int line = 0; line < linenum; line++) {
		for (int column = 0; column < columnnum; column++) {
			gGUIControl* control = getControl(line, column);
			if(control != nullptr && control->isEnabled()) {
				control->windowResized(w, h);
			}
		}
	}
}
