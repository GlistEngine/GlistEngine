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
		ICON_FILE_LIGHT,
		ICON_FOLDER_LIGHT,
		ICON_FOLDEROPENED_LIGHT,
		ICON_SAVE_LIGHT,
		ICON_PLUS_LIGHT,
		ICON_BRUSH_LIGHT,
		ICON_DELETE_LIGHT,
		ICON_DOWNLOAD_LIGHT,
		ICON_UPLOAD_LIGHT,
		ICON_FONT_LIGHT,
		ICON_HOME_LIGHT,
		ICON_PENLOGO_LIGHT,
		ICON_PICTURE_LIGHT,
		ICON_PRINTER_LIGHT,
		ICON_REPAIR_LIGHT,
		ICON_RETURN_LIGHT,
		ICON_FORWARD_LIGHT,
		ICON_SETTINGS_LIGHT,
		ICON_SPLIT_LIGHT,
		ICON_TEXT_LIGHT,
		ICON_ZOOMIN_LIGHT,
		ICON_ZOOMOUT_LIGHT,
		ICON_SEARCH_LIGHT,
		ICON_PLAY_LIGHT,
		ICON_PAUSE_LIGHT,
		ICON_STOP_LIGHT,
		ICON_FAV_LIGHT,
		ICON_INFO_LIGHT,
		ICON_HELP_LIGHT,
		ICON_COPY_LIGHT,
/*		ICON_FILE_DARK,
		ICON_FOLDER_DARK,
		ICON_FOLDEROPENED_DARK,
		ICON_SAVE_DARK,
		ICON_PLUS_DARK,
		ICON_BRUSH_DARK,
		ICON_DELETE_DARK,
		ICON_DOWNLOAD_DARK,
		ICON_UPLOAD_DARK,
		ICON_FONT_DARK,
		ICON_HOME_DARK,
		ICON_PENLOGO_DARK,
		ICON_PICTURE_DARK,
		ICON_PRINTER_DARK,
		ICON_REPAIR_DARK,
		ICON_RETURN_DARK,
		ICON_FORWARD_DARK,
		ICON_SETTINGS_DARK,
		ICON_SPLIT_DARK,
		ICON_TEXT_DARK,
		ICON_ZOOMIN_DARK,
		ICON_ZOOMOUT_DARK*/
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

	std::string getBase64IconFileLight16();
//	std::string getBase64IconFileDark16();
	std::string getBase64IconFolderLight16();
//	std::string getBase64IconFolderDark16();
	std::string getBase64IconFolderOpenedLight16();
	//std::string getBase64IconFolderOpenedDark16();
	std::string getBase64IconSaveLight16();
//	std::string getBase64IconSaveDark16();
	std::string getBase64IconPlusLight16();
//	std::string getBase64IconPlusDark16();
	std::string getBase64IconBrushLight16();
//	std::string getBase64IconBrushDark16();
	std::string getBase64IconDeleteLight16();
//	std::string getBase64IconDeleteDark16();
	std::string getBase64IconDownloadLight16();
//	std::string getBase64IconDownloadDark16();
	std::string getBase64IconUploadLight16();
//	std::string getBase64IconUploadDark16();
	std::string getBase64IconFontLight16();
//	std::string getBase64IconFontDark16();
	std::string getBase64IconHomeLight16();
//	std::string getBase64IconHomeDark16();
	std::string getBase64IconPenLight16();
//	std::string getBase64IconPenDark16();
	std::string getBase64IconPictureLight16();
//	std::string getBase64IconPictureDark16();
	std::string getBase64IconPrinterLight16();
//	std::string getBase64IconPrinterDark16();
	std::string getBase64IconRepairLight16();
//	std::string getBase64IconRepairDark16();
	std::string getBase64IconReturnLight16();
	std::string getBase64IconForwardLight16();
//	std::string getBase64IconForwardDark16();
//	std::string getBase64IconReturnDark16();
	std::string getBase64IconSettingsLight16();
//	std::string getBase64IconSettingsDark16();
	std::string getBase64IconSplitLight16();
//	std::string getBase64IconSplitDark16();
	std::string getBase64IconTextLight16();
//	std::string getBase64IconTextDark16();
	std::string getBase64IconZoominLight16();
//	std::string getBase64IconZoominDark16();
	std::string getBase64IconZoomoutLight16();
//	std::string getBase64IconZoomoutDark16();
	std::string getBase64IconSearchLight16();
	std::string getBase64IconPlayLight16();
	std::string getBase64IconPauseLight16();
	std::string getBase64IconStopLight16();
	std::string getBase64IconFavLight16();
	std::string getBase64IconInfoLight16();
	std::string getBase64IconHelpLight16();
	std::string getBase64IconCopyLight16();

	std::string getBase64IconFile40();
	std::string getBase64IconFolder40();
	std::string getBase64IconFolderOpened40();
	std::string getBase64IconSave40();
};

#endif /* UI_GGUIRESOURCES_H_ */
