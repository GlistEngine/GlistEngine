/*
 * gGUIStatusBar.cpp
 *
 *  Created on: 27 Jul 2022
 *      Author: Batuhan Sarihan
 */

#include "gGUIStatusBar.h"
#include "gBaseCanvas.h"
#include "gBaseApp.h"

gGUIStatusBar::gGUIStatusBar() {

	sizerrescaling = true;
	selectedtext = -1;
	statusbarw = getScreenWidth();
	statusbarh = 30;
	statusbarx = 0;
	statusbary = getScreenHeight() - statusbarh;
	//top = getScreenHeight() - statusbarh;
	setSizer(&statussizer);
	//text font
	textx[0] = 15;
	textx[1] = statusbarx + statusbarw / 2;
	textx[2] = textx[1] + statusbarw / 6;
	textx[3] = textx[2] + statusbarw / 6;
	for (int order = 0; order < textobjectsize; ++order) {
		text[order] = "Example";
		texty[order] = statusbary + statusbarh / 2;
		textsliderx[order] = textx[order] - 10;
		textslidery[order] = statusbary;
		textsliderw[order] = 5;
		textsliderh[order] = statusbarh;
	}
}
gGUIStatusBar::~gGUIStatusBar() {

}

void gGUIStatusBar::draw() {
		//update coordinates for any change of screen resolution
		gColor oldcolor = *renderer->getColor();
		renderer->setColor(foregroundcolor);
		statusbarDraw();
		statusbarAllTextDraw();
		if(guisizer) guisizer->draw();
		renderer->setColor(&oldcolor);
}

void gGUIStatusBar::keyPressed(int key) {
	//gLogi("gGUIStatusBar") << key;
}

void gGUIStatusBar::keyReleased(int key) {

}

void gGUIStatusBar::windowResized(int w, int h) {
 //gLogi("width: ") << w;
 //gLogi("height ") << h;
 updateStatusBarCoordinate(w, h);
 updateTextCoordinate(w, h);
}

void gGUIStatusBar::updateStatusBarCoordinate(int w, int h) {
	 //update necessary coordinate update x and h not needed
	 statusbary = h - statusbarh;
	 statusbarw = getScreenWidth();
	 statusbarx = 0;
	 statusbary = getScreenHeight() - statusbarh;
}

void gGUIStatusBar::updateTextCoordinate(int w, int h) {
 	for (int order = 0; order < textobjectsize; ++order) {
 		textx[0] = 15;
 		textx[1] = statusbarx + statusbarw / 2;
 		textx[2] = textx[1] + statusbarw / 6;
 		textx[3] = textx[2] + statusbarw / 6;
 		texty[order] = statusbary + statusbarh / 2;
 		textsliderx[order] = textx[order];
 		textslidery[order] = statusbary;
 	}
}

void gGUIStatusBar::statusbarDraw() {
	gDrawRectangle(statusbarx, statusbary, statusbarw, statusbarh, true);
	renderer->setColor(middlegroundcolor->r, middlegroundcolor->g, middlegroundcolor->b);
	//ust cizgi
	gDrawLine(statusbarx, statusbary, statusbarx + statusbarw, statusbary);
	//alt cizgi
	gDrawLine(statusbarx, statusbary + statusbarh, statusbarx + statusbarw, statusbary+ statusbarh);
}

void gGUIStatusBar::statusbarAllTextDraw() {
	//statusbarsliders
	renderer->setColor(middlegroundcolor->r, middlegroundcolor->g, middlegroundcolor->b);
	for (int id = 0; id < textobjectsize; ++id) {
		if(text[id].empty())continue;
		//çizgi
		gDrawLine(textsliderx[id], textslidery[id] + textsliderh[id]* 0.05f, textsliderx[id], textslidery[id] + textsliderh[id]*0.95f);
		//statusbartexts
		font->drawText(text[id], textx[id], texty[id]);
	}
}

void gGUIStatusBar::mousePressed(int x, int y, int button) {
	//gLogi("x: ") << x;
	//gLogi("y: ") << y;
	//gLogi("textx: ") << textx[0];
	//gLogi("texty: ") << texty[0];
	for (int id = 0; id < textobjectsize; ++id) {
		if(x > textsliderx[id] - 5 && x < textsliderx[id] + textsliderw[id] + 5 && y > statusbary && y < statusbary + statusbarh) {
			selectedtext = id;
			textsliderx[selectedtext] = x;
			textx[selectedtext] = x + 10;
		}
	}
}

void gGUIStatusBar::mouseDragged(int x, int y, int button) {
	//gLogi("dragx: ") << x;
	//gLogi("dragy: ") << y;
	if(selectedtext != -1) {
		textsliderx[selectedtext] = x;
		textx[selectedtext] = x + 10;
	}
}

void gGUIStatusBar::mouseReleased(int x, int y, int button) {
	//gLogi("releasex: ") << x;
	//gLogi("releasey") << y;
	if(selectedtext != -1) {
		textsliderx[selectedtext] = x;
		textx[selectedtext] = x + 10;
		selectedtext = -1;
	}
}

