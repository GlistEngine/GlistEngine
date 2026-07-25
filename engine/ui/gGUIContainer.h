/*
 * gGUIContainer.h
 *
 *  Created on: Aug 20, 2021
 *      Author: noyan
 */

#ifndef UI_GGUICONTAINER_H_
#define UI_GGUICONTAINER_H_

#include "gGUIScrollable.h"
#include "gGUISizer.h"


class gGUIContainer: public gGUIScrollable {
public:
	gGUIContainer();
	virtual ~gGUIContainer();

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) override;
	void set(int x, int y, int w, int h);

	void setSizer(gGUISizer* guiSizer);
	gGUISizer* getSizer();

	/**
	 * Sets the extent of the container's content in GUI units.  The visible
	 * rectangle remains the size assigned by its parent; overflow is handled by
	 * this container's gGUIScrollable base class.
	 */
	void setContentSize(int contentWidth, int contentHeight);
	void enableContentScrolling(bool enabled);
	bool isContentScrollingEnabled() const;

	int getCursor(int x, int y) override;
	void keyPressed(int key) override;
	void keyReleased(int key) override;
	void charPressed(unsigned int codepoint) override;
	void mouseMoved(int x, int y) override;
	void mousePressed(int x, int y, int button) override;
	void mouseDragged(int x, int y, int button) override;
	void mouseReleased(int x, int y, int button) override;
	void mouseScrolled(int x, int y) override;
	void windowResized(int w, int h) override;

	void update() override;
	void draw() override;
	void drawContent() override;

	void setCursorOn(bool isOn) override;


protected:
	gGUISizer* guisizer, temporaryemptysizer;
	int topbarh;
	int contentwidth, contentheight;
	bool contentscrollingenabled;
	bool iscontentdragging;
	bool iscontentdragmoved;
	int contentdragstartx, contentdragstarty;
	int contentdragscrollx, contentdragscrolly;

private:
	static const int CONTENT_DRAG_SLOP;
};

#endif /* UI_GGUICONTAINER_H_ */
