/*
 * gGUIResources.h
 *
 *  Created on: Sep 25, 2021
 *      Author: noyan
 */

#ifndef UI_GGUIRESOURCES_H_
#define UI_GGUIRESOURCES_H_

#include "gObject.h"
class gTexture;

/**
 * gGUIResources keeps all images in base64 format that the programmer can use
 * in their software thanks to their getter functions. We keep this data as
 * base64 because asset images may be lost or corrupted in the file.This results
 * in loss of image and bad experience for a developer. All the images have
 * their own getter function and this getter functions return base64 string
 * value of the image.
 *
 * There are 2 types of getter functions in this class. The first of them is
 * getBase64Icon... function. This function provides us to get base64 string of
 * an image. Also every image has their own getBase64Icon... function. We use
 * these functions inside the initialize function as parameters of the decode
 * function. gDecodeBase64 is a function that converts the base64 string sent
 * into the decode function to image data. Thanks to initialize function, we can
 * add image that we decoded to gImage array that we created.Thus, we will be
 * able to use any image we want in other classes.
 *
 *
 * HOW TO USE
 * - First of all, you have to create a getter function that containing its own
 * name in gGUIResources.h file. (Ex :std::string getBase64IconFile16(); )
 *
 * - After that, you have to enumeration procces to determine id of image which
 * you want to add. (Ex : ICON_FILE,(in the enum))
 *
 * - You have to define the body of your getter function in gGUIResources.cpp.
 * And this function have to return base64 string of your image.(You can encode
 * your image by using encode function of GlistEngine)
 *
 * - Examples of encoding procces in a canvas :
 * img.loadImage("Copy.png");
 * gLogi("base") << "base64img:" << gEncodeBase64(img.getImageData(),
 * img.getWidth() * img.getHeight() * img.getComponentNum());
 *
 * - After you define body, go to initialize function and set your image to array
 * by using id of your image.Don't forget that you have to use decode function
 * inside setImageData function. Also, gDecodeBase64 have to  contain getter
 * function of your image. (Examples avaible below)
 *
 * - After we add the image to array, we can use this image in every class we
 * want. To be able to use it, we have to create an object from gGUIResources
 * class (Ex: gGUIResources res;) and have to use getIconImage method of this
 * object  ( Ex : res.getIconImage(gGUIResources::The id which you enumeration)).
 * Don't forget to add res.initialize(); function to the constructor of your class.
 * This function makes the initialize function of gGUIResources class work.
 *
 * To draw this use draw function (Ex : res.getIconImage(id)->draw(....). If you
 * want the users to choose the images by using your class. You have to define
 * imageid variable as a parameter of getIconImage in your class. Define a
 * function that get id information from developer user to your class and
 * defines it into the iconid variable in your class. Thus, users can use the
 * icons which you add by using the function you define to get id from users in
 * your class.
 */
class gGUIResources : public gObject {
public:
	static const int ICON_NONE;
	// In order to use the pictures in the classes we have created, we perform
	// the enumuration process and determine their ids.
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
		ICON_WARNING,
		ICON_ERROR,
		ICON_CALENDAR,
		ICON_CLOCK,
		ICON_DOWN,
		ICON_UP,
		ICON_MINIMIZEBLACK,
		ICON_MAXIMIZEBLACK,
		ICON_RESTOREBLACK,
		ICON_EXITBLACK,
		ICON_MINIMIZEWHITE,
		ICON_MAXIMIZEWHITE,
		ICON_RESTOREWHITE,
		ICON_EXITWHITE,
		COLORPICKER
	};
	//bigicon
	enum {
		ICONBIG_ERROR,
		ICONBIG_INFO,
		ICONBIG_WARNING
	};

	gGUIResources();
	virtual ~gGUIResources();

	void initialize();
	bool isInitialized();

	int getIconWidth();
	int getIconHeight();
	int getIconFormat();

	int getIconNum();
	gTexture* getIconImage(int iconId, bool isIconBig = false);

private:
	static const int iconnum;
	static const int bigiconnum;

	bool isinitialized;
	int iconw, iconh, bigiconw, bigiconh, iconformat;
	// The array we keep pictures
	gTexture* icon;
	gTexture* bigicon;
	// Getter functions where we keep images in base64 format
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
	std::string getBase64IconWarning16();
	std::string getBase64IconError16();
	std::string getBase64IconCalendar16();
	std::string getBase64IconClock16();
	std::string getBase64IconDown16();
	std::string getBase64IconUp16();

	std::string getBase64IconMinimizeBlack16();
	std::string getBase64IconMaximizeBlack16();
	std::string getBase64IconRestoreBlack16();
	std::string getBase64IconExitBlack16();
	std::string getBase64IconMinimizeWhite16();
	std::string getBase64IconMaximizeWhite16();
	std::string getBase64IconRestoreWhite16();
	std::string getBase64IconExitWhite16();
	std::string getBase64ColorPicker();

	std::string getBase64IconFile40();
	std::string getBase64IconFolder40();
	std::string getBase64IconFolderOpened40();
	std::string getBase64IconSave40();

	std::string getBase64IconBigError64();
	std::string getBase64IconBigInfo64();
	std::string getBase64IconBigWarning64();
};

#endif /* UI_GGUIRESOURCES_H_ */
