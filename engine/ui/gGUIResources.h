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
		ICON_SAVE,
		ICON_PLUS,
		ICON_BRUSH,
		ICON_DELETE,
		ICON_DOWNLOAD,
		ICON_UPLOAD,
		ICON_FONT,
		ICON_HOME,
		ICON_PENLOGO,
		ICON_PICTURE,
		ICON_PRINTER,
		ICON_REPAIR,
		ICON_RETURN,
		ICON_FORWARD,
		ICON_SETTINGS,
		ICON_SPLIT,
		ICON_TEXT,
		ICON_ZOOMIN,
		ICON_ZOOMOUT,
		ICON_SEARCH,
		ICON_PLAY,
		ICON_PAUSE,
		ICON_STOP,
		ICON_FAV,
		ICON_INFO,
		ICON_HELP,
		ICON_COPY,
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
	static const int iconnum = 30;

	int iconw, iconh, iconformat;

	gImage icon[iconnum];

	std::string getBase64IconFile16();
	std::string getBase64IconFolder16();
	std::string getBase64IconFolderOpened16();
	std::string getBase64IconSave16();
	std::string getBase64IconPlus16();
	std::string getBase64IconBrush16();
	std::string getBase64IconDelete16();
	std::string getBase64IconDownload16();;
	std::string getBase64IconUpload16();
	std::string getBase64IconFont16();
	std::string getBase64IconHome16();
	std::string getBase64IconPen16();
	std::string getBase64IconPicture16();
	std::string getBase64IconPrinter16();
	std::string getBase64IconRepair16();
	std::string getBase64IconReturn16();
	std::string getBase64IconForward16();
	std::string getBase64IconSettings16();
	std::string getBase64IconSplit16();
	std::string getBase64IconText16();
	std::string getBase64IconZoomin16();
	std::string getBase64IconZoomout16();
	std::string getBase64IconSearch16();
	std::string getBase64IconPlay16();
	std::string getBase64IconPause16();
	std::string getBase64IconStop16();
	std::string getBase64IconFav16();
	std::string getBase64IconInfo16();
	std::string getBase64IconHelp16();
	std::string getBase64IconCopy16();

	std::string getBase64IconFile40();
	std::string getBase64IconFolder40();
	std::string getBase64IconFolderOpened40();
	std::string getBase64IconSave40();
};

#endif /* UI_GGUIRESOURCES_H_ */
