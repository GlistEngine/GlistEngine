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
#if GLIST_ANDROID || GLIST_IOS
	// Controls sat flush against each other, which on a touch screen reads as one
	// block rather than as separate things to hit. Both axes inset a slot by this
	// much on every side, so the two numbers mean the same thing and a gap between
	// neighbours is twice one of them - 48 units, which on a 720 unit wide design
	// is about a fifteenth of the page between one thing and the next, with half
	// that left as a margin down the edges.
	slotpadding = 24;
	slotheightpadding = 24;
	referenceheight = 0;
#else
	// Desktop keeps its old values: the pointer is precise there and the existing
	// layouts are already drawn around them.
	slotpadding = 2;
	slotheightpadding = 0;
#endif
	alignvertically = false;
	lineprs = nullptr;
	columnprs = nullptr;
	linetprs = nullptr;
	columntprs = nullptr;
	setSize(1, 1);
}

gGUISizer::~gGUISizer() {
	// Allocated by setSize() and never released. Every sizer leaked four arrays.
	delete[] lineprs;
	delete[] columnprs;
	delete[] linetprs;
	delete[] columntprs;
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

#if GLIST_ANDROID || GLIST_IOS
int gGUISizer::getRowTop(int lineNo) {
	if((int)rowtops.size() != linenum) computeRowHeights();
	// A sizer sized to no rows at all leaves these empty, and the size check above
	// is satisfied by two zeroes. Reading the vector then walks off the front of
	// it, so the bounds are checked rather than assumed.
	if(lineNo < 0 || lineNo >= (int)rowtops.size()) return 0;
	return rowtops[lineNo];
}

int gGUISizer::getRowHeight(int lineNo) {
	if((int)rowheights.size() != linenum) computeRowHeights();
	if(lineNo < 0 || lineNo >= (int)rowheights.size()) return 0;
	return rowheights[lineNo];
}
#else
int gGUISizer::getRowTop(int lineNo) {
	return (int)((float)height * linetprs[lineNo]);
}

int gGUISizer::getRowHeight(int lineNo) {
	return (int)((float)height * (linetprs[lineNo + 1] - linetprs[lineNo]));
}
#endif

int gGUISizer::getSlotWidth(int lineNo, int columnNo) { //
	return width * columnprs[columnNo];
}

int gGUISizer::getSlotHeight(int lineNo, int columnNo) { //
	return getRowHeight(lineNo);
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
	return top + getRowTop(lineNo);
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

	delete[] lineprs;
	lineprs = new float[linenum];
	for(int i = 0; i < linenum; i++) {
		lineprs[i] = 1.0f / (float)linenum;
	}

	delete[] columnprs;
	columnprs = new float[columnnum];
	for(int i = 0; i < columnnum; i++) {
		columnprs[i] = 1.0f / (float)columnnum;
	}

	delete[] linetprs;
	linetprs = new float[linenum + 1];
	linetprs[0] = 0.0f;
	linetprs[linenum] = 1.0f;
	for(int i = 1; i < linenum; i++) {
		linetprs[i] = 0.0f;
		for(int j = 0; j < i; j++) {
			linetprs[i] += lineprs[j];
		}
	}

	delete[] columntprs;
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
	// Filled before the old array is released, because the source can be that very
	// array: a sizer with space lines re-proportions itself by calling
	// setLineProportions(lineprs) from windowResized(). Releasing first would leave
	// the loop reading freed memory.
	float* newlineprs = new float[linenum];
	for(int i = 0; i < linenum; i++) {
		newlineprs[i] = proportions[i];
	}
	delete[] lineprs;
	lineprs = newlineprs;

	delete[] linetprs;
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
	// Same aliasing as setLineProportions above: copy first, release after.
	float* newcolumnprs = new float[columnnum];
	for(int i = 0; i < columnnum; i++) {
		newcolumnprs[i] = proportions[i];
	}
	delete[] columnprs;
	columnprs = newcolumnprs;

	delete[] columntprs;
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
#if GLIST_ANDROID || GLIST_IOS
	// The new control can change what its row needs, and that moves every row
	// after it, so the whole sizer is laid out again rather than just this slot.
	reloadControls();
#else
	reloadControl(*control, line, column);
#endif
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
			if(y >= top + getRowTop(i) - 1 && y <= top + getRowTop(i) + 1) {
				return CURSOR_VRESIZE;
			}
		}
	}

	int column = -1;
	int line = -1;
	for(int i = 1; i < linenum + 1; i++) {
		int rowbottom = i < linenum ? getRowTop(i) : getRowTop(i - 1) + getRowHeight(i - 1);
		if(y < top + rowbottom) {
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

#if GLIST_ANDROID || GLIST_IOS
void gGUISizer::setReferenceHeight(int referenceHeight) {
	if(referenceheight == referenceHeight) return;
	referenceheight = referenceHeight;
	reloadControls();
}

int gGUISizer::getRowNaturalHeight(int lineNo) {
	int designheight = (int)((float)referenceheight * lineprs[lineNo]);
	int naturalheight = 0;
	bool hascontrol = false;
	for(int column = 0; column < columnnum; column++) {
		gGUIControl* control = getControl(lineNo, column);
		if(!control) continue;
		hascontrol = true;
		// Anything with a layout of its own is measured against the share of the
		// reference this row would have had, so the floors keep meaning the same
		// thing all the way down the tree instead of collapsing one level in.
		control->setReferenceHeight(designheight);
		int controlheight = control->getNaturalHeight();
		if(controlheight > naturalheight) naturalheight = controlheight;
	}
	// Both insets, since the row now carries one above its control and one below.
	if(naturalheight > 0) naturalheight += slotheightpadding * 2;
	// Every row keeps at least the share the reference gives it, whether or not
	// anything in it has an opinion. Without this floor a row holding something
	// that does not measure itself is starved the moment the page grows: the rows
	// that do measure themselves take the whole of the new height between them and
	// leave nothing over.
	if(designheight > naturalheight) naturalheight = designheight;
	// And a row that holds anything at all is never shorter than the padding it
	// carries. A page with enough rows can drive the reference share below that,
	// and the slot would then be handed a negative height.
	if(hascontrol && naturalheight < slotheightpadding * 2) naturalheight = slotheightpadding * 2;
	return naturalheight;
}

int gGUISizer::getNaturalHeight() {
	int naturalheight = 0;
	for(int line = 0; line < linenum; line++) naturalheight += getRowNaturalHeight(line);
	return naturalheight;
}

void gGUISizer::computeRowHeights() {
	rowheights.assign(linenum, 0);
	rowtops.assign(linenum, 0);
	if(linenum <= 0) return;

	std::vector<int> minimums(linenum, 0);
	std::vector<bool> ispinned(linenum, false);
	for(int line = 0; line < linenum; line++) minimums[line] = getRowNaturalHeight(line);

	// A row whose proportional share is too small for its content is pinned to
	// what it needs; the rest share out what is left, in their own proportions.
	// Pinning shrinks the pool, which can push another row below its own minimum,
	// so this repeats until a pass pins nothing new. At most linenum passes, since
	// every pass that changes anything pins at least one row.
	int remaining = height;
	float flexweight = 0.0f;
	for(int line = 0; line < linenum; line++) flexweight += lineprs[line];
	bool ispinnednew = true;
	while(ispinnednew) {
		ispinnednew = false;
		for(int line = 0; line < linenum; line++) {
			if(ispinned[line] || minimums[line] <= 0) continue;
			float share = flexweight > 0.0f ? (float)remaining * (lineprs[line] / flexweight) : 0.0f;
			// The tolerance matters: at the design height a row's share and its
			// floor are the same number arrived at by two different roundings, and
			// without it they would disagree by a unit and pin the whole page onto
			// the slower path for no reason.
			if(share >= (float)minimums[line] - 1.0f) continue;
			ispinned[line] = true;
			rowheights[line] = minimums[line];
			remaining -= minimums[line];
			flexweight -= lineprs[line];
			ispinnednew = true;
		}
	}
	// Nothing needed pinning, so the old proportional formula is used verbatim
	// rather than merely reproduced. That makes the ordinary case - which is
	// almost every page - not approximately unchanged but bit for bit identical.
	bool isanyrowpinned = false;
	for(int line = 0; line < linenum; line++) {
		if(ispinned[line]) isanyrowpinned = true;
	}
	if(!isanyrowpinned) {
		for(int line = 0; line < linenum; line++) {
			rowtops[line] = (int)((float)height * linetprs[line]);
			rowheights[line] = (int)((float)height * (linetprs[line + 1] - linetprs[line]));
		}
		return;
	}

	// Everything asked for more than there is. The rows keep their minimums and
	// the sizer overflows its box, which is the honest outcome: whoever sized it
	// was told the real natural height and chose not to give it.
	if(remaining < 0) remaining = 0;
	// The flexible rows are measured from a running total rather than one by one,
	// so that truncating each row does not quietly lose a unit per row - nine rows
	// were losing nine units of the page that way.
	float cumulative = 0.0f;
	int used = 0;
	for(int line = 0; line < linenum; line++) {
		if(ispinned[line]) continue;
		cumulative += lineprs[line];
		int end = flexweight > 0.0f ? (int)((float)remaining * (cumulative / flexweight)) : 0;
		rowheights[line] = end - used;
		used = end;
	}

	int rowtop = 0;
	for(int line = 0; line < linenum; line++) {
		rowtops[line] = rowtop;
		rowtop += rowheights[line];
	}
}
#endif

void gGUISizer::reloadControls() {
	if(oldwidth != width || oldheight != height) checkSpaces();
#if GLIST_ANDROID || GLIST_IOS
	computeRowHeights();
#endif
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
	int w = width * (columntprs[column + 1] - columntprs[column]) - slotpadding * 2;
	// Rows are measured rather than merely divided, so that a row holding
	// something with a size of its own is not squeezed below it. On desktop the
	// helpers return the old proportional figures unchanged.
	int y = top + getRowTop(line) + slotheightpadding;
#if GLIST_ANDROID || GLIST_IOS
	// Twice, matching the horizontal padding: a slot is inset by the same amount
	// top and bottom as it is left and right. Subtracting it once left the control
	// flush with the bottom of its row, so one number produced a visible gap
	// sideways and half of one downwards.
	int h = getRowHeight(line) - slotheightpadding * 2;
#else
	int h = getRowHeight(line) - slotheightpadding;
#endif
	// A slot narrower or shorter than its own padding would otherwise be handed a
	// negative size, which controls treat as an enormous unsigned one when they
	// come to allocate or index against it.
	if(w < 0) w = 0;
	if(h < 0) h = 0;
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
				gDrawRectangle(left + (width * columntprs[column]), top + getRowTop(line), width * columnprs[column], getRowHeight(line), true);
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
				int t = top + getRowTop(i) + k - 1;
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
