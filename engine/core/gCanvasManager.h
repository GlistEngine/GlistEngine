/*
 * ofDisplayManager.h
 *
 *  Created on: May 1, 2014
 *      Author: noyan
 */

#ifndef OFDISPLAYMANAGER_H_
#define OFDISPLAYMANAGER_H_

#include "gBaseCanvas.h"

class gCanvasManager : public gObject {

public:
	gCanvasManager();
	virtual ~gCanvasManager();

	void update();
	void setCurrentCanvas(gBaseCanvas *ocnv);
	gBaseCanvas* getCurrentCanvas();
	gBaseCanvas* getTempCanvas();


private:
	gBaseCanvas *currentCanvas, *tempCanvas;
	short displaychangement = 0;
	static const short DISPLAY_CHANGE_NONE = 0, DISPLAY_CHANGE_CURRENT = 1;


};

#endif /* OFDISPLAYMANAGER_H_ */
