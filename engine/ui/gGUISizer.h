/*
 * gGUISizer.h
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#ifndef UI_GGUISIZER_H_
#define UI_GGUISIZER_H_

#include "gGUIControl.h"

#include <vector>


class gGUISizer: public gGUIControl {
public:
	static const int SIZERTYPE_HORIZONTAL = 0, SIZERTYPE_VERTICAL = 1, SIZERTYPE_GRID = 2;

	gGUISizer();
	virtual ~gGUISizer();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void set(int x, int y, int w, int h);
	int getSizerType();

	void setSize(int lineNum, int columnNum);
	int getLineNum();
	int getColumnNum();
	void setLineProportions(float* proportions);
	void setColumnProportions(float* proportions);
	void setSlotPadding(int padding, int height = 0);

	void setControl(int lineNo, int columnNo, gGUIControl* guiControl);
	void removeControl(int lineNo, int columnNo);
	gGUIControl* getControl(int lineNo, int columnNo);

	void update();
	void draw();

	void enableBorders(bool isEnabled);
	void enableResizing(bool isEnabled);
	void enableBackgroundFill(bool isEnabled);

	void setAlignContentVertically(bool enabled);
	bool isAlignContentVertically();

	int getCursor(int x, int y);
	void keyPressed(int key);
	void keyReleased(int key);
	void charPressed(unsigned int codepoint);
	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseScrolled(int x, int y);
	void mouseEntered();
	void mouseExited();
	void windowResized(int w, int h);
	/*
	 * Where a row starts relative to the sizer's top, and how tall it is.
	 *
	 * The single answer to "where is row n", used by the slot geometry, the
	 * borders, the background fill and the cursor test alike. They each used to
	 * work the proportions out for themselves, which was fine while that was the
	 * only way rows were laid out - once rows could be measured instead, the
	 * copies disagreed with the controls and the borders were drawn across them.
	 */
	int getRowTop(int lineNo);
	int getRowHeight(int lineNo);

	int getSlotWidth(int lineNo, int columnNo);
	int getSlotHeight(int lineNo, int columnNo);
	int getSlotX(int lineNo, int columnNo);
	int getSlotY(int lineNo, int columnNo);

	bool isControlSet(int lineNo, int columnNo);

#if GLIST_ANDROID || GLIST_IOS
	/*
	 * The sum of what every row needs for its content, which is how tall this
	 * sizer has to be for nothing in it to be squashed. 0 when nothing inside has
	 * an opinion, which is the usual case.
	 *
	 * Recursive: a sizer inside a slot answers for its own rows too.
	 */
	int getNaturalHeight() override;

	// What one row needs: the tallest opinion among its columns, but never less
	// than the share the reference height gives it.
	int getRowNaturalHeight(int lineNo);

	/*
	 * The height this sizer would have if nothing in it needed extra room - the
	 * one the page was designed for, or the screen if that is taller.
	 *
	 * Told rather than learned. An earlier version had the sizer remember the
	 * first height it was ever given, which went wrong in both directions: the
	 * first height arrived before the controls did, so it was measured short and
	 * the rows then overran their box; and once learned it never changed, so a
	 * rotation kept using the old orientation's figure. Being given the number
	 * makes it exact and makes it follow the rotation.
	 */
	void setReferenceHeight(int referenceHeight) override;
	int getReferenceHeight() const { return referenceheight; }
#endif

private:
	struct Entry {
		gGUIControl* control = nullptr;
		bool isset = false;
	};

	int sizertype;
	int linenum, columnnum;
	std::vector<Entry> guicontrols;
	bool bordersenabled;
	float* lineprs;
	float* columnprs;
	float* linetprs;
	float* columntprs;
	bool resizable;
	int resizecolumn, resizeline;
	int resizex, resizey;
	bool fillbackground;
	int slotpadding;
	int slotheightpadding;
	bool alignvertically;

#if GLIST_ANDROID || GLIST_IOS
	// Where each row starts and how tall it is, in units, relative to the sizer's
	// own top. Rebuilt by computeRowHeights() whenever the slots are reloaded.
	// Only mobile lays rows out this way; the desktop path keeps using the
	// proportions directly and is not compiled with any of this.
	std::vector<int> rowtops;
	std::vector<int> rowheights;
	// Row floors are taken from this rather than from the current height, so that
	// growing the page cannot grow the floors and feed itself. See
	// setReferenceHeight().
	int referenceheight;

	void computeRowHeights();
#endif

private:
	int detectSizerType();
	void reloadControls();
	void reloadControl(gGUIControl& control);
	void reloadControl(gGUIControl& control, int line, int column);

	int indexOf(int line, int column) const;

	void checkSpaces();
};

#endif /* UI_GGUISIZER_H_ */
