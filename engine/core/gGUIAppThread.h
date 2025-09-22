/*
 * gGUIAppThread.h
 *
 *  Created on: 9 Aug 2024
 *      Author: noyan
 */

#ifndef CORE_GGUIAPPTHREAD_H_
#define CORE_GGUIAPPTHREAD_H_

#include "gThread.h"
#include "gBaseApp.h"


class gGUIAppThread : public gThread {
public:
	gGUIAppThread(gBaseApp* root);
	virtual ~gGUIAppThread();

	void threadFunction();

	void setTargetFramerate(int framerate);
	int getTargetFramerate();

private:
	gBaseApp* root;
	int targetframerate;
	std::chrono::duration<double, std::milli> sleepdurationmillis;
};

#endif /* CORE_GGUIAPPTHREAD_H_ */
