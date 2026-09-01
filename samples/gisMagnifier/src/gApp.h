/*
 * gApp.h
 *
 * Created on: Aug 26, 2026
 * Authors: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#ifndef GAPP_H_
#define GAPP_H_

#include "gBaseApp.h"


class gApp : public gBaseApp {
public:
	gApp();
	gApp(int argc, char** argv);
	~gApp();

	void setup();
	void update();
};

#endif /* GAPP_H_ */
