/*
 * gGUIFrame.cpp
 *
 *  Created on: Aug 11, 2021
 *      Author: noyan
 */

#include "gGUIFrame.h"
#include "gGUISizer.h"
#include "gGUIMenubar.h"
#include "gGUIToolbar.h"
#include "gGUIStatusBar.h"
#include "gGUIContextMenu.h"
#include "gGUITooltipText.h"
#include "gGUITreelist.h"


gGUIFrame::gGUIFrame() {
	guisizer = nullptr;
	menubar = nullptr;
	statusbar = nullptr;
	contextmenu = nullptr;
	treelist = nullptr;
	for(int i = 0; i < vectooltiptext.size(); i++) { vectooltiptext[i] = nullptr;}
	isprogressshown = false;
}

gGUIFrame::gGUIFrame(gBaseApp* root) {
	guisizer = nullptr;
	menubar = nullptr;
	statusbar = nullptr;
	contextmenu = nullptr;
	treelist = nullptr;
	isprogressshown = false;
	setParentSlotNo(0, 0);
	left = 0;
	top = 0;
	right = renderer->getWidth();
	bottom = renderer->getHeight();
	width = renderer->getWidth();
	height = renderer->getHeight();
	setRootApp(root);
}

gGUIFrame::~gGUIFrame() {
}

void gGUIFrame::setup() {
}

void gGUIFrame::update() {
//	gLogi("gGUIFrame") << "update";
	if(guisizer) guisizer->update();
}

void gGUIFrame::draw() {
//	gLogi("gGUIFrame") << "draw";
#if GLIST_ANDROID || GLIST_IOS
	// A GUI app never clears the screen - the interface is expected to cover it,
	// and it always did. While the page is stretched past its end it no longer
	// does: the band shows ground the layout does not reach, and what sits there
	// is whatever the last frame left, which is the bottom of the page a moment
	// ago. That is the flicker of a duplicated page edge during the bounce. The
	// exposed strip is painted over before anything else is drawn.
	if(gRenderer::getOverscrollX() != 0 || gRenderer::getOverscrollY() != 0) {
		gColor* oldcolor = renderer->getColor();
		renderer->setColor(backgroundcolor);
		gDrawRectangle(gRenderer::getScrollX() + gRenderer::getOverscrollX(),
				gRenderer::getScrollY() + gRenderer::getOverscrollY(),
				gRenderer::getUnitViewportWidth(), gRenderer::getUnitViewportHeight(), true);
		renderer->setColor(oldcolor);
	}
#endif
	if(guisizer) guisizer->draw();
	if(toolbarnum > 0) for(int i = 0; i < toolbarnum; i++) toolbars[i]->draw();
	if(verticaltoolbarnum > 0) for(int i = 0; i < verticaltoolbarnum; i++) verticaltoolbars[i]->draw();
	if(menubar) menubar->draw();
	if(statusbar) statusbar->draw();
	if(contextmenu) contextmenu->draw();
	if(treelist) treelist->draw();
	for(int i = 0; i < vectooltiptext.size(); i++) vectooltiptext[i]->draw();
	if(isprogressshown) {
		cprdeg += 2.0f * cpspeed;
		if(cprdeg >= 360.0f) cprdeg -= 360.0f;
		gColor* oldcolor = renderer->getColor();
		renderer->setColor(&cpcolor);
		res.getIconImage(gGUIResources::ICONBIG_LOADING, true)->draw(cpx, cpy, cpw, cph, cprdeg);
		renderer->setColor(oldcolor);
	}
}
