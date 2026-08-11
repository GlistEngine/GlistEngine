#ifndef GAPP_H_
#define GAPP_H_

#include "gBaseApp.h"

class gApp : public gBaseApp {
public:
	gApp(int argc, char** argv);
	void setup() override;
};

#endif
