/*
 * gGUIColorbox.cpp
 *
 *  Created on: 25 Tem 2022
 *      Author: Medine
 */

#include "gGUIColorbox.h"
#include "gBaseCanvas.h"
#include "gBaseApp.h"

gGUIColorbox::gGUIColorbox() {
    setsize = false;
    color.set(0, 0, 0);
    colorboxw = 0;
    colorboxh = 0;
}

gGUIColorbox::~gGUIColorbox() {

}

void gGUIColorbox::draw() {
    if(!setsize){
    colorboxw = width;
    colorboxh = height;
    }
    gColor oldcolor = renderer->getColor();
    renderer->setColor(buttoncolor);
    gDrawRectangle(left, top, colorboxw, colorboxh, true);
    //renderer->setColor(gColor(1, 1, 1));
    //gDrawRectangle(left + 2, top + 2, colorboxw - 4 , colorboxh - 4 , true);
    renderer->setColor(color);
    gDrawRectangle(left + 5, top + 5, colorboxw - 10 , colorboxh - 10 , true);


}

void gGUIColorbox::setColorboxsize(int colorboxwidth, int colorboxheight){
    setsize = true;
    colorboxw = colorboxwidth;
    colorboxh = colorboxheight;


}


void gGUIColorbox::setColorboxColor(float r, float g, float b){
	if(color.r == r && color.g == g && color.b == b) return;
    color.set(r, g, b);
    if(root->getCurrentCanvas() != nullptr) root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_COLORBOX_CHANGED);
}

void gGUIColorbox::setColorboxColor(gColor newColor){
	if(color.r == newColor.r && color.g == newColor.g && color.b == newColor.b) return;
	color = newColor;
    if(root->getCurrentCanvas() != nullptr) root->getCurrentCanvas()->onGuiEvent(id, G_GUIEVENT_COLORBOX_CHANGED);
}

gColor gGUIColorbox::getcolor() {
    return color;
}

int gGUIColorbox::getWidth(){
    return colorboxw;
}

int gGUIColorbox::getHeight() {
    return colorboxh;
}
