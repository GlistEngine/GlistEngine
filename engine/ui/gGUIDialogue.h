/*
 * gGUIDialogue.h
 *
 *  Created on: 27 Jul 2022
 *      Author: Umut Can
 */

/* INSTRUCTIONS ON HOW TO USE
 *
 * - After declaring a gGUIDialogue object, pass the same object as an argument to gGUIManager's "setupDialogue(gGUIDialogue* dialogue)" function.
 * This function will initialize the object's dimensions and sizer before pushing it to an array of gGUIDialogue objects. By default, title bar and
 * buttons bar of the same object will also be initialized within the same function. It should be noted that the gGUIDialogue objects pushed later
 * will be drawn on top of the ones pushed earlier (if the objects have the "isdialogueactive" boolean variable set as TRUE).
 *
 * EX. "appmanager->getGUIManager()->setupDialogue(&dialogue);"
 *
 * - In order to draw the gGUIDialogue object's elements on the canvas (or remove the elements from the canvas), we need to call gGUIDialogue's
 * "setIsDialogueActive(bool isDialogueActive)" function.
 *
 * EX. "dialogue.setIsDialogueActive(true);"
 *
 */

#ifndef UI_GGUIDIALOGUE_H_
#define UI_GGUIDIALOGUE_H_

#include "gGUIForm.h"
#include "gGUIContainer.h"
#include "gGUIImageButton.h"
#include "gGUIBitmap.h"
#include "gGUIText.h"

class gGUIDialogue: public gGUIForm {
public:
	static const int EVENT_NONE = 0, EVENT_MINIMIZE = 1, EVENT_MAXIMIZE = 2, EVENT_RESTORE = 3, EVENT_EXIT = 4;
	static const int RESIZE_NONE = 0, RESIZE_LEFT = 1, RESIZE_RIGHT = 2, RESIZE_TOP = 3, RESIZE_BOTTOM = 4;
	enum {
		DIALOGUETYPE_OK,
		DIALOGUETYPE_YESNOCANCEL,
		DIALOGUETYPE_OKCANCEL,
		DIALOGUETYPE_YESNO
	};
	enum {
		TITLETYPE_EXITMINMAX,
		TITLETYPE_EXITMAX,
		TITLETYPE_EXITMIN,
		TITLETYPE_EXIT
	};
	enum {
		ICONTYPE_NONE,
		ICONTYPE_ERROR,
		ICONTYPE_INFO,
		ICONTYPE_WARNING
	};

	gGUIDialogue();
	virtual ~gGUIDialogue();

	void update();
	void draw();

	bool show();
	bool hide();
	bool isShown();

	void setTitleBar(gGUIContainer* titleBar);
	void setButtonsBar(gGUIContainer* buttonsBar);
	void setMessageBar(gGUIContainer* messageBar);
	gGUIContainer* getTitleBar();
	gGUIContainer* getButtonsBar();
	gGUIContainer* getMessageBar();
	void resetTitleBar();
	void resetButtonsBar();
	void resetMessageBar();

	void setTitle(std::string title);
	void setMessageBarSizer(gGUISizer* sizer);

	void setMinimizeButton(gGUIImageButton* minimizeButton);
	void setMaximizeButton(gGUIImageButton* maximizeButton);
	void setExitButton(gGUIImageButton* exitButton);

	void setButtonEvent(int buttonEvent);
	int getButtonEvent();

	void enableDrag(bool isDragEnabled);
	void enableResize(bool isResizeEnabled);

	void setIsMaximized(bool isMaximized);
	void transformDialogue(int left, int top, int width, int height);

	void setMessageText(std::string messageText);
	std::string getMessageText();
	void setIconType(int iconId);
	void setDialogueType(int typeId);
	void setTitleType(int typeId);

	int getCursor(int x, int y);
	void mouseMoved(int x, int y);
	void mousePressed(int x, int y, int button);
	void mouseDragged(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

	int getOKButtonId();
	int getCancelButtonId();
	int getYesButtonId();
	int getNoButtonId();

	gGUIButton* getOKButton();
	gGUIButton* getCancelButton();
	gGUIButton* getYesButton();
	gGUIButton* getNoButton();

private:
	bool isdialogueshown;

	gGUIContainer* titlebar;
	gGUIContainer* buttonsbar;
	gGUIContainer* messagebar;

	gGUIImageButton* minimizebutton;
	gGUIImageButton* maximizebutton;
	gGUIImageButton* exitbutton;

	static const int deftitlebarh = 35, deftitlebarbitmapw = 24, deftitlebarbuttonw = 48;
	static const int defbuttonsbarh = 45, defbuttonsbarbuttonw = 100, defbuttonsbarbuttonh = 27;

	void initDefTitleBar();
	void initDefButtonsBar();
	void initDefMessageBar();

	gGUIContainer deftitlebar;
	gGUISizer deftitlebarsizer;
	gGUIBitmap deftitlebarbitmap;
	gGUIText deftitlebartext;
	gGUIImageButton deftitlebarminimizebutton;
	gGUIImageButton deftitlebarmaximizebutton;
	gGUIImageButton deftitlebarexitbutton;

	gGUIContainer defbuttonsbar;
	gGUISizer defbuttonsbarsizer;
	gGUIButton defbuttonsbarokbutton;
	gGUIButton defbuttonsbaryesbutton;
	gGUIButton defbuttonsbarnobutton;
	gGUIButton defbuttonsbarcancelbutton;

	gGUIContainer defmessagebar;
	gGUISizer defmessagebarsizer;
	gGUIText defmessagetext;
	gGUIImageButton defdialogueicon;

	int buttontrigger, buttonevent;

	int defmessagebartopspace, defmessagebarrightspace;

	int dialoguetype;
	int titletype;

	bool isdragenabled, isresizeenabled;
	bool ismaximized, isdragged;
	bool isiconenabled;

	int dragposx, dragposy, sizeposx, sizeposy;

	int resizeposition;
};

#endif /* UI_GGUIDIALOGUE_H_ */
