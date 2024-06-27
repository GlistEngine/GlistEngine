/*
 * gGUIDate.cpp
 *
 *  Created on: 19 Tem 2023
 *      Author: halit
 */

#include "gGUIDate.h"


gGUIDate::gGUIDate() {
    // TODO Auto-generated constructor stub
    w = 270;
    h = 180;
    dx = 0;
    dy = 0;
    leftmargin = 5;
    topmargin = 25;
    std::vector<std::string> daysnames;
	daysnames.push_back("Mon");
	daysnames.push_back("Tue");
	daysnames.push_back("Wed");
	daysnames.push_back("Thu");
	daysnames.push_back("Fri");
	daysnames.push_back("Sat");
	daysnames.push_back("Sun");
	setDaysName(daysnames); //cropLimit parameter have been added
    for (int i = 0; i < 31; i++) {
        daysnumderswidths[i] = font->getStringWidth(std::to_string(i));
    }
    textheight = font->getStringHeight("W");
}

void gGUIDate::setup() {
	gLogi("Date SetUp");
}

gGUIDate::~gGUIDate() {
    // TODO Auto-generated destructor stub
}

void gGUIDate::draw() {
    drawCalendar(11, 2022);
}

void gGUIDate::drawCalendar(int month, int year) {
    int startDay = getStartDay(month, year);
    int daysInMonth = getDaysInMonth(month, year);

    x = left + dx;
    y = top + dy;

    renderer->setColor(middlegroundcolor);
    gDrawRectangle(x, y, w, h, true);

    renderer->setColor(fontcolor);
    int tx = leftmargin, ty = topmargin;
    int cellWidth = w / 7;
    for(int i = 0; i < 7; i++) {
        font->drawText(days[i], x + tx + cellWidth / 2 - daysnameswidths[i] / 2, y + ty);
        tx += cellWidth;
    }

    // Calculate the number of rows needed in the calendar
    int numRows = (startDay + daysInMonth + 6) / 7;

    // Draw the days in the calendar
    tx = leftmargin;
    int cellHeight = (h - font->getStringHeight("W") - 5 - topmargin) / numRows;
    ty += font->getStringHeight("W") + 5; // Use "W" as a representative character for height
    int dayCounter = 1;
    for (int row = 0; row < numRows; row++) {
        for (int col = 0; col < 7; col++) {
            if (row > 0 || col >= startDay) {
                if (dayCounter <= daysInMonth) {
                    std::stringstream dayStr;
                    dayStr << dayCounter; // Removed zero padding
                    font->drawText(dayStr.str(), x + tx + cellWidth / 2 - font->getStringWidth(dayStr.str()) / 2, y + ty + cellHeight / 2 - font->getStringHeight(dayStr.str()) / 2);
                    dayCounter++;
                }
            }
            tx += cellWidth;
        }
        tx = leftmargin;
        ty += cellHeight;
    }
}

int gGUIDate::getStartDay(int month, int year) {
    // Zeller's Congruence algorithm to calculate the day of the week
    // 0 -> Saturday, 1 -> Sunday, ..., 6 -> Friday
    if (month < 3) {
        month += 12;
        year--;
    }
    int h = (1 + (13 * (month + 1)) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
    return (h + 5) % 7;
}

int gGUIDate::getDaysInMonth(int month, int year) {

    if (month == 2) {
        if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
            return 29; // Leap year
        else
            return 28; // Non-leap year
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else {
        return 31;
    }
}

void gGUIDate::setSize(int w, int h) {
    this->w = w;
    this->h = h;
}

void gGUIDate::setDaysName(std::vector<std::string> days, int cropLimit, bool cropmode) {
    for (int i = 0; i < 7; i++) {
    	//STRING SINIRLAMA - if cropmode is false, it works the old way
    	if(!cropmode) {
    		this->days[i] = days[i];
    	} else {
        	if(days[i].size() > cropLimit) {
        		this->days[i] = days[i].substr(0 , cropLimit);
        	} else {
        		this->days[i] = days[i];
        	}
    	}
    }
    for (int i = 0; i < 7; i++) {

		this->daysnameswidths[i] = font->getStringWidth(this->days[i]);
	}
}

void gGUIDate::setPosition(int x, int y) {
	dx = x;
	dy = y;
}
