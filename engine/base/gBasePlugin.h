/*
 * gBasePlugin.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef ENGINE_BASE_GBASEPLUGIN_H_
#define ENGINE_BASE_GBASEPLUGIN_H_

#include "gRenderObject.h"
#include "gBaseApp.h"
#include <deque>


class gBasePlugin : public gRenderObject {
public:
	gBasePlugin();
	virtual ~gBasePlugin();

	virtual void setup();
	virtual void update();

	virtual void keyPressed(int key);
	virtual void keyReleased(int key);
	virtual void mouseMoved(int x, int y );
	virtual void mouseDragged(int x, int y, int button);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void mouseEntered();
	virtual void mouseExited();

	static std::deque<gBasePlugin*> usedplugins;

private:
};

#endif /* ENGINE_BASE_GBASEPLUGIN_H_ */
