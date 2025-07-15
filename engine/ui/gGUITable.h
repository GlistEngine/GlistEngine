/*
 * gGUITable.h
 *
 *  Created on: 28 Tem 2023
 *       Author: Nazim Gultekin
 */

#ifndef UI_GGUITABLE_H_
#define UI_GGUITABLE_H_

#include "gImage.h"
#include "gGUIControl.h"
#include "gFont.h"
#include "gBaseGUIObject.h"
#include "gColor.h"
#include "gGUIScrollable.h"
#include "gGUIResources.h"
#include <ghc/filesystem.hpp>
namespace fd = ghc::filesystem;

class gGUITable : public gGUIScrollable{
public:
	gGUITable();
	virtual ~gGUITable();
	void setup();
	void update();
    /*
     * Draw the entire table including background, selected, and moved rectangles,
     * as well as the images and file names.
     */
	void draw();
	void drawIcons();
	void mouseMoved(int x, int y );
	void mousePressed(int x, int y, int button);
	void keyPressed(int key);

	void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);
	void addIcon(int iconId, const std::string& title);
	void setFontColor(gColor color);
	void setTableColor(gColor color);
	void setSelectedTable();

	gColor* getTableColor();
	gColor* getFontColor();
	void getSelectedTable();
	int setSelectedFileNo(int selectedFileNo);
	int getSelectedFileNo();

	void setPath(std::string path);
    /*
     * Set the table to display the contents of the specified directory.
     * Clears the current image and text lists and populates them with
     * the contents of the directory.
     */
	void setDirectoryToTable();
    /*
     * Sets the state of the previous button.
     *
     * param @isPreviousPressed Pointer to the state of the previous button.
     *                          Indicates whether the previous button is pressed.
     */
	void setPreviousButton(bool* isPreviousPressed);

private:
	void generateFile();
    /*
     * Draw the background rectangle of the table.
     */
    void drawBackground();
    /*
     * Draw the rectangle highlighting the selected file.
     */
    void drawSelectedRectangle();
    /*
     * Draw the rectangle indicating a moved file.
     */
    void drawMovedRectangle();
    /*
     * Draw the image at the given index from the imagelist.
     *
     * param @index Index of the image in the imagelist to draw.
     */
    void drawImage(int index);
    /*
     * Draw the file name at the given index from the imagetextlist.
     *
     * param @index Index of the file name in the imagetextlist to draw.
     */
    void drawFileName(int index);
    /*
     * Calculates the positions of files based on the current screen dimensions.
     * Uses file dimensions and spacing parameters to arrange files in a grid layout.
     * Adjusts image sizes for optimal display within each grid cell.
     */
    void clearFileCoordinatesAndDimensions();
    /*
     * Appends the selected file name to the current path.
     * This method is called when a file is selected in the GUI table.
     */
    void appendSelectedFileNameToPath();
    /*
     * Handles the state of the previous button based on user input.
     * Checks if the previous button is pressed and adjusts the directory path accordingly.
     */
    void handlePreviousButtonState();
    /*
     * Calculates the positions of files based on the current screen dimensions.
     * Uses file dimensions and spacing parameters to arrange files in a grid layout.
     * Adjusts image sizes for optimal display within each grid cell.
     */
    void calculateFilePositions();


	bool isfilepressed;
	static const int FILE_X = 0, FILE_Y = 1, FILE_W = 2, FILE_H = 3;
	std::vector<int> iconlist;
	std::deque<std::string> imagetextlist;
	int posX, posY, col, row;
	std::string fileName;
	std::string substring;
	int start, charsToPrint;
	int maxcharinline;
	int textX, textY;
	int x, y, filew, fileh;
	int screenwidth, screenheight;
	int maxcolumncount;
	std::vector<int> imagew;
	std::vector<int> imageh;
	std::vector<int> filex;
	std::vector<int> filey;
	int cornerspace;
	int sonucw, sonuch;
	gColor fcolor, backgroundcolor;
	int minh, minw;
	bool isselected, ismoved;
	int movedfileno, selectedfileno;
	bool mousepressedonlist;
	int work;
	std::string path;
	gImage isfolder;
	gImage isfile;
	bool* ispreviouspressed;
	bool pressedstate;
};

#endif /* UI_GGUITABLE_H_ */
