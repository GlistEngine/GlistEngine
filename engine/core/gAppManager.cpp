/*
* gAppManager.cpp
*
*  Created on: May 6, 2020
*      Author: noyan
*/

#include <thread>
#include "gAppManager.h"
#include "gBaseApp.h"
#include "gBaseCanvas.h"
#include "gCanvasManager.h"
#include "gBaseComponent.h"
#include "gBasePlugin.h"
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
#include "gGLFWWindow.h"
#include <GLFW/glfw3.h>
#elif defined(ANDROID)
#include "gAndroidWindow.h"
#endif

/*
void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int width, int height, bool isResizable) {
   int wmode = windowMode;
   if(wmode == G_WINDOWMODE_NONE) wmode = G_WINDOWMODE_APP;

   gAppManager appmanager;
#ifdef ANDROID
   gAndroidWindow window;
#else
   gGLFWWindow window;
#endif
   window.setEventHandler(appmanager.getEventHandler());
   std::string appname = appName;
   if (appname == "") appname = "GlistApp";
   window.setTitle(appname);
   appmanager.setWindow(&window);
   baseApp->setAppManager(&appmanager);
   int screenwidth = width, screenheight = height;
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
   if (wmode == G_WINDOWMODE_GAME || wmode == G_WINDOWMODE_FULLSCREEN || wmode == G_WINDOWMODE_FULLSCREENGUIAPP) {
	   glfwInit();
	   const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	   screenwidth = mode->width;
	   screenheight = mode->height;
	   glfwTerminate();
   }
#endif
   int screenscaling = G_SCREENSCALING_AUTO;
   if(wmode == G_WINDOWMODE_FULLSCREENGUIAPP || wmode == G_WINDOWMODE_GUIAPP) screenscaling = G_SCREENSCALING_NONE;
   appmanager.runApp(appName, baseApp, screenwidth, screenheight, wmode, width, height, screenscaling, isResizable, G_LOOPMODE_NORMAL);
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int unitWidth, int unitHeight, int screenScaling, int width, int height, bool isResizable) {
   int wmode = windowMode;
   if(wmode == G_WINDOWMODE_NONE) wmode = G_WINDOWMODE_APP;

   gAppManager appmanager;
#ifdef ANDROID
   gAndroidWindow window;
#else
   gGLFWWindow window;
#endif
   window.setEventHandler(appmanager.getEventHandler());
   std::string appname = appName;
   if (appname == "") appname = "GlistApp";
   window.setTitle(appname);
   appmanager.setWindow(&window);
   baseApp->setAppManager(&appmanager);
   int screenwidth = width, screenheight = height;
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
   if (wmode == G_WINDOWMODE_GAME || wmode == G_WINDOWMODE_FULLSCREEN || wmode == G_WINDOWMODE_FULLSCREENGUIAPP) {
	   glfwInit();
	   const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	   screenwidth = mode->width;
	   screenheight = mode->height;
	   glfwTerminate();
   }
#endif
   appmanager.runApp(appName, baseApp, screenwidth, screenheight, wmode, unitWidth, unitHeight, screenScaling, isResizable, G_LOOPMODE_NORMAL);
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int loopMode) {
   gAppManager appmanager;
   std::string appname = appName;
   if (appname == "") appname = "GlistApp";
   baseApp->setAppManager(&appmanager);
   appmanager.runApp(appName, baseApp, 0, 0, G_WINDOWMODE_NONE, 0, 0, 0, false, loopMode);
}
*/

int pow2(int x, int p) {
   int i = 1;
   for (int j = 1; j <= p; j++) i *= x;
   return i;
}

gAppManager::gAppManager() {
   appname = "";
   window = nullptr;
   app = nullptr;
   windowmode = G_WINDOWMODE_GAME;
   usewindow = true;
   loopmode = G_LOOPMODE_NORMAL;
   loopalways = true;
   canvasmanager = nullptr;
   guimanager = nullptr;
   ismouseentered = false;
   for(int i = 0; i < maxmousebuttonnum; i++) {
	   mousebuttonpressed[i] = false;
   }
   pressed = 0;
   framerate = 60;
   timestepnano = AppClockDuration(1'000'000'000 / (framerate + 1));
   starttime = AppClock::now();
   endtime = starttime;
   deltatime = AppClockDuration(0);
   lag = AppClockDuration(0);
   elapsedtime = 0;
   updates = 0;
   draws = 0;
   uci = 0;
   ucj = 0;
   upi = 0;
   upj = 0;
   canvasset = false;
   iswindowfocused = false;
   isjoystickenabled = false;
   gpbuttonstate = false;
   for(int i = 0; i < maxjoysticknum; i++) {
	   for(int j = 0; j < maxjoystickbuttonnum; j++) joystickbuttonstate[i][j] = false;
	   joystickconnected[i] = false;
   }
   joystickaxecount = 0;
   eventhandler = G_BIND_FUNCTION(onEvent);
}

void gAppManager::runApp(const std::string& appName, gBaseApp *baseApp, int width, int height, int windowMode, int unitWidth, int unitHeight, int screenScaling, bool isResizable, int loopMode) {
   // Create app
   appname = appName;
   app = baseApp;
   windowmode = windowMode;
   if(windowmode == G_WINDOWMODE_NONE) usewindow = false;
   if(!usewindow) {
	   framerate = INT_MAX;
	   timestepnano = AppClockDuration(1'000'000'000 / (framerate + 1));
	   starttime = AppClock::now();
	   endtime = starttime;
	   deltatime = AppClockDuration(0);
	   lag = AppClockDuration(0);
   }

   // Create loop mode
   loopmode = loopMode;
   if(loopmode == G_LOOPMODE_NONE) loopalways = false;

   // Create window
   if(usewindow) {
	   window->initialize(width, height, windowMode, isResizable);

	   // Update the width and height in case it was changed by the operating system or the window implementation
	   width = window->getWidth();
	   height = window->getHeight();

	   canvasmanager = new gCanvasManager();
       guimanager = new gGUIManager(app, width, height);

	   for(int i = 0; i < maxjoysticknum; i++) {
		   joystickconnected[i] = window->isJoystickPresent(i);
		   if(joystickconnected[i]) {
			   isjoystickenabled = true;
		   }
	   }
   }

   // Run app
   app->setup();

   // Exit if console app does not loop
   if(!loopalways) return;

   // Add a temporary empty canvas to prevent errors
   if(usewindow) {
	   gBaseCanvas::setScreenSize(width, height);
	   gBaseCanvas::setUnitScreenSize(unitWidth, unitHeight);
	   gBaseCanvas::setScreenScaling(screenScaling);
   }

   // Main loop
   while(!usewindow || !window->getShouldClose()) {
	   // Delta time calculations
	   endtime = AppClock::now();
	   deltatime = endtime - starttime;
	   elapsedtime += deltatime.count();
	   starttime = endtime;

       internalUpdate();

	   if(!usewindow || !window->vsync) {
		   /* Less precision, but lower CPU usage for non vsync */
		   sleeptime = (timestepnano - (AppClock::now() - starttime)).count() / 1e9;
		   if (sleeptime > 0.0f) {
			   preciseSleep(sleeptime);
		   }
		   /* Much more precise method, but eats up CPU */
		   //			lag += deltatime;
		   //			while(lag >= targettimestep) {
		   //				lag -= targettimestep;
		   //				internalUpdate();
		   //			}
	   }
   }

   if(usewindow) window->close();
}

void gAppManager::internalUpdate() {
   if(!usewindow) {
	   app->update();
	   for (uci = 0; uci < gBaseComponent::usedcomponents.size(); uci++) gBaseComponent::usedcomponents[uci]->update();
	   return;
   }

   canvasmanager->update();
#if(ANDROID)
   // todo gamepad support for android
#else
   if(isjoystickenabled && iswindowfocused) {
	   GLFWgamepadstate gpstate;
	   for(uci = 0; uci < maxjoysticknum; uci++) {
		   if(!joystickconnected[uci]) continue;

		   glfwGetGamepadState(uci, &gpstate);
		   for(ucj = 0; ucj < maxjoystickbuttonnum; ucj++) {
			   gpbuttonstate = false;
			   if(gpstate.buttons[ucj]) gpbuttonstate = true;
			   if(gpbuttonstate != joystickbuttonstate[uci][ucj]) {
				   if(gpbuttonstate) canvasmanager->getCurrentCanvas()->gamepadButtonPressed(uci, ucj);
				   else canvasmanager->getCurrentCanvas()->gamepadButtonReleased(uci, ucj);
			   }
			   joystickbuttonstate[uci][ucj] = gpbuttonstate;
		   }
	   }
   }
#endif

   guimanager->update();
   app->update();
   for(uci = 0; uci < gBaseComponent::usedcomponents.size(); uci++) gBaseComponent::usedcomponents[uci]->update();
   for(upi = 0; upi < gBasePlugin::usedplugins.size(); upi++) gBasePlugin::usedplugins[upi]->update();
   gBaseCanvas* canvas = canvasmanager->getCurrentCanvas();
   if(canvas != nullptr) {
	   canvas->update();
	   updates++;
       if(window->isRendering()) {
           canvas->clearBackground();
           for (upj = 0; upj < renderpassnum; upj++) {
               renderpassno = upj;
               canvas->draw();
               draws++;
           }
       }
   }

   if(window->isRendering()) {
       guimanager->draw();
   }
   window->update();

   if (elapsedtime >= 1'000'000'000){
	   elapsedtime = 0;
	   updates = 0;
	   draws = 0;
   }
}

void gAppManager::preciseSleep(double seconds) {
   t_estimate = 5e-3;
   t_mean = 5e-3;
   t_m2 = 0;
   t_count = 1;

   while (seconds > t_estimate) {
	   auto t_start = AppClock::now();
	   std::this_thread::sleep_for(std::chrono::milliseconds(1));
	   auto end = AppClock::now();

	   t_observed = (end - t_start).count() / 1e9;
	   seconds -= t_observed;

	   ++t_count;
	   t_delta = t_observed - t_mean;
	   t_mean += t_delta / t_count;
	   t_m2   += t_delta * (t_observed - t_mean);
	   t_stddev = sqrt(t_m2 / (t_count - 1));
	   t_estimate = t_mean + t_stddev;
   }

   // spin lock
   auto start = AppClock::now();
   while ((AppClock::now() - start).count() / 1e9 < seconds);
}

void gAppManager::onEvent(gEvent& event) {
   if(event.ishandled) return;

   gEventDispatcher dispatcher(event);
   dispatcher.dispatch<gWindowResizeEvent>(G_BIND_FUNCTION(onWindowResizedEvent));
   dispatcher.dispatch<gCharTypedEvent>(G_BIND_FUNCTION(onCharTypedEvent));
   dispatcher.dispatch<gKeyPressedEvent>(G_BIND_FUNCTION(onKeyPressedEvent));
   dispatcher.dispatch<gKeyReleasedEvent>(G_BIND_FUNCTION(onKeyReleasedEvent));
   dispatcher.dispatch<gMouseMovedEvent>(G_BIND_FUNCTION(onMouseMovedEvent));
   dispatcher.dispatch<gMouseButtonPressedEvent>(G_BIND_FUNCTION(onMouseButtonPressedEvent));
   dispatcher.dispatch<gMouseButtonReleasedEvent>(G_BIND_FUNCTION(onMouseButtonReleasedEvent));
   dispatcher.dispatch<gWindowMouseEnterEvent>(G_BIND_FUNCTION(onWindowMouseEnterEvent));
   dispatcher.dispatch<gWindowMouseExitEvent>(G_BIND_FUNCTION(onWindowMouseExitEvent));
   dispatcher.dispatch<gMouseScrolledEvent>(G_BIND_FUNCTION(onMouseScrolledEvent));
   dispatcher.dispatch<gWindowFocusEvent>(G_BIND_FUNCTION(onWindowFocusEvent));
   dispatcher.dispatch<gWindowLoseFocusEvent>(G_BIND_FUNCTION(onWindowLoseFocusEvent));
   dispatcher.dispatch<gJoystickConnectEvent>(G_BIND_FUNCTION(onJoystickConnectEvent));
   dispatcher.dispatch<gJoystickDisconnectEvent>(G_BIND_FUNCTION(onJoystickDisconnectEvent));

   if(canvasmanager && canvasmanager->getCurrentCanvas()) canvasmanager->getCurrentCanvas()->onEvent(event);
   // todo pass event to app and plugins
}

bool gAppManager::onWindowResizedEvent(gWindowResizeEvent& event) {
    if (!canvasmanager || !canvasmanager->getCurrentCanvas()) return true;
    setScreenSize(event.getWidth(), event.getHeight());
	return false;
}

bool gAppManager::onCharTypedEvent(gCharTypedEvent& event) {
   if(guimanager->isframeset) guimanager->charPressed(event.getCharacter());
   for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->charPressed(event.getCharacter());
   canvasmanager->getCurrentCanvas()->charPressed(event.getCharacter());
   return false;
}

bool gAppManager::onKeyPressedEvent(gKeyPressedEvent& event) {
   if (!canvasmanager->getCurrentCanvas()) return true;
   if(guimanager->isframeset) guimanager->keyPressed(event.getKeyCode());
   for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->keyPressed(event.getKeyCode());
   canvasmanager->getCurrentCanvas()->keyPressed(event.getKeyCode());
   return false;
}

bool gAppManager::onKeyReleasedEvent(gKeyReleasedEvent& event) {
   if (!canvasmanager->getCurrentCanvas()) return true;
   if(guimanager->isframeset) guimanager->keyReleased(event.getKeyCode());
   for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->keyReleased(event.getKeyCode());
   canvasmanager->getCurrentCanvas()->keyReleased(event.getKeyCode());
   return false;
}

bool gAppManager::onMouseMovedEvent(gMouseMovedEvent& event) {
   if (!canvasmanager->getCurrentCanvas()) return true;
   int xpos = event.getX();
   int ypos = event.getY();
   if (gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
	   xpos = gRenderer::scaleX(event.getX());
	   ypos = gRenderer::scaleY(event.getY());
   }
   if (pressed) {
	   if(guimanager->isframeset) guimanager->mouseDragged(xpos, ypos, pressed);
	   for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseDragged(xpos, ypos, pressed);
	   canvasmanager->getCurrentCanvas()->mouseDragged(xpos, ypos, pressed);
   } else {
	   if(guimanager->isframeset) guimanager->mouseMoved(xpos, ypos);
	   for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseMoved(xpos, ypos);
	   canvasmanager->getCurrentCanvas()->mouseMoved(xpos, ypos);
   }
   return false;
}

bool gAppManager::onMouseButtonPressedEvent(gMouseButtonPressedEvent& event) {
   if (!canvasmanager->getCurrentCanvas()) return true;
   mousebuttonpressed[event.getMouseButton()] = true;
   pressed |= pow2(2, event.getMouseButton() + 1);
   int xpos = event.getX();
   int ypos = event.getY();
   if (gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
	   xpos = gRenderer::scaleX(event.getX());
	   ypos = gRenderer::scaleY(event.getY());
   }
   if(guimanager->isframeset) guimanager->mousePressed(xpos, ypos, event.getMouseButton());
   for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mousePressed(xpos, ypos, event.getMouseButton());
   canvasmanager->getCurrentCanvas()->mousePressed(xpos, ypos, event.getMouseButton());
   return false;
}

bool gAppManager::onMouseButtonReleasedEvent(gMouseButtonReleasedEvent& event) {
   if (!canvasmanager->getCurrentCanvas()) return true;
   mousebuttonpressed[event.getMouseButton()] = false;
   pressed &= ~pow2(2, event.getMouseButton() + 1);
   int xpos = event.getX();
   int ypos = event.getY();
   if(gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
	   xpos = gRenderer::scaleX(event.getX());
	   ypos = gRenderer::scaleY(event.getY());
   }
   if(guimanager->isframeset) guimanager->mouseReleased(xpos, ypos, event.getMouseButton());
   for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseReleased(xpos, ypos, event.getMouseButton());
   canvasmanager->getCurrentCanvas()->mouseReleased(xpos, ypos, event.getMouseButton());
   return false;
}

bool gAppManager::onWindowMouseEnterEvent(gWindowMouseEnterEvent& event) {
   if(!canvasmanager->getCurrentCanvas()) return true;
   ismouseentered = true;
   for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseEntered();
   canvasmanager->getCurrentCanvas()->mouseEntered();
   return false;
}

bool gAppManager::onWindowMouseExitEvent(gWindowMouseExitEvent& event) {
   if(!canvasmanager->getCurrentCanvas()) return true;
   ismouseentered = false;
   for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseExited();
   canvasmanager->getCurrentCanvas()->mouseExited();
   return false;
}

bool gAppManager::onMouseScrolledEvent(gMouseScrolledEvent& event) {
   if (!canvasmanager->getCurrentCanvas()) return true;
   if(guimanager->isframeset) guimanager->mouseScrolled(event.getOffsetX(), event.getOffsetY());
   for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseScrolled(event.getOffsetX(), event.getOffsetY());
   canvasmanager->getCurrentCanvas()->mouseScrolled(event.getOffsetX(), event.getOffsetY());
   return false;
}

bool gAppManager::onJoystickConnectEvent(gJoystickConnectEvent& event) {
   if(event.getJoystickId() >= maxjoysticknum) return true;

   if(event.isGamepad()) {
	   joystickconnected[event.getJoystickId()] = true;
	   isjoystickenabled = true;
   }
   if (!canvasmanager->getCurrentCanvas()) return true;
   canvasmanager->getCurrentCanvas()->joystickConnected(event.getJoystickId(), event.isGamepad(), true);
   return false;
}

bool gAppManager::onJoystickDisconnectEvent(gJoystickDisconnectEvent& event) {
   if(event.getJoystickId() >= maxjoysticknum) return true;

   bool wasgamepad = joystickconnected[event.getJoystickId()];
   joystickconnected[event.getJoystickId()] = false;
   isjoystickenabled = false;
   // If at least one joystick is connected
   for(int i = 0; i < maxjoysticknum; i++) {
	   if(joystickconnected[event.getJoystickId()]) {
		   isjoystickenabled = true;
		   break;
	   }
   }
   if (!canvasmanager->getCurrentCanvas()) return true;
   canvasmanager->getCurrentCanvas()->joystickConnected(event.getJoystickId(), wasgamepad, false);
   return false;
}

bool gAppManager::onWindowFocusEvent(gWindowFocusEvent& event) {
   iswindowfocused = true;
   return false;
}

bool gAppManager::onWindowLoseFocusEvent(gWindowLoseFocusEvent& event) {
   iswindowfocused = false;
   return false;
}

void gAppManager::setWindow(gBaseWindow *baseWindow) {
   window = baseWindow;
}

int gAppManager::getWindowMode() {
   return windowmode;
}

void gAppManager::setCursor(int cursorId) {
   window->setCursor(cursorId);
}

void gAppManager::setCursorMode(int cursorMode) {
   window->setCursorMode(cursorMode);
}

gCanvasManager* gAppManager::getCanvasManager() {
   return canvasmanager;
}

void gAppManager::setCurrentCanvas(gBaseCanvas *baseCanvas) {
   canvasmanager->setCurrentCanvas(baseCanvas);
   canvasset = true;
}

gBaseCanvas* gAppManager::getCurrentCanvas() {
   return canvasmanager->getCurrentCanvas();
}

void gAppManager::setScreenSize(int width, int height) {
   canvasmanager->getCurrentCanvas()->setScreenSize(width, height);
   if(canvasset) canvasmanager->getCurrentCanvas()->windowResized(width, height);
   if(canvasset && guimanager->isframeset) guimanager->windowResized(width, height);
}

void gAppManager::setFramerate(int targetFramerate) {
   framerate = targetFramerate;
   timestepnano = AppClockDuration(1'000'000'000 / (framerate + 1));
}

std::string gAppManager::getAppName() {
   return appname;
}


gGUIManager* gAppManager::getGUIManager() {
   return guimanager;
}

void gAppManager::setCurrentGUIFrame(gGUIFrame *guiFrame) {
   guimanager->setCurrentFrame(guiFrame);
}

gGUIFrame* gAppManager::getCurrentGUIFrame() {
   return guimanager->getCurrentFrame();
}

void gAppManager::enableVsync() {
    window->setVsync(true);
}

void gAppManager::disableVsync() {
    window->setVsync(false);
}

int gAppManager::getFramerate() {
   return (long long)(1'000'000'000 / deltatime.count());
}

double gAppManager::getElapsedTime() {
   return deltatime.count() / 1'000'000'000.0f;
}

void gAppManager::setClipboardString(std::string text) {
   window->setClipboardString(text);
}

std::string gAppManager::getClipboardString() {
   return window->getClipboardString();
}

int gAppManager::getLoopMode() {
   return loopmode;
}

bool gAppManager::isWindowFocused() {
   return iswindowfocused;
}

EventHandlerFn gAppManager::getEventHandler() {
	return eventhandler;
}

void gAppManager::setWindowSize(int width, int height) {
	window->setWindowSize(width, height);
}

void gAppManager::setWindowResizable(bool isResizable) {
	window->setWindowResizable(isResizable);
}

void gAppManager::setWindowSizeLimits(int minWidth, int minHeight, int maxWidth, int maxHeight) {
	window->setWindowSizeLimits(minWidth, minHeight, maxWidth, maxHeight);
}

bool gAppManager::isJoystickConnected(int joystickId) {
	if(joystickId >= maxjoysticknum) return false;
	return joystickconnected[joystickId];
}

int gAppManager::getJoystickAxesCount(int joystickId) {
	return joystickaxecount;
}

const float* gAppManager::getJoystickAxes(int joystickId) {
	if(joystickId >= maxjoysticknum || !isjoystickenabled || !joystickconnected[joystickId]) {
		return nullptr;
	}
	return window->getJoystickAxes(joystickId, &joystickaxecount);
}

bool gAppManager::isJoystickButtonPressed(int joystickId, int buttonId) {
	if(joystickId >= maxjoysticknum || !isjoystickenabled || !joystickconnected[joystickId]) {
		return false;
	}
	return window->isJoystickButtonPressed(joystickId, buttonId);
}
