/*
 * gGUITimebox.cpp
 *
 *  Created on: 4 Aug 2022
 *      Author: Baris Karakaya
 */

#include "gGUITimebox.h"
#include "gGUISlider.h"
#include "gBaseCanvas.h"
#include "gBaseApp.h"


gGUITimebox::gGUITimebox() {
	//Boxes starting and finishing coordination
#if GLIST_ANDROID || GLIST_IOS
	// Widen the frame on mobile: at 170 the three digit boxes plus the colons and
	// the up/down button do not fit, so two-digit values render clipped. Everything
	// the control draws (button, triangles, sizer width) is derived from timeboxw,
	// so a wider value scales the whole layout consistently.
	timeboxw = 260;
#else
	timeboxw = 170;
#endif
	timeboxh = 60;
	buttoncoverw = 25;
	buttoncoverh = 30;
	buttoncoverx = timeboxw - buttoncoverw - 2;
	buttoncovery = top + 24;
	//Triangles x coordinations
	triangleax = timeboxw - (buttoncoverw / 2) - 7;
	trianglebx = timeboxw - (buttoncoverw / 2) - 12;
	trianglecx = timeboxw - (buttoncoverw / 2) - 2;
	//Triangles y coordinations
	triangleay1 = buttoncovery + 4;
	triangleby1 = buttoncovery + 14;
	trianglecy1 = buttoncovery + 14;
	triangleay2 = buttoncovery + 27;
	triangleby2 = buttoncovery + 17;
	trianglecy2 = buttoncovery + 17;

	bluecolor = gColor(0.1f, 0.45f, 0.87f);
	whitecolor = gColor(1.0f, 1.0f, 1.0f);

	//Begining values..
	hour = 10;
	minute = 45;
	second = 15;

	isdisabled = false;
	ispressedb1 = false;
	ispressedb2 = false;
	ispressednow = false;
	istoggle = false;
	ishour = true;
	isminute = false;
	issecond = false;

	buttontype = 0;
	//Sizer & Controller Options...
	lineno = 3;
	columno = 6;
	timeboxsizer.setSize(lineno, columno);
	float lineprops[] = {0.27f, 0.25f, 0.2f};
	// One proportion per column: setSize(lineno, columno) made columno=6, so the
	// array has to hold 6 entries. It previously had 5, and setColumnProportions
	// copies columno of them - reading one past the end (undefined behaviour). The
	// trailing column takes the remaining width.
#if GLIST_ANDROID || GLIST_IOS
	// On mobile the inner sizer is constrained to timeboxw, so the desktop's
	// 0.1-width digit columns come out too narrow for a two-digit value and the
	// hour/minute/second render clipped to a single digit. Widen the three digit
	// columns to ~0.18 (about 47px at timeboxw 260) so "00".."59" always fit with
	// margin, and leave the right ~43% for the up/down button. The colon separators
	// are positioned from the resulting box geometry in draw().
	float columnprops[] = {0.02f, 0.18f, 0.18f, 0.18f, 0.01f, 0.43f};
#else
	float columnprops[] = {0.040f, 0.1f, 0.1f, 0.1f, 0.025f, 0.635f};
#endif
	timeboxsizer.setColumnProportions(columnprops);
	timeboxsizer.setLineProportions(lineprops);
	timeboxsizer.enableBorders(false);
#if GLIST_ANDROID || GLIST_IOS
	// Tight internal packing, not the page's 24 unit finger gap: the hour, minute
	// and second boxes have to line up inside the frame this control draws around
	// them (left + 5, top + 10, timeboxw x timeboxh). At 24 they are pushed out of
	// that frame and off the up/down triangles. See the same note in gGUINumberBox.
	timeboxsizer.setSlotPadding(2, 0);
#endif
	hourbox.setNumeric(true);
	minutebox.setNumeric(true);
	secondbox.setNumeric(true);
#if GLIST_ANDROID || GLIST_IOS
	// On mobile the time is set with the up/down triangles this control draws, not by
	// typing. Leaving the three inner digit boxes editable meant one tap put all three
	// nested textboxes into edit mode at once and each asked for the soft keyboard -
	// the wrong interaction, and the source of the crash / voice-input menu seen on
	// device. Make the digits display-only; the triangles still change the values.
	hourbox.setEditable(false);
	minutebox.setEditable(false);
	secondbox.setEditable(false);
#endif
	timeboxsizer.setControl(1, 1, &hourbox);
	timeboxsizer.setControl(1, 2, &minutebox);
	timeboxsizer.setControl(1, 3, &secondbox);
	setSizer(&timeboxsizer);
}

gGUITimebox::~gGUITimebox() {

}

void gGUITimebox::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo,int parentSlotColumnNo, int x, int y, int w, int h) {
	totalh = h;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	gGUIScrollable::setDimensions(w, h);
#if GLIST_ANDROID || GLIST_IOS
	// The frame (draw: left + 5, top + 10, timeboxw x timeboxh) and the colon
	// separators are laid out for a timeboxw-wide control. Handing the inner sizer
	// the full slot width - which on a page is the whole screen - spreads the hour,
	// minute and second boxes across it, far from the frame drawn around them.
	// Constrain the sizer to the frame's width so the boxes stay inside it.
	guisizer->set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y + topbarh, timeboxw, h - topbarh);
#else
	guisizer->set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y + topbarh, w, h - topbarh);
#endif
#if GLIST_ANDROID || GLIST_IOS
	// Show the starting time. Done here, after the inner textboxes are positioned by
	// the sizer above - setText recomputes letter positions, so it must run on a box
	// that is already set(), not from the constructor. Mobile only: on desktop the
	// digit boxes are typed into directly, and rewriting them on every relayout
	// would overwrite what the user typed with the triangle-tracked values.
	hourbox.setText(gToStr(hour));
	minutebox.setText(gToStr(minute));
	secondbox.setText(gToStr(second));
#endif
}

void gGUITimebox::set(int x, int y, int w, int h) {
	left = x;
	top = y;
	right = x + w;
	bottom = y + h;
	width = w;
	height = h;
#if GLIST_ANDROID || GLIST_IOS
	guisizer->set(x, y + topbarh, timeboxw, h - topbarh);
#else
	guisizer->set(x, y + topbarh, w, h - topbarh);
#endif
}

void gGUITimebox::keyPressed(int key) {
	hourbox.keyPressed(key);
	minutebox.keyPressed(key);
	secondbox.keyPressed(key);
}

void gGUITimebox::keyReleased(int key) {
	hourbox.keyReleased(key);
	minutebox.keyReleased(key);
	secondbox.keyReleased(key);
}

void gGUITimebox::charPressed(unsigned int codepoint) {
	hourbox.charPressed(codepoint);
	minutebox.charPressed(codepoint);
	secondbox.charPressed(codepoint);
}

void gGUITimebox::mousePressed(int x, int y, int button) {
	gGUIContainer::mousePressed(x, y, button);
	hourbox.mousePressed(x, y, button);
	minutebox.mousePressed(x, y, button);
	secondbox.mousePressed(x, y, button);
	if(isdisabled) return;

	// Tapping a digit field selects which of hour/minute/second the up/down triangles
	// act on. The x ranges below have to line up with where the boxes are actually
	// drawn; on mobile that is the widened timeboxw layout (hour ~[5,41],
	// minute ~[41,78], second ~[78,115]), and the whole box height is tappable.
#if GLIST_ANDROID || GLIST_IOS
	int selhourl = left + 5,   selhourr = left + 41;
	int selminl  = left + 41,  selminr  = left + 78;
	int selsecl  = left + 78,  selsecr  = left + 115;
	int seltop = top,          selbot   = top + timeboxh;
#else
	int selhourl = left + 20,  selhourr = left + 50;
	int selminl  = left + 60,  selminr  = left + 90;
	int selsecl  = left + 100, selsecr  = left + 130;
	int seltop = top + buttoncovery, selbot = top + buttoncovery + 25;
#endif
	if(x >= selhourl && x < selhourr && y >= seltop && y < selbot){
		ishour = true;
		isminute = false;
		issecond = false;
	}

	if(x >= selminl && x < selminr && y >= seltop && y < selbot){
		ishour = false;
		isminute = true;
		issecond = false;
	}

	if(x >= selsecl && x < selsecr && y >= seltop && y < selbot){
		ishour = false;
		isminute = false;
		issecond = true;
	}


	if(x >= left + trianglebx && x < left + trianglecx && y >= top + buttoncovery + 4 && y < top + buttoncovery + 14) {
		buttontype = 1;
			if(!ispressedb1 && ishour) {
				ispressedb1 = true;
				ispressednow = true;
				setHour();
			}

			if(!ispressedb1 && isminute) {
				ispressedb1 = true;
				ispressednow = true;
				setMinute();
			}

			if(!ispressedb1 && issecond) {
				ispressedb1 = true;
				ispressednow = true;
				setSecond();
			}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}

	if(x >= left + trianglebx && x < left + trianglecx && y >= top + buttoncovery + 17 && y < top + buttoncovery + 27) {
		buttontype = 2;
			if(!ispressedb2 && ishour) {
				ispressedb2 = true;
				ispressednow = true;
				setHour();
			}
			if(!ispressedb2 && isminute) {
				ispressedb2 = true;
				ispressednow = true;
				setMinute();
			}
			if(!ispressedb2 && issecond) {
				ispressedb2 = true;
				ispressednow = true;
				setSecond();
			}
		root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONPRESSED);
	}
}

void gGUITimebox::mouseReleased(int x, int y, int button) {
	gGUIContainer::mouseReleased(x, y, button);
	hourbox.mouseReleased(x, y, button);
	minutebox.mouseReleased(x, y, button);
	secondbox.mouseReleased(x, y, button);
	if(isdisabled) return;
#if GLIST_ANDROID || GLIST_IOS
	// The desktop condition below can never be true with the mobile timeboxw: it
	// needs x to be both >= timeboxw/2 - 14 (116) and < left + 12, so the release
	// branch - and with it the G_GUIEVENT_BUTTONRELEASED notification - never ran.
	// Test the area the up/down button is actually drawn in.
	bool releasedonbutton = x >= left + timeboxw - buttoncoverw - 6 && x < left + timeboxw - 6
			&& y >= top + buttoncovery && y < top + buttoncovery + buttoncoverh;
#else
	bool releasedonbutton = x >= timeboxw / 2 - 14 && x < left + 12 && y >= top + 9 && y < top + 12;
#endif
	if(releasedonbutton) {
		if(!istoggle) {
			ispressedb1 = false;
			ispressedb2 = false;
		}
				else {
					if(!ispressednow) {
						ispressedb1 = false;
						ispressedb2 = false;
					}
				}
				root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_BUTTONRELEASED);
	} else {
		if(!istoggle) {
			ispressedb1 = false;
			ispressedb2 = false;
		}
	}
	ispressednow = false;
}

void gGUITimebox::mouseDragged(int x, int y, int button) {
	hourbox.mouseDragged(x, y, button);
	minutebox.mouseDragged(x, y, button);
	secondbox.mouseDragged(x, y, button);
}

void gGUITimebox::update() {
	hourbox.update();
	minutebox.update();
	secondbox.update();
}

#if GLIST_ANDROID || GLIST_IOS
int gGUITimebox::getNaturalHeight() {
	return timeboxh;
}
#endif

void gGUITimebox::draw() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(middlegroundcolor);
	gDrawRectangle(left + 5, top + 10, timeboxw, timeboxh,true);
	renderer->setColor(bluecolor);
	gDrawRectangle(left + timeboxw - buttoncoverw - 6, top + buttoncovery, buttoncoverw, buttoncoverh,true);
	renderer->setColor(whitecolor);
	gDrawTriangle(left + triangleax, top + triangleay1 + ispressedb1, left + trianglebx, top + triangleby1 + ispressedb1, left + trianglecx,
			top + trianglecy1 + ispressedb1, true);
	gDrawTriangle(left + triangleax, top + triangleay2 + ispressedb2 ,left + trianglebx, top + triangleby2 + ispressedb2, left + trianglecx,
			top + trianglecy2 + ispressedb2, true);

	if(ispressednow){
		renderer->setColor(middlegroundcolor);
		if(buttontype == 1){
			gDrawTriangle(left + triangleax, top + triangleay1 + ispressedb1, left + trianglebx, top + triangleby1 + ispressedb1, left + trianglecx,
					top + trianglecy1 + ispressedb1, true);
		}
		if(buttontype == 2){
			gDrawTriangle(left + triangleax, top + triangleay2 + ispressedb2, left + trianglebx, top + triangleby2 + ispressedb2, left + trianglecx,
					top + trianglecy2 + ispressedb2, true);
		}
	}
	renderer->setColor(fontcolor);
#if !(GLIST_ANDROID || GLIST_IOS)
	font->drawText(":", 51.2, 69);
	font->drawText(":", 89, 69);
#endif
	font->drawText(title, left + 2, top + 5);
	if(guisizer) guisizer->draw();
#if GLIST_ANDROID || GLIST_IOS
	// Colons are placed from the actual box geometry, so they stay in the gaps
	// whatever the mobile column widths work out to, and drawn after the sizer so the
	// opaque digit boxes never paint over them. Vertically centred on the digit row.
	renderer->setColor(fontcolor);
	int coy = hourbox.top + hourbox.height / 2 + (int)(font->getSize() / 2);
	font->drawText(":", (hourbox.right + minutebox.left) / 2 - 2, coy);
	font->drawText(":", (minutebox.right + secondbox.left) / 2 - 2, coy);
#endif
	renderer->setColor(oldcolor);
}

void gGUITimebox::setHour(){
	if(ispressednow && buttontype == 1){
		if(hour >= 23){
			hour = -1;
		}
		hour++;
		hourbox.setText(gToStr(hour));
	}

	if (ispressednow && buttontype == 2){
		if(hour <= 0){
			hour = 24;
		}
		hour--;
		hourbox.setText(gToStr(hour));
	}
}

void gGUITimebox::setMinute(){
	if(ispressednow && buttontype == 1){
		if(minute >= 59){
			minute = -1;
		}
		minute++;
		minutebox.setText(gToStr(minute));
	}

	if (ispressednow && buttontype == 2){
		if(minute <= 0){
			minute = 60;
		}
		minute--;
		minutebox.setText(gToStr(minute));
	}
}

void gGUITimebox::setSecond(){
	if(ispressednow && buttontype == 1){
		if(second >= 59){
			second = -1;
		}
		second++;
		secondbox.setText(gToStr(second));
	}

	if (ispressednow && buttontype == 2){
		if(second <= 0){
			second = 60;
		}
		second--;
		secondbox.setText(gToStr(second));
	}
}
