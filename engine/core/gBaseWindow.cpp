/*
 * gBaseWindow.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gBaseWindow.h"
#include "gAppManager.h"
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
#include "backward.hpp"
#endif
#include "gWindowEvents.h"

#include <sstream>
#include <ostream>
#include <iostream>

std::string gBaseWindow::signalname[32];


gBaseWindow::gBaseWindow() {
	signalname[0] = "SIGHUP";
	signalname[1] = "SIGINT";
	signalname[2] = "SIGQUIT";
	signalname[3] = "SIGILL";
	signalname[4] = "SIGTRAP";
	signalname[5] = "SIGABRT";
	signalname[6] = "";
	signalname[7] = "SIGFPE";
	signalname[8] = "SIGFPE";
	signalname[9] = "SIGKILL";
	signalname[10] = "SIGBUS";
	signalname[11] = "SIGSEGV";
	signalname[12] = "SIGSYS";
	signalname[13] = "SIGPIPE";
	signalname[14] = "SIGALRM";
	signalname[15] = "SIGTERM";
	signalname[16] = "SIGURG";
	signalname[17] = "SIGSTOP";
	signalname[18] = "SIGTSTP";
	signalname[19] = "SIGCONT";
	signalname[20] = "SIGCHLD";
	signalname[21] = "SIGTTIN";
	signalname[22] = "SIGTTOU";
	signalname[23] = "SIGPOLL";
	signalname[24] = "SIGXCPU";
	signalname[25] = "SIGXFSZ";
	signalname[26] = "SIGVTALRM";
	signalname[27] = "SIGPROF";
	signalname[28] = "";
	signalname[29] = "";
	signalname[30] = "SIGUSR1";
	signalname[31] = "SIGUSR2";

	vsync = false;
	width = 0;
	height = 0;
	windowmode = G_WINDOWMODE_NONE;
	title = "GlistApp";
	isfocused = false;

	signal(SIGSEGV, sighandler);
}

gBaseWindow::~gBaseWindow() {
}

void gBaseWindow::initialize(int width, int height, int windowMode, bool isResizable) {
	setSize(width, height);
	windowmode = windowMode;
}

bool gBaseWindow::getShouldClose() {
	return false;
}

bool gBaseWindow::isRendering() {
	return true;
}

void gBaseWindow::update() {

}

void gBaseWindow::close() {

}

bool gBaseWindow::isVsyncEnabled() {
	return vsync;
}

void gBaseWindow::setVsync(bool vsync) {
	this->vsync = vsync;
}

void gBaseWindow::enableVsync() {
	setVsync(true);
}

void gBaseWindow::enableVsync(bool vsync) {
	setVsync(vsync);
}

void gBaseWindow::disableVsync() {
	setVsync(false);
}

void gBaseWindow::setCursor(int cursorNo) {

}

void gBaseWindow::setCursorMode(int cursorMode) {

}

void gBaseWindow::setCursorPos(int x, int y) {

}

void gBaseWindow::setIcon(std::string pngFullpath) {

}

void gBaseWindow::setIcon(unsigned char* imageData, int w, int h) {

}

void gBaseWindow::setClipboardString(std::string text) {

}

std::string gBaseWindow::getClipboardString() {
	return "";
}

void gBaseWindow::setSize(int width, int height) {
	this->width = width;
	this->height = height;
	gWindowResizeEvent event{width, height};
	callEvent(event);
}

void gBaseWindow::setWindowSize(int width, int height) {

}

int gBaseWindow::getWidth() {
	return width;
}

int gBaseWindow::getHeight() {
	return height;
}

void gBaseWindow::setTitle(const std::string& windowTitle) {
	title = windowTitle;
}

void gBaseWindow::setTitle(std::string&& windowTitle) {
	title = std::move(windowTitle);
}

const std::string& gBaseWindow::getTitle() {
	return title;
}

void gBaseWindow::setWindowResizable(bool isResizable) {

}

void gBaseWindow::setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) {

}

void gBaseWindow::setEventHandler(EventHandlerFn handler) {
	eventhandler = handler;
}

bool gBaseWindow::isJoystickPresent(int joystickId) {
	return false;
}

void gBaseWindow::callEvent(gEvent& event) {
	if(eventhandler) eventhandler(event);
}

void gBaseWindow::sighandler(int signum) {
//	std::cerr << "Process " << getpid() << " got signal " << signum << " " << signalname[signum] << std::endl;
//	signal(signum, SIG_DFL);
//	kill(getpid(), signum);
/*
//	std::cout << "***** SECTION 2 *****" << std::endl;
	  void* array[20];
	  size_t size;

	  // get void*'s for all entries on the stack
	  size = backtrace(array, 20);

	  // print out all the frames to stderr
	  backtrace_symbols_fd(array, size, STDERR_FILENO);
	  exit(1);
*/
#if defined(WIN32) || defined(LINUX)
//		std::cout << "***** ANOTHER METHOD *****" << std::endl;
	  backward::StackTrace st;


	  st.load_here(99); //Limit the number of trace depth to 99
	  //st.skip_n_firsts(2);//This will skip some backward internal function from the trace

	  backward::Printer p;
	  p.snippet = true;
	  p.object = true;
	  p.color_mode = backward::ColorMode::automatic;
	  p.address = true;
	  p.print(st, stderr);
	  exit(1);
#endif
}

bool gBaseWindow::isGamepadButtonPressed(int joystickId, int buttonId) {
    return false;
}

const float* gBaseWindow::getJoystickAxes(int joystickId, int* axisCountPtr) {
	return nullptr;
}

/*
void stacker() {

using namespace backward;
StackTrace st;


st.load_here(99); //Limit the number of trace depth to 99
st.skip_n_firsts(3);//This will skip some backward internal function from the trace

Printer p;
p.snippet = true;
p.object = true;
p.color_mode = ColorMode::automatic;
p.address = true;
p.print(st, stderr);
}
*/

