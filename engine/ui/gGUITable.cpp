/*
 * gGUITable.cpp
 *
 *  Created on: 28 Tem 2023
 *      Author: Nazim Gultekin
 */

#include "gGUITable.h"
#include "gApp.h"

gGUITable::gGUITable() {
	// TODO Auto-generated constructor stub
	screenwidth = getScreenWidth();
	screenheight = getScreenHeight();
	x = 16;
	y = 20;
	filew = 90;
	fileh = 100;
	maxcolumncount = screenwidth / (filew * 2);
	sonucw = 0;
	sonuch = 0;
	cornerspace = 30;
	minh = 4;
	minw = 1;
	isfilepressed = false;
	isselected = false;
	ismoved = false;
	movedfileno = getScreenHeight();
	selectedfileno = -1;
	mousepressedonlist = false;
	work = 0;
	maxcharinline = 8;
	ispreviouspressed = nullptr;
	pressedstate = false;
}

gGUITable::~gGUITable() {

}

void gGUITable::set(gBaseApp *root, gBaseGUIObject *topParentGUIObject,
		gBaseGUIObject *parentGUIObject, int parentSlotLineNo,
		int parentSlotColumnNo, int x, int y, int w, int h) {
	totalw = width;
	totalh = height;
	gGUIScrollable::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	setDimensions(totalw, totalh);
}

void gGUITable::update() {
	// If a file is selected, update path and reset selected file number
	if (selectedfileno != -1) {
	    clearFileCoordinatesAndDimensions();
	    appendSelectedFileNameToPath();
	}
	handlePreviousButtonState();
	calculateFilePositions();
}

void gGUITable::draw() {
    // Restore the old font color
    gColor oldcolor = renderer->getColor();
    drawBackground();
    drawSelectedRectangle();
    drawMovedRectangle();
    drawIcons();
    renderer->setColor(oldcolor);
}

void gGUITable::drawIcons() {
    for (int index = 0; index < iconlist.size(); ++index) {
        drawImage(index);
        drawFileName(index);
    }
}

void gGUITable::addIcon(int iconId, const std::string& title) {
	iconlist.push_back(iconId);
    imagetextlist.push_back(title);
}

void gGUITable::setFontColor(gColor color) {
	fcolor = color;
}

gColor* gGUITable::getFontColor() {
	return &fcolor;
}

void gGUITable::setTableColor(gColor color) {
	backgroundcolor = color;
}

int gGUITable::setSelectedFileNo(int selectedFileNo) {
	selectedfileno = selectedFileNo;
	return selectedfileno;
}

int gGUITable::getSelectedFileNo() {
	return selectedfileno;
}

gColor* gGUITable::getTableColor() {
	return &backgroundcolor;
}

void gGUITable::mousePressed(int x, int y, int button) {
	for (int index = 0; index < iconlist.size(); index++) {
		if (x >= filex[index] && x <= filex[index] + filew && y >= filey[index] + cornerspace && y <= filey[index] + fileh + cornerspace) {
			isselected = true;
			selectedfileno = index;
			break;
		}
		else {
			isselected = false;
		}
	}
}

void gGUITable::mouseMoved(int x, int y) {
		for (int index = 0; index < iconlist.size(); index++) {
			if (x >= filex[index] && x <= filex[index] + filew && y >= filey[index] && y <= filey[index] + fileh) {
				ismoved = true;
				movedfileno = index;
				break;
			}
			else ismoved = false;
		}
}

void gGUITable::setPath(std::string path) {
	this->path = path;
	setDirectoryToTable();
}

void gGUITable::setDirectoryToTable() {
    // clear the image list and image text list
	iconlist.clear();
    imagetextlist.clear();
    try {
        // check if the path exists and is a directory
        if (fd::exists(path) && fd::is_directory(path)) {
            // iterate through each entry in the directory
            for (const auto& entry : fd::directory_iterator(path)) {
            	if (fd::is_directory(entry.status())) {
            		addIcon(gGUIResources::ICON_FOLDER, entry.path().filename().string());
            	}
            	else
            		addIcon(gGUIResources::ICON_FILE, entry.path().filename().string());
            }
        }
    } catch (const fd::filesystem_error& e) {
        // handle filesystem errors
        std::cerr << "FileSystem Error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // handle general errors
        std::cerr << "General Error: " << e.what() << std::endl;
    }
}

void gGUITable::keyPressed(int key) {
}

void gGUITable::setPreviousButton(bool *isPreviousPressed) {
	ispreviouspressed = isPreviousPressed;
}

void gGUITable::drawBackground() {
	gColor oldcolor = renderer->getColor();
	renderer->setColor(backgroundcolor);
    gDrawRectangle(0, 0, screenwidth, screenheight, true);
    renderer->setColor(oldcolor);
}

void gGUITable::drawSelectedRectangle() {
    if (isselected) {
        gColor oldcolor = renderer->getColor();
        gColor selected = gColor(0.8f, 0.7f, 0.5f);
        renderer->setColor(selected);
        gDrawRectangle(filex[selectedfileno], filey[selectedfileno] + cornerspace, filew, fileh, true);
        renderer->setColor(oldcolor);
    }
}

void gGUITable::drawMovedRectangle() {
    if (ismoved) {
        gColor oldcolor = renderer->getColor();
        gColor moved = gColor(0.8f, 0.7f, 0.5f);
        renderer->setColor(moved);
        gDrawRectangle(filex[movedfileno], filey[movedfileno] + cornerspace, filew, fileh - 30, false);
        renderer->setColor(oldcolor);
    }
}

void gGUITable::drawImage(int index) {
		if(iconlist[index] != gGUIResources::ICON_NONE) {
			gTexture* icon;
			icon = res.getIconImage(iconlist[index]);
			icon->drawSub(filex[index] + (filew - imagew[index]) / 2,
                    filey[index] + cornerspace,
                    imagew[index],
                    imageh[index],
                    0, 0, icon->getWidth(), icon->getHeight());
		}

}

void gGUITable::drawFileName(int index) {
    std::string fileName = imagetextlist[index];
    if (fileName.size() > maxcharinline) {
        int start = 0;
        int charsToPrint = maxcharinline;
        while (start < fileName.size()) {
            std::string substring = fileName.substr(start, charsToPrint);
            int textX = filex[index] + (filew / 2) - (font->getStringWidth(substring) / 2);
            int textY = filey[index] + fileh - 20 + (start / maxcharinline) * (font->getLineHeight() + 2) + cornerspace;
            font->drawText(substring, textX, textY);
            start += charsToPrint;
            charsToPrint = std::min(maxcharinline, (int)fileName.size() - start);
        }
    } else {
        int textX = filex[index] + (filew / 2) - (font->getStringWidth(fileName) / 2);
        int textY = filey[index] + fileh - 20 + cornerspace;
        font->drawText(fileName, textX, textY);
    }
}

void gGUITable::clearFileCoordinatesAndDimensions() {
    filex.clear();
    filey.clear();
    imagew.clear();
    imageh.clear();
}

void gGUITable::appendSelectedFileNameToPath() {
    path = path + "\\" + imagetextlist[selectedfileno];
    setPath(path);
    selectedfileno = -1;
}

void gGUITable::handlePreviousButtonState() {
    if (ispreviouspressed != nullptr) {
        if (*ispreviouspressed == true && pressedstate == false) {
            clearFileCoordinatesAndDimensions();

            size_t lastSlash = path.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                path = path.substr(0, lastSlash);
            }
            lastSlash = path.find_last_of("/\\");
            if (lastSlash == std::string::npos) {
                path += "\\";
            }
            setPath(path);
            pressedstate = true;
        }
        if (*ispreviouspressed == false) {
            pressedstate = false;
        }
    }
}

void gGUITable::calculateFilePositions() {
    maxcolumncount = screenwidth / (filew + cornerspace);

    for (int i = 0; i < iconlist.size(); i++) {
    	// A pointer of type gTexture for the icon image is defined.
    	gTexture* iconimage;
    	// The image of the specified icon is retrieved from the resource manager (res).
    	iconimage = res.getIconImage(iconlist[i]);

        col = i % maxcolumncount;
        row = i / maxcolumncount;

        posX = x + col * (filew + cornerspace);
        posY = y + row * (fileh + cornerspace);

        filex.push_back(posX);
        filey.push_back(posY);

        imagew.push_back(filew - 40);
        imageh.push_back(fileh - 40);

        if (filew < fileh) {
            if (imagew[i] >= imageh[i]) {
                // Horizontal image
                sonucw = imagew[i] / filew;
                imagew[i] /= sonucw + minw;
                imageh[i] /= sonucw + minw;
            } else if (imageh[i] > imagew[i]) {
                // Vertical image
                sonuch = imageh[i] / fileh;
                if (!(imageh[i] <= fileh)) {
                    imagew[i] /= sonuch + minh;
                    imageh[i] /= sonuch + minh;
                }
            }
        }
    }
}
