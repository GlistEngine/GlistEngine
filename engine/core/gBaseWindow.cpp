/*
 * gBaseWindow.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gBaseWindow.h"
#include <iostream>
#include "gAppManager.h"
#if defined(WIN32) || defined(LINUX)
#include "backward.hpp"
#endif
#include <sstream>
#include <ostream>
#include <iostream>

std::string gBaseWindow::signalname[32];


gBaseWindow::gBaseWindow() {
	appmanager = NULL;
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
	windowmode = gAppManager::WINDOWMODE_NONE;
	title = "GlistApp";

	signal(SIGSEGV, sighandler);
}

gBaseWindow::~gBaseWindow() {
}


void gBaseWindow::setAppManager(gAppManager *appManager) {
	appmanager = appManager;
}


void gBaseWindow::initialize(int width, int height, int windowMode) {
	setSize(width, height);
	windowmode = windowMode;
}

bool gBaseWindow::getShouldClose() {
	return false;
}

void gBaseWindow::update() {

}

void gBaseWindow::close() {

}

bool gBaseWindow::isVsyncEnabled() {
	return vsync;
}

void gBaseWindow::enableVsync(bool vsync) {
	this->vsync = vsync;
}

void gBaseWindow::setCursor(int cursorNo) {

}

void gBaseWindow::setCursorMode(int cursorMode) {

}

void gBaseWindow::setSize(int width, int height) {
	this->width = width;
	this->height = height;
	appmanager->setScreenSize(width, height);
}

int gBaseWindow::getWidth() {
	return width;
}

int gBaseWindow::getHeight() {
	return height;
}

void gBaseWindow::setTitle(std::string windowTitle) {
	title = windowTitle;
}

std::string gBaseWindow::getTitle() {
	return title;
}

void gBaseWindow::onCharEvent(unsigned int key) {
	appmanager->onCharEvent(key);
}

void gBaseWindow::onKeyEvent(int key, int action) {
	appmanager->onKeyEvent(key, action);
}

void gBaseWindow::onMouseMoveEvent(double xpos, double ypos) {
	appmanager->onMouseMoveEvent(xpos, ypos);
}

void gBaseWindow::onMouseButtonEvent(int button, int action, double xpos, double ypos) {
	appmanager->onMouseButtonEvent(button, action, xpos, ypos);
}

void gBaseWindow::onMouseEnterEvent(int entered) {
	appmanager->onMouseEnterEvent(entered);
}

void gBaseWindow::onMouseScrollEvent(double xoffset, double yoffset) {
	appmanager->onMouseScrollEvent(xoffset, yoffset);
}

void gBaseWindow::sighandler(int signum) {
	std::cerr << "Process " << getpid() << " got signal " << signum << " " << signalname[signum] << std::endl;
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

