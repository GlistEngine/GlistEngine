/*
 * gGUIResources.h
 *
 *  Created on: Sep 25, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIRESOURCES_H_
#define UI_GGUIRESOURCES_H_

#include "gBaseGUIObject.h"
#include "gImage.h"


class gGUIResources {
public:
	static const int ICON_NONE = -1;

	enum {
		ICON_FILE,
		ICON_FOLDER,
		ICON_FOLDEROPENED,
		ICON_SAVE
	};

	gGUIResources();
	virtual ~gGUIResources();

	void initialize();

	int getIconWidth();
	int getIconHeight();
	int getIconFormat();

	int getIconNum();
	gImage* getIconImage(int iconId);

private:
	static const int iconnum = 4;

	int iconw, iconh, iconformat;

	gImage icon[iconnum];

	std::string getBase64IconFile16();
	std::string getBase64IconFolder16();
	std::string getBase64IconFolderOpened16();
	std::string getBase64IconSave16();

	std::string getBase64IconFile40();
	std::string getBase64IconFolder40();
	std::string getBase64IconFolderOpened40();
	std::string getBase64IconSave40();
};

#endif /* UI_GGUIRESOURCES_H_ */
