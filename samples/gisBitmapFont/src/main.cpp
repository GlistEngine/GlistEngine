#include "gApp.h"
#include "gAppManager.h"

int main(int argc, char** argv) {
	gStartEngine(new gApp(argc, argv), "Bitmap Font Sample", G_WINDOWMODE_APP, 960, 540);
	return 0;
}
