/*
 * gAppManager.cpp
 *
 *  Created on: May 6, 2020
 *      Author: Noyan Culum
 */

#include "gAppManager.h"
#include "gInputManager.h"
#include "gBaseComponent.h"
#include "gBasePlugin.h"
#include "gEventHook.h"
#include "gBaseApp.h"
#include "gCanvasManager.h"
#include "gGUIFrame.h"
#include "gGUIScrollable.h"

#include <cmath>
#include <thread>
#include "gGUIAppThread.h"
#include "gTracy.h"

// Platform specific window implementation
#if defined(WIN32) || defined(LINUX) || TARGET_OS_OSX
#include "gGLFWWindow.h"
#elif defined(ANDROID)
#include "gAndroidWindow.h"
#include "gAndroidCanvas.h"
#include "gAndroidApp.h"
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#   include "gIOSWindow.h"
#   include "gIOSCanvas.h"
#   include "gIOSApp.h"
#   include "gIOSMain.h"
#elif defined(EMSCRIPTEN)
#include "gGLFWWindow.h"
#include "gWebCanvas.h"
#include "gWebApp.h"
#include "emscripten.h"
#endif

#if GLIST_ANDROID || GLIST_IOS
namespace {

// Tuning for the mobile page gestures. All distances are in unit space, so they
// keep the same physical size across screens: the layout scale is what maps unit
// space onto pixels and it is held fixed by design.

// Exponential decay rate of the fling, per second: the speed is multiplied by
// e^(-rate * dt) each frame. A flick therefore covers speed/rate units in all,
// which at this value puts an ordinary flick at about half a screen and a hard
// one at rather more than a screen, over one to one and a half seconds.
constexpr float flingdecayrate = 3.0f;
// Below this release speed the gesture was a drag, not a flick, and the page
// stops where the finger left it.
constexpr float flingminstartspeed = 60.0f;
// The fling ends here rather than crawling towards zero forever.
constexpr float flingstopspeed = 20.0f;
// A frame longer than this is a stall or a return from the background, and
// integrating it whole would teleport the page.
constexpr float flingmaxstep = 0.1f;
// Velocity samples closer together than this cannot be timed reliably, and ones
// further apart are stale by the time the finger is released.
constexpr float velocitymininterval = 0.001f;
constexpr float velocitymaxinterval = 0.2f;
// Weight of the newest sample. Smoothing keeps a single stuttered frame from
// deciding the flick, while still following the finger closely.
constexpr float velocitysmoothing = 0.7f;

// How far past its end the page may be pulled, in units. The pull resists as it
// nears this, so it is a limit approached rather than a distance reached.
constexpr float overscrollmax = 90.0f;
// Exponential rate at which the page springs back once nothing holds it out.
// Much faster than the fling decay: the bounce should read as the page refusing
// to go further, not as a movement of its own.
constexpr float overscrollreturnrate = 12.0f;
// Under this the spring is done and the offset is dropped to zero outright.
constexpr float overscrollstopdistance = 0.5f;
// Seconds of the leftover fling speed that go into the bounce when a throw
// reaches the end. Small, because a hard fling carries a lot of speed and the
// bounce should stay a hint rather than becoming a second scroll.
constexpr float overscrollbouncetime = 0.12f;

// How long the indicator stays fully opaque after the last movement, and how
// long it then takes to fade out.
constexpr float scrollindicatorholdtime = 0.15f;
constexpr float scrollindicatorfadetime = 0.15f;
constexpr int scrollindicatorthickness = 12;
constexpr int scrollindicatormargin = 4;
// Keeps the thumb grabbable-looking on very long pages.
constexpr int scrollindicatorminlength = 24;
constexpr int scrollindicatoralpha = 110;
// While the page is stretched past its end, the thumb's tip on the side being
// pulled darkens. This is how black that tip gets at full stretch; its length
// grows with the stretch, see overscrollTipLength().
constexpr int scrollindicatordarktipalpha = 145;

// How long the darkened tip is for a given stretch: proportional to how far past
// the end the finger is, the whole thumb at full stretch, nothing when not
// stretched. Sign of overscroll is dropped here - which end it applies to is the
// caller's business.
int overscrollTipLength(int overscroll, int thumblength) {
	int stretch = overscroll < 0 ? -overscroll : overscroll;
	if(stretch <= 0) return 0;
	float ratio = (float)stretch / overscrollmax;
	if(ratio > 1.0f) ratio = 1.0f;
	int length = (int)(thumblength * ratio);
	if(length > thumblength) length = thumblength;
	return length;
}

// A release has to be delivered to end a press that the control is holding, but
// delivering it where the finger actually is would land inside the control and
// count as a click on it: gGUIButton fires G_GUIEVENT_BUTTONRELEASED whenever the
// release point falls within its bounds. Sending the release from a point no
// control can occupy takes the press back instead of completing it.
constexpr int cancelledreleaseposition = -100000;

}
#endif

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int width, int height, bool isResizable) {
    gStartEngine(baseApp, appName, windowMode, width, height, G_SCREENSCALING_AUTO_ONCE, width, height, isResizable);
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int unitWidth, int unitHeight, int screenScaling, int width, int height, bool isResizable) {
    if(windowMode == G_WINDOWMODE_NONE) windowMode = G_WINDOWMODE_APP;
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
    ios_main(baseApp, appName.c_str(), windowMode, unitWidth, unitHeight, screenScaling, width, height, isResizable);
#elif defined(ANDROID)
    new gAppManager(appName, baseApp, width, height, windowMode, unitWidth, unitHeight, screenScaling, isResizable, G_LOOPMODE_NORMAL);
#else
    gAppManager manager(appName, baseApp, width, height, windowMode, unitWidth, unitHeight, screenScaling, isResizable, G_LOOPMODE_NORMAL);
	manager.runApp();
#endif
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int loopMode) {
#if !(defined(GLIST_OPENGLES))
	gAppManager manager(appName, baseApp, 0, 0, G_WINDOWMODE_NONE, 0, 0, G_SCREENSCALING_NONE, false, loopMode);
	manager.runApp();
#else
	throw std::runtime_error("windowless applications are not supported yet for this platform!");
#endif
}

int pow(int x, int p) {
    int i = 1;
    for (int j = 1; j <= p; j++) i *= x;
    return i;
}

gAppManager* appmanager = nullptr;

#ifdef EMSCRIPTEN
void emscriptenTick() {
	appmanager->emscriptenLoop();
}
#endif

gAppManager::gAppManager(const std::string& appName, gBaseApp *baseApp, int width, int height,
                         int windowMode, int unitWidth, int unitHeight, int screenScaling,
                         bool isResizable, int loopMode) : appname(appName), app(baseApp), width(width), height(height),
                                                                 windowmode(windowMode), unitwidth(unitWidth), unitheight(unitHeight), screenscaling(screenScaling),
                                                                 isresizable(isResizable), loopmode(loopMode) {
	ansilocale = "";
#if defined(WIN32)
	ansilocale = setlocale(LC_ALL, ".ACP");
#endif
    appmanager = this;
	inputmanager = new gInputManager();
	if(windowMode != G_WINDOWMODE_NONE) {
		canvasmanager = new gCanvasManager();
	} else {
		canvasmanager = nullptr;
	}
	guimanager = nullptr;
    initialized = false;
    initializedbefore = false;
    iscanvasset = false;
    isrunning = false;
    setupcomplete = false;
    guiappthread = nullptr;
    // Mouse
    ismouseentered = false;
    mousebuttonstate = 0;
    for(int i = 0; i < maxmousebuttonnum; i++) {
        mousebuttonpressed[i] = false;
    }
    targetframerate = 60;
	framerate = targetframerate;
    updateTime();
    starttime = AppClock::now();
    endtime = starttime;
    deltatime = AppClockDuration(0);
    totaltime = 0;
    totalupdates = 0;
    totaldraws = 0;
    iswindowfocused = false;
#ifdef ANDROID
    deviceorientation = DEVICEORIENTATION_PORTRAIT;
    olddeviceorientation = DEVICEORIENTATION_PORTRAIT;
    delayedresize = false;
#endif

    // Joystick
    isjoystickenabled = false;
    for(int i = 0; i < maxjoysticknum; i++) {
        for(int j = 0; j < maxjoystickbuttonnum; j++) joystickbuttonstate[i][j] = false;
        joystickconnected[i] = false;
    }
    joystickaxecount = 0;
    eventhandler = G_BIND_FUNCTION(onEvent);

	if (appname.empty()) {
		appname = "GlistApp";
	}
	if(windowMode != G_WINDOWMODE_NONE) {
		usewindow = true;
#if defined(ANDROID)
		window = new gAndroidWindow();
#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
        window = new gIOSWindow();
#else
		window = new gGLFWWindow();
#endif
		window->setEventHandler(eventhandler);
	} else {
		usewindow = false;
		window = nullptr;
		setTargetFramerate(INT_MAX);
	}

	isguiapp = false;
	if(windowMode == G_WINDOWMODE_GUIAPP || windowMode == G_WINDOWMODE_FULLSCREENGUIAPP) isguiapp = true;
	if(isguiapp) guiappthread = new gGUIAppThread(baseApp);

	// The fixed scale plus scrolling model targets phone screens, where a
	// rotation reshapes the screen underneath a layout the user cannot resize.
	// Desktop windows are resized freely and keep the older proportional
	// behaviour, so the model stays off there. Among the mobile window modes the
	// GUI app ones are the ones that lay out a page of controls.
#if GLIST_ANDROID || GLIST_IOS
	scrollablelayout = isguiapp;
#else
	scrollablelayout = false;
#endif
	layoutscale = 0.0f;
	layoutreferencewidth = 0;
	layoutreferenceheight = 0;
	istouchpressed = false;
	istouchscrolling = false;
	istouchrebaseneeded = false;
	istouchownedbycontrol = false;
	touchstartx = 0;
	touchstarty = 0;
	touchstartscrollx = 0;
	touchstartscrolly = 0;
	desiredscrollx = 0;
	desiredscrolly = 0;
#if GLIST_ANDROID || GLIST_IOS
	isflinging = false;
	flingvelocityx = 0.0f;
	flingvelocityy = 0.0f;
	flingpositionx = 0.0f;
	flingpositiony = 0.0f;
	hastouchvelocity = false;
	lasttouchmovetime = AppClock::now();
	lasttouchscrollx = 0;
	lasttouchscrolly = 0;
	iscontentdrivenheight = true;
	lastcontentheight = -1;
	ungrownlayoutheight = 0;
	overscrollx = 0.0f;
	overscrolly = 0.0f;
	scrollindicatortimer = 0.0f;
	scrollindicatorlastscrollx = 0;
	scrollindicatorlastscrolly = 0;
#endif
}

gAppManager::~gAppManager() {
	gBaseGUIObject::cleanupResources();
	// This will ask to stop and wait for it to complete
	// Then it will delete all the resources it holds
	delete guiappthread;
    delete canvasmanager;
    delete guimanager;
    delete window;
    delete inputmanager;
    inputmanager = nullptr;
    gRenderObject::destroyRenderer();
}

gGUIAppThread* gAppManager::getGUIAppThread() {
	return guiappthread;
}

void gAppManager::setup() {
    if(setupcomplete) {
        return;
    }
    app->setup();
    setupcomplete = true;
}

void gAppManager::runApp() {
	initialize();
	setup();
	if(isguiapp) guiappthread->start();
	loop();
	stop();
}

void gAppManager::initialize() {
    if(initialized || loopmode == G_LOOPMODE_NONE) {
		app->start();
        return;
    }
	deltatime = AppClockDuration(0);

#if GLIST_ANDROID || GLIST_IOS
	// A gesture cannot span a trip through the background: the touches that
	// would have ended it went to a window that no longer exists. onAppPauseEvent
	// already clears it, but that runs off a queue which a stopping loop may
	// never drain, so this is where the guarantee actually holds.
	resetTouchGesture();
#endif

	if(usewindow) {
		window->initialize(width, height, windowmode, isresizable);
		window->setTitle(appname);
		// Update size
		width = window->getWidth();
		height = window->getHeight();
		if(unitwidth == 0) {
			unitwidth = this->width;
		}
		if(unitheight == 0) {
			unitheight = this->height;
		}
		// Create renderer
		gRenderObject::createRenderer();
		gBaseGUIObject::initializeResources();
		// Update renderer dimensions
		renderer->setScreenSize(width, height);
		renderer->setUnitScreenSize(unitwidth, unitheight);
		renderer->setScreenScaling(screenscaling);

		// The scale the app was designed with is captured once and every later
		// layout is derived from it, so that no rotation can redefine what the
		// app considers its own scale.
		if(!initializedbefore) {
			layoutreferencewidth = unitwidth;
			layoutreferenceheight = unitheight;
			layoutscale = computeLayoutScale(width, height);
		}
		// On Android this whole function runs again every time the app returns
		// from the background, because stop() clears the initialized flag and
		// onStart calls initialize() afresh. updateScrollableLayout() re-applies
		// the remembered scroll position, so the user keeps their place in the
		// page across that - and equally if the surface size arrives late and
		// this first layout turns out to be the wrong one.
		if(isScrollableLayoutActive()) updateScrollableLayout(width, height);

		// Create managers if not created
		if(!guimanager) {
			guimanager = new gGUIManager(app, renderer->getWidth(), renderer->getHeight());
			guimanager->getCurrentFrame()->getRenderer()->updateLights();
		}
	}
    initialized = true;
    initializedbefore = true;
    app->start();
}

void gAppManager::loop() {
    if(loopmode == G_LOOPMODE_NONE) {
        return;
    }

#ifdef DEBUG
    if(usewindow) {
		assert(window);
//        assert(renderer);
    }
#endif
    //gLogi("gAppManager") << "starting loop";
    isrunning = true;
#ifdef ANDROID
	// Android drives the app through initialize()/setup()/loop() directly instead
	// of runApp(), so the GUI app thread must also be started here. Every other
	// platform reaches this function through runApp(), which has already started
	// it, so the call is confined here rather than changing their startup path.
	if(isguiapp && guiappthread) guiappthread->start();
#endif
	starttime = AppClock::now();

#ifdef ANDROID
	// This ensures the resize function is called before calling setup for the canvas when
	// device orientation is changed.
	while(delayedresize) {
		executeQueue();
	}
#endif

#ifdef EMSCRIPTEN
	emscripten_set_main_loop(emscriptenTick, 0, true);
#endif
#if !(TARGET_OS_IPHONE || TARGET_OS_SIMULATOR || EMSCRIPTEN)
    while (isrunning && (!usewindow || !window->getShouldClose())) {
        // Delta time calculations
        endtime = AppClock::now();
        deltatime = endtime - starttime;
        totaltime += deltatime.count();
        starttime = endtime;
		totalupdates++;

        tick();

        if(totaltime >= 1'000'000'000) {
        	framerate = totalupdates;
            totaltime = 0;
            totalupdates = 0;
            totaldraws = 0;
        }

        if(!usewindow || !window->vsync) {
            double sleepTime = (targettimestep - (AppClock::now() - starttime)).count() / 1'000'000'000.0;
            if(sleepTime > 0) {
                preciseSleep(sleepTime);
            }
        }
    }
    //gLogi("gAppManager") << "stopping loop";
#ifdef ANDROID
	// The thread must be joined before this function returns, because loop() can
	// be entered again on the next onStart and gThread::start() cannot safely
	// reassign a still-joinable std::thread. Only Android re-enters loop(); the
	// other platforms keep joining it in the destructor as they always have, so
	// their shutdown order is left untouched.
	if(isguiapp && guiappthread) {
		guiappthread->stop();
		guiappthread->wait();
	}
#endif
    app->stop();
    if(usewindow) {
        window->close();
    }
    initialized = false;
#endif // !(TARGET_OS_IPHONE || TARGET_OS_SIMULATOR)
}

#ifdef EMSCRIPTEN
void gAppManager::emscriptenLoop() {
	if (!isrunning || (usewindow && window->getShouldClose())) {
		emscripten_cancel_main_loop();

		app->stop();
		gRenderObject::destroyRenderer();
		if(usewindow) {
			window->close();
		}
		initialized = false;
		return;
	}

	// Delta time calculations
	endtime = AppClock::now();
	deltatime = endtime - starttime;
	totaltime += deltatime.count();
	starttime = endtime;

	tick();

	if(totaltime >= 1'000'000'000) {
		totaltime = 0;
		totalupdates = 0;
		totaldraws = 0;
	}

	if(!usewindow || !window->vsync) {
		double sleepTime = (targettimestep - (AppClock::now() - starttime)).count() / 1'000'000'000.0;
		if(sleepTime > 0) {
			preciseSleep(sleepTime);
		}
	}
}
#endif

void gAppManager::stop() {
    isrunning = false;
}

std::string gAppManager::getAppName() {
	return appname;
}

gCanvasManager* gAppManager::getCanvasManager() {
	return canvasmanager;
}

gGUIManager* gAppManager::getGUIManager() {
	return guimanager;
}

int gAppManager::getLoopMode() {
	return loopmode;
}

bool gAppManager::isWindowFocused() {
	return iswindowfocused;
}

bool gAppManager::callEvent(gEvent& event) {
	eventhandler(event);
	return event.ishandled;
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

void gAppManager::enableScrollableLayout(bool enable) {
#if GLIST_ANDROID || GLIST_IOS
	if(scrollablelayout == enable) return;
	scrollablelayout = enable;
	if(!initialized) return;
	if(enable) {
		updateScrollableLayout(renderer->getScreenWidth(), renderer->getScreenHeight());
	} else {
		gRenderer::setUnitViewportWidth(0);
		gRenderer::setUnitViewportHeight(0);
		// There is no page to be anywhere in once the model is off, so the
		// remembered position must not survive to be re-applied if it is
		// switched back on later.
		desiredscrollx = 0;
		desiredscrolly = 0;
		// The model derived the unit size from the screen, so switching it off
		// has to hand the design size back. setScreenSize() alone would not:
		// its proportional branch recalculates the unit size from itself and
		// would simply keep whatever the model last derived.
		if(layoutreferencewidth > 0 && layoutreferenceheight > 0) {
			gRenderer::setUnitScreenSize(layoutreferencewidth, layoutreferenceheight);
		}
		setScreenSize(renderer->getScreenWidth(), renderer->getScreenHeight());
	}
#else
	// Mobile only, so there is nothing to switch here. See the constructor.
	(void)enable;
#endif
}

float gAppManager::computeLayoutScale(int screenWidth, int screenHeight) const {
	if(unitwidth <= 0) return 1.0f;
	// The orientation the app happens to be launched in must not decide its
	// scale, otherwise starting a phone in landscape renders it differently
	// from starting it upright and turning it. The design size is therefore
	// matched against a physical side of the screen rather than a screen axis:
	// a portrait design spans the short side and a landscape one the long side,
	// and neither side changes identity when the device rotates.
	int shortside = screenWidth < screenHeight ? screenWidth : screenHeight;
	int longside = screenWidth < screenHeight ? screenHeight : screenWidth;
	int designside = unitheight > unitwidth ? shortside : longside;
	if(designside < 1) return 1.0f;
	return (float)designside / (float)unitwidth;
}

bool gAppManager::isScrollableLayoutActive() const {
	return scrollablelayout && layoutscale > 0.0f && screenscaling >= G_SCREENSCALING_AUTO;
}

void gAppManager::updateScrollableLayout(int screenWidth, int screenHeight) {
	// Everything follows from holding the scale still: the unit space is simply
	// however much of it the screen can show at that scale. A rotation widens
	// the layout and shortens the visible band instead of restretching both,
	// which is why controls keep their size and proportions across it.
	int unitw = (int)std::lround(screenWidth / layoutscale);
	int viewportheight = (int)std::lround(screenHeight / layoutscale);
	if(unitw < 1) unitw = 1;
	if(viewportheight < 1) viewportheight = 1;

	// The layout keeps the size it was designed for even when the screen is
	// smaller than that; the remainder becomes scrollable rather than squeezed
	// or, on the horizontal axis, cut off with no way to reach it. A portrait
	// design only ever overflows vertically and a landscape one horizontally,
	// so in practice at most one of the two axes actually scrolls.
	int layoutwidth = unitw > layoutreferencewidth ? unitw : layoutreferencewidth;
	int layoutheight = viewportheight > layoutreferenceheight ? viewportheight : layoutreferenceheight;
#if GLIST_ANDROID || GLIST_IOS
	// A third candidate: what the content itself needs. The design size says how
	// tall the page was drawn for, not how much has since been put on it, and a
	// page with more rows than it was designed for used to squash them all rather
	// than grow. Measured against the previous pass, which is enough because the
	// width - and so every wrap and every content height that follows from it -
	// is settled before the height is ever asked about.
	// Kept so that a later measurement outside this function asks the same
	// question this one did. Measuring against the grown height instead would let
	// each answer raise the next one.
	ungrownlayoutheight = layoutheight;
	if(iscontentdrivenheight) {
		int contentheight = measureLayoutContentHeight(layoutheight);
		if(contentheight > layoutheight) layoutheight = contentheight;
	}
#endif

	gRenderer::setUnitScreenSize(layoutwidth, layoutheight);
	gRenderer::setUnitViewportWidth(unitw);
	gRenderer::setUnitViewportHeight(viewportheight);
	gRenderer::setScreenSize(screenWidth, screenHeight);
	// Each setter above clamps the position against the sizes known at that
	// moment, so it is re-applied here, once they all agree on the new layout.
	// The remembered position is the source rather than the renderer's own:
	// reading that back would return whatever the intermediate steps clamped it
	// to, which is how a rotation or a return from the background used to lose
	// the user's place. Deliberately not written back - a layout too short to
	// hold the position must not shorten the memory of it, or rotating away and
	// back would not return to where the user was.
	gRenderer::setScrollX(desiredscrollx);
	gRenderer::setScrollY(desiredscrolly);

#if GLIST_ANDROID || GLIST_IOS
	// The indicator otherwise only appears once the page has already moved,
	// which leaves the user no way to tell there is anything below the fold.
	// Showing it briefly whenever the layout is (re)built - at launch and after
	// every rotation - is what makes the overflow discoverable.
	if(gRenderer::getMaxScrollX() > 0 || gRenderer::getMaxScrollY() > 0) {
		scrollindicatortimer = scrollindicatorholdtime + scrollindicatorfadetime;
	}
	scrollindicatorlastscrollx = gRenderer::getScrollX();
	scrollindicatorlastscrolly = gRenderer::getScrollY();
#endif

	if(iscanvasset && canvasmanager && canvasmanager->getCurrentCanvas()) {
		canvasmanager->getCurrentCanvas()->windowResized(renderer->getWidth(), renderer->getHeight());
	}
	if(guimanager && guimanager->isframeset) {
		guimanager->windowResized(renderer->getWidth(), renderer->getHeight());
	}
}

void gAppManager::setScrollPosition(int scrollX, int scrollY) {
	gRenderer::setScrollX(scrollX);
	gRenderer::setScrollY(scrollY);
	// Stored after the setters, so the memory holds a position that was actually
	// reachable rather than however far past the end the gesture asked for.
	desiredscrollx = gRenderer::getScrollX();
	desiredscrolly = gRenderer::getScrollY();
}

void gAppManager::setScreenSize(int width, int height) {
	G_PROFILE_ZONE_SCOPED_N("gAppManager::setScreenSize()");
	G_PROFILE_ZONE_VALUE(width);
	G_PROFILE_ZONE_VALUE(height);
	if(isScrollableLayoutActive()) {
		updateScrollableLayout(width, height);
		return;
	}
	if(screenscaling == G_SCREENSCALING_AUTO_ONCE) {
		// We don't want to update the unitresolution, that's why its setting directly. gAppManager needs to be a friend of gRenderer to do this.
		renderer->unitwidth = renderer->scaleX(width);
		renderer->unitheight = renderer->scaleY(height);
	}
	renderer->setScreenSize(width, height);
    if(iscanvasset && canvasmanager->getCurrentCanvas()) {
	    canvasmanager->getCurrentCanvas()->windowResized(renderer->getWidth(), renderer->getHeight());
    }
    if(guimanager && guimanager->isframeset) {
	    guimanager->windowResized(renderer->getWidth(), renderer->getHeight());
    }
}

void gAppManager::setCurrentCanvas(gBaseCanvas* canvas) {
    canvasmanager->setCurrentCanvas(canvas);
    iscanvasset = true;
}

gBaseCanvas* gAppManager::getCurrentCanvas() {
    return canvasmanager->getCurrentCanvas();
}

void gAppManager::setTargetFramerate(int framerate) {
    targetframerate = framerate;
	updateTime();
}

int gAppManager::getTargetFramerate() {
    return targetframerate;
}

int gAppManager::getFramerate() {
    return framerate;
}

void gAppManager::enableVsync() {
    window->setVsync(true);
}

void gAppManager::disableVsync() {
    window->setVsync(false);
}

void gAppManager::setCurrentGUIFrame(gGUIFrame *guiFrame) {
    guimanager->setCurrentFrame(guiFrame);
}

double gAppManager::getElapsedTime() {
    return deltatime.count() / 1'000'000'000.0;
}

void gAppManager::setClipboardString(const std::string &clipboard) {
    window->setClipboardString(clipboard);
}

std::string gAppManager::getClipboardString() {
    return window->getClipboardString();
}

void gAppManager::setCursor(int cursorId) {
    window->setCursor(cursorId);
}

void gAppManager::setCursorMode(gCursorMode cursorMode) {
    window->setCursorMode(cursorMode);
}

gCursorMode gAppManager::getCursorMode() {
	return window->getCursorMode();
}

void gAppManager::setWindowIcon(std::string pngFullpath) {
	window->setIcon(pngFullpath);
}

void gAppManager::setWindowIcon(unsigned char* imageData, int w, int h) {
	window->setIcon(imageData, w, h);
}

void gAppManager::setWindowTitle(const std::string& windowTitle) {
	window->setTitle(windowTitle);
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

bool gAppManager::isGamepadButtonPressed(int joystickId, int buttonId) {
    if(joystickId >= maxjoysticknum || !isjoystickenabled || !joystickconnected[joystickId]) {
        return false;
    }
    return window->isGamepadButtonPressed(joystickId, buttonId);
}

std::string gAppManager::getANSILocale() {
	return ansilocale;
}

#ifdef ANDROID
void gAppManager::setDeviceOrientation(DeviceOrientation orientation) {
	if(!isrunning) {
		gAndroidUtil::setDeviceOrientation(orientation);
		delayedresize = true;
	} else {
		gAndroidUtil::setDeviceOrientation(orientation);
	}
}
#endif

void gAppManager::tick() {
	G_PROFILE_FRAME_MARK();
    if(!usewindow) {
		G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): Non Window Update");
        app->update();
    	for (gBaseComponent*& component : gBaseComponent::usedcomponents) {
    		component->update();
    	}
		executeQueue();
        return;
    }

#ifdef ANDROID
	// The Java-side resize notification travels through two queues before it
	// reaches the engine; it can be dropped when the queue is full and it can
	// observe a stale surface size mid-rotation, in which case no corrective
	// notification ever follows and the screen stays stuck in the old
	// orientation. The real surface size is therefore polled every frame and a
	// mismatch is fed into the normal resize path, which makes missed or stale
	// notifications self-healing on the next frame.
	if(window->isRendering()) {
		EGLDisplay currentdisplay = eglGetCurrentDisplay();
		EGLSurface currentsurface = eglGetCurrentSurface(EGL_DRAW);
		EGLint surfacewidth, surfaceheight;
		if(currentdisplay != EGL_NO_DISPLAY && currentsurface != EGL_NO_SURFACE &&
				eglQuerySurface(currentdisplay, currentsurface, EGL_WIDTH, &surfacewidth) &&
				eglQuerySurface(currentdisplay, currentsurface, EGL_HEIGHT, &surfaceheight) &&
				(surfacewidth != renderer->getScreenWidth() || surfaceheight != renderer->getScreenHeight())) {
			renderer->setViewport(0, 0, surfacewidth, surfaceheight);
			window->setSize(surfacewidth, surfaceheight);
		}
	}
#endif

#if GLIST_ANDROID || GLIST_IOS
	// Ahead of the managers, so that the controls are updated against the scroll
	// position this frame will actually be drawn at.
	// Before the fling and the layout work below, so that everything this frame
	// sees the same page height.
	updateLayoutContentHeight();
	if(isScrollableLayoutActive()) {
		updateFling();
		// After the fling, so a throw that reaches the end this frame starts
		// springing back on the next one rather than being cut short here.
		updateOverscroll();
		updateScrollIndicator();
	}
	// Outside the layout check: a control's own content scrolls whether or not
	// the page around it does.
	if(isguiapp) gGUIScrollable::updateContentFling();
#endif

    if(canvasmanager) canvasmanager->update();
    if(guimanager) guimanager->update();
    if(!isguiapp) {
		G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): App Update");
    	app->update();
    }
    {
		G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): Components Update");
    	for (gBaseComponent*& component : gBaseComponent::usedcomponents) {
    		component->update();
    	}
    }
    {
		G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): Plugins Update");
    	for (gBasePlugin*& component : gBasePlugin::usedplugins) {
    		component->update();
    	}
    }

    gBaseCanvas* canvas = nullptr;
    if(!isguiapp) {
		if(canvasmanager) canvas = canvasmanager->getCurrentCanvas();
		if(canvas) {
			G_PROFILE_ZONE_SCOPED_N("gAppManager::tick(): Canvas Update");
			canvas->update();
		}
    }

    if(window->isRendering()) {
    	if(!isguiapp) {
			if(canvas) {
				canvas->clearBackground();
				for (int i = 0; i < renderpassnum; i++) {
					G_PROFILE_ZONE_SCOPED_N("gGUIManager::tick(): Render Pass");
					G_PROFILE_ZONE_VALUE(i);
					renderpassno = i;
					gBaseCanvas::getRenderer()->updateScene();
					canvas->draw();
				}
			}
    	}
		if(guimanager) guimanager->draw();
#if GLIST_ANDROID || GLIST_IOS
		// Last, so that it sits above the page rather than under its controls.
		if(isScrollableLayoutActive()) drawScrollIndicator();
#endif
        totaldraws++;
    }
	if(inputmanager) inputmanager->update();
	if(usewindow) window->update();
	executeQueue();
}

void gAppManager::onEvent(gEvent& event) {
    if(event.ishandled) return;

    gEventDispatcher dispatcher(event);
    dispatcher.dispatch<gWindowResizeEvent>(G_BIND_FUNCTION(onWindowResizedEvent));
    dispatcher.dispatch<gWindowScaleChangedEvent>(G_BIND_FUNCTION(onWindowScaleChangedEvent));
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
    dispatcher.dispatch<gAppPauseEvent>(G_BIND_FUNCTION(onAppPauseEvent));
    dispatcher.dispatch<gAppResumeEvent>(G_BIND_FUNCTION(onAppResumeEvent));
#if GLIST_ANDROID || GLIST_IOS
    dispatcher.dispatch<gDeviceOrientationChangedEvent>(G_BIND_FUNCTION(onDeviceOrientationChangedEvent));
    dispatcher.dispatch<gTouchEvent>(G_BIND_FUNCTION(onTouchEvent));
#endif
    if(canvasmanager && getCurrentCanvas()) getCurrentCanvas()->onEvent(event);
	for (gBaseComponent*& component : gBaseComponent::usedcomponents) {
		component->onEvent(event);
	}
	for (gBasePlugin*& component : gBasePlugin::usedplugins) {
		component->onEvent(event);
	}
	for (gEventHook*& hook : gEventHook::hooks) {
		hook->onEvent(event);
	}
}

bool gAppManager::onWindowResizedEvent(gWindowResizeEvent& event) {
    if(!canvasmanager || !initialized || (!isguiapp && !getCurrentCanvas() && !canvasmanager->getTempCanvas())) {
        return true;
    }
#ifdef ANDROID
    delayedresize = false;
    bool swapdimensions = false;
    // The scrollable layout derives the whole unit space from the screen and
    // the fixed scale, so it already produces the rotated layout on its own and
    // the swap below would only fight it.
    if(!isScrollableLayoutActive() && gRenderer::getScreenScaling() >= G_SCREENSCALING_AUTO) {
		// Rotation is detected from the surface dimensions carried by the event
		// itself instead of the device orientation events: those are dispatched
		// from the Java UI thread and can reach the engine after this resize
		// event, in which case an orientation-based check misses the rotation,
		// the unit swap never happens and the proportional rescale below
		// corrupts the unit sizes. The surface shape flipping between
		// portrait-like and landscape-like is a race-free rotation signal.
		int oldwidth = renderer->getScreenWidth();
		int oldheight = renderer->getScreenHeight();
		swapdimensions = oldwidth != oldheight && event.getWidth() != event.getHeight() &&
				(oldwidth > oldheight) != (event.getWidth() > event.getHeight());
    }
    olddeviceorientation = deviceorientation;
	if(swapdimensions) {
		// Device rotated: the unit space rotates with it (e.g. 720x1280 becomes
		// 1280x720). The proportional AUTO_ONCE recalculation in setScreenSize()
		// assumes a same-orientation resize and would corrupt the unit values
		// here, so screen and unit sizes are set directly and the canvas/GUI are
		// notified only once both hold their final values.
		gRenderer::setUnitScreenSize(renderer->getUnitHeight(), renderer->getUnitWidth());
		gRenderer::setScreenSize(event.getWidth(), event.getHeight());
		if(iscanvasset && canvasmanager->getCurrentCanvas()) {
			canvasmanager->getCurrentCanvas()->windowResized(renderer->getWidth(), renderer->getHeight());
		}
		if(guimanager && guimanager->isframeset) {
			guimanager->windowResized(renderer->getWidth(), renderer->getHeight());
		}
		return false;
	}
#endif
    setScreenSize(event.getWidth(), event.getHeight());
    return false;
}

bool gAppManager::onWindowScaleChangedEvent(gWindowScaleChangedEvent& event) {
	if(!canvasmanager || !initialized || (!getCurrentCanvas() && !canvasmanager->getTempCanvas())) {
		return true;
	}
	renderer->width = event.getWidth();
	renderer->height = event.getHeight();
	if(screenscaling == G_SCREENSCALING_AUTO_ONCE) {
		renderer->unitwidth = renderer->width / event.getScaleX();
		renderer->unitheight = renderer->height / event.getScaleY();
	}
	gRenderer::updateProjectionMatrix2d();
	return false;
}

bool gAppManager::onCharTypedEvent(gCharTypedEvent& event) {
//	if (!canvasmanager || !getCurrentCanvas()) return true;
#if GLIST_ANDROID || GLIST_IOS
    // In a GUI app the soft keyboard delivers this on the platform UI thread while
    // the loop draws the GUI on its own thread; driving the widget tree from here
    // would race that drawing. The GUI part is queued to the loop, exactly as touch
    // is in handleGUITouch(). Off the GUI-app path the loop runs on this thread, so
    // the old direct call stands.
    if(isguiapp) {
        unsigned int character = event.getCharacter();
        submitToMainThread([this, character]() {
            if(guimanager && guimanager->isframeset) guimanager->charPressed(character);
        });
    } else if(guimanager->isframeset) {
        guimanager->charPressed(event.getCharacter());
    }
#else
    if(guimanager->isframeset) guimanager->charPressed(event.getCharacter());
#endif
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->charPressed(event.getCharacter());
    if(canvasmanager && getCurrentCanvas()) getCurrentCanvas()->charPressed(event.getCharacter());
    return false;
}

bool gAppManager::onKeyPressedEvent(gKeyPressedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
#if GLIST_ANDROID || GLIST_IOS
    if(isguiapp) {
        int keycode = event.getKeyCode();
        submitToMainThread([this, keycode]() {
            if(guimanager && guimanager->isframeset) guimanager->keyPressed(keycode);
        });
    } else if(guimanager->isframeset) {
        guimanager->keyPressed(event.getKeyCode());
    }
#else
    if(guimanager->isframeset) guimanager->keyPressed(event.getKeyCode());
#endif
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->keyPressed(event.getKeyCode());
    if(canvasmanager && getCurrentCanvas()) getCurrentCanvas()->keyPressed(event.getKeyCode());
    return false;
}

bool gAppManager::onKeyReleasedEvent(gKeyReleasedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
#if GLIST_ANDROID || GLIST_IOS
    if(isguiapp) {
        int keycode = event.getKeyCode();
        submitToMainThread([this, keycode]() {
            if(guimanager && guimanager->isframeset) guimanager->keyReleased(keycode);
        });
    } else if(guimanager->isframeset) {
        guimanager->keyReleased(event.getKeyCode());
    }
#else
    if(guimanager->isframeset) guimanager->keyReleased(event.getKeyCode());
#endif
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->keyReleased(event.getKeyCode());
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->keyReleased(event.getKeyCode());
    return false;
}

bool gAppManager::onMouseMovedEvent(gMouseMovedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
    int xpos = event.getX();
    int ypos = event.getY();
    if(renderer->getScreenScaling() >= G_SCREENSCALING_AUTO) {
        xpos = gRenderer::scaleX(event.getX());
        ypos = gRenderer::scaleY(event.getY());
    }
    if(mousebuttonstate) {
        if(guimanager->isframeset) guimanager->mouseDragged(xpos, ypos, mousebuttonstate);
        for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseDragged(xpos, ypos, mousebuttonstate);
        if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseDragged(xpos, ypos, mousebuttonstate);
    } else {
        if(guimanager->isframeset) guimanager->mouseMoved(xpos, ypos);
        for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseMoved(xpos, ypos);
        if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseMoved(xpos, ypos);
    }
    return false;
}

bool gAppManager::onMouseButtonPressedEvent(gMouseButtonPressedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
    mousebuttonpressed[event.getMouseButton()] = true;
    mousebuttonstate |= pow(2, event.getMouseButton() + 1);
    int xpos = event.getX();
    int ypos = event.getY();
    if(gRenderer::getScreenScaling() >= G_SCREENSCALING_AUTO) {
        xpos = gRenderer::scaleX(event.getX());
        ypos = gRenderer::scaleY(event.getY());
    }
    if(guimanager->isframeset) guimanager->mousePressed(xpos, ypos, event.getMouseButton());
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mousePressed(xpos, ypos, event.getMouseButton());
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mousePressed(xpos, ypos, event.getMouseButton());
    return false;
}

bool gAppManager::onMouseButtonReleasedEvent(gMouseButtonReleasedEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
    mousebuttonpressed[event.getMouseButton()] = false;
    mousebuttonstate &= ~pow(2, event.getMouseButton() + 1);
    int xpos = event.getX();
    int ypos = event.getY();
    if(gRenderer::getScreenScaling() > G_SCREENSCALING_AUTO) {
        xpos = gRenderer::scaleX(event.getX());
        ypos = gRenderer::scaleY(event.getY());
    }
    if(guimanager->isframeset) guimanager->mouseReleased(xpos, ypos, event.getMouseButton());
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseReleased(xpos, ypos, event.getMouseButton());
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseReleased(xpos, ypos, event.getMouseButton());
    return false;
}

bool gAppManager::onWindowMouseEnterEvent(gWindowMouseEnterEvent& event) {
//    if(!canvasmanager || !getCurrentCanvas()) return true;
    ismouseentered = true;
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseEntered();
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseEntered();
    return false;
}

bool gAppManager::onWindowMouseExitEvent(gWindowMouseExitEvent& event) {
//    if(!canvasmanager || !getCurrentCanvas()) return true;
    ismouseentered = false;
    for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseExited();
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseExited();
    return false;
}

bool gAppManager::onMouseScrolledEvent(gMouseScrolledEvent& event) {
//    if (!canvasmanager || !getCurrentCanvas()) return true;
#if GLIST_ANDROID || GLIST_IOS
    // Page scrolling with the wheel belongs to the scrollable layout, which is a
    // mobile only feature. On desktop the wheel keeps its original meaning and
    // is delivered to the controls, so this branch is compiled out there.
    if(gRenderer::getMaxScrollX() > 0 || gRenderer::getMaxScrollY() > 0) {
        // The layout does not fit on screen, so the wheel moves the page itself
        // rather than reaching the controls on it.
        setScrollPosition(gRenderer::getScrollX() - event.getOffsetX() * wheelscrollstep,
                gRenderer::getScrollY() - event.getOffsetY() * wheelscrollstep);
    } else if(guimanager && guimanager->isframeset) {
        guimanager->mouseScrolled(event.getOffsetX(), event.getOffsetY());
    }
#else
    if(guimanager && guimanager->isframeset) {
        guimanager->mouseScrolled(event.getOffsetX(), event.getOffsetY());
    }
#endif
    for (gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseScrolled(event.getOffsetX(), event.getOffsetY());
    if(canvasmanager && getCurrentCanvas()) canvasmanager->getCurrentCanvas()->mouseScrolled(event.getOffsetX(), event.getOffsetY());
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

bool gAppManager::onJoystickConnectEvent(gJoystickConnectEvent& event) {
    if(event.getJoystickId() >= maxjoysticknum) return true;

    joystickconnected[event.getJoystickId()] = true;
    isjoystickenabled = true;
    if (!canvasmanager || !getCurrentCanvas()) return true;
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
    if (!canvasmanager || !getCurrentCanvas()) return true;
    canvasmanager->getCurrentCanvas()->joystickConnected(event.getJoystickId(), wasgamepad, false);
    return false;
}

#if GLIST_IOS
void gAppManager::iosLoop()
{
    if(!(isrunning && (!usewindow || !window->getShouldClose())))
    {
        app->stop();
        if(usewindow) {
            window->close();
        }
        initialized = false;
    }
    
    // Delta time calculations
    endtime = AppClock::now();
    deltatime = endtime - starttime;
    totaltime += deltatime.count();
    starttime = endtime;
	totalupdates++;

    tick();

    if(totaltime >= 1'000'000'000) {
    	framerate = totalupdates;
        totaltime = 0;
        totalupdates = 0;
        totaldraws = 0;
    }

    if(!usewindow || !window->vsync) {
        double sleepTime = (targettimestep - (AppClock::now() - starttime)).count() / 1'000'000'000.0;
        if(sleepTime > 0) {
            preciseSleep(sleepTime);
        }
    }
    //gLogi("gAppManager") << "stopping loop";
}
#endif // GLIST_IOS

bool gAppManager::onAppPauseEvent(gAppPauseEvent& event) {
    submitToMainThread([this]() {
#if GLIST_ANDROID || GLIST_IOS
		// The app can be sent to the background with a finger still on the glass
		// and the platform is not obliged to deliver the matching cancel. The
		// gesture would then stay half open: its anchor belongs to a touch that
		// no longer exists, and a fling would resume on return as if no time had
		// passed. Cancelled rather than merely dropped, so that the part of the
		// swipe that sent the app away does not stay on the page as scroll.
		cancelTouchGesture();
#endif
		if (app) {
			app->pause();
		}
		if(canvasmanager && getCurrentCanvas()) {
			getCurrentCanvas()->pause();
		}
	});
    return false;
}

bool gAppManager::onAppResumeEvent(gAppResumeEvent& event) {
    submitToMainThread([this]() {
		if (app) {
			app->resume();
		}
		if(canvasmanager && getCurrentCanvas()) {
			getCurrentCanvas()->resume();
		}
	});
    return false;
}

#if GLIST_ANDROID || GLIST_IOS
bool gAppManager::onDeviceOrientationChangedEvent(gDeviceOrientationChangedEvent& event) {
	deviceorientation = event.getOrientation();
    if(canvasmanager && getCurrentCanvas()) {
		if (
#if GLIST_ANDROID
            auto* target = dynamic_cast<gAndroidCanvas*>(getCurrentCanvas())
#elif GLIST_IOS
            auto* target = static_cast<gIOSCanvas*>(getCurrentCanvas())
#elif GLIST_WEB
			auto* target = static_cast<gWebCanvas*>(getCurrentCanvas())
#endif
        ) {
			target->deviceOrientationChanged(event.getOrientation());
		}
    }
    return false;
}

void gAppManager::handleGUITouch(gTouchEvent& event) {
	if(event.getInputCount() < 1) return;
	int inputindex = gClamp(event.getActionIndex(), 0, event.getInputCount() - 1);
	// Touch events are delivered on the platform UI thread while the loop is
	// drawing the GUI on its own thread, and driving the controls from here
	// would mutate the widget tree underneath that drawing. The position is
	// copied out of the event, whose input array belongs to the caller's stack
	// and does not outlive this call, and the gesture is processed in the loop.
	int touchx = event.getInputs()[inputindex].x;
	int touchy = event.getInputs()[inputindex].y;
	ActionType action = event.getAction();
	submitToMainThread([this, touchx, touchy, action]() {
		processGUITouch(touchx, touchy, action);
	});
}

void gAppManager::processGUITouch(int touchX, int touchY, ActionType action) {
	int x = touchX;
	int y = touchY;
	// Converted here rather than on arrival so that the position is measured
	// against the screen size the loop currently holds.
	if(gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
		x = gRenderer::scaleX(x);
		y = gRenderer::scaleY(y);
	}
	bool isframeset = guimanager && guimanager->isframeset;

	switch(action) {
	case ACTIONTYPE_DOWN:
		anchorTouchGesture(x, y);
		istouchpressed = true;
		istouchscrolling = false;
		istouchrebaseneeded = false;
		istouchownedbycontrol = false;
		// Any coasting content stops the moment a finger lands, wherever it lands.
		gGUIScrollable::cancelContentFling();
		if(isframeset) {
			// The GUI routes presses and drags only to the control it considers
			// the cursor to be on (gGUISizer::mousePressed and mouseDragged both
			// test iscursoron), and that flag is set nowhere but mouseMoved. A
			// mouse sets it by hovering on the way to the click; a finger arrives
			// with no hover at all, so without this the whole GUI is deaf to
			// touch. The hover is therefore synthesised at the touch point.
			guimanager->mouseMoved(x, y);
			guimanager->mousePressed(x, y, 0);
		}
		break;
	case ACTIONTYPE_POINTER_DOWN:
	case ACTIONTYPE_POINTER_UP:
		// A finger joined or left the gesture, so the index the positions are
		// read from can start referring to a different finger. Measuring that
		// finger against the anchor of the one that is gone would jump the page,
		// so the gesture is re-anchored on the next move instead, which carries
		// it on smoothly from whichever finger remains.
		istouchrebaseneeded = true;
		break;
	case ACTIONTYPE_MOVE: {
		if(!istouchpressed) break;
		if(istouchrebaseneeded) {
			// Deliberately keeps istouchscrolling as it is: re-anchoring moves
			// the origin of the gesture, it does not start a new one.
			anchorTouchGesture(x, y);
			istouchrebaseneeded = false;
			break;
		}
		if(!istouchscrolling) {
			// The controls see the drag before the page does. A scrollable one
			// claims it as soon as it starts moving its own content, and while it
			// holds the claim the page stays still - otherwise a list inside a
			// scrolling page could never be scrolled, because the page would take
			// every gesture that began on it.
			if(isframeset) guimanager->mouseDragged(x, y, 0);
			if(gGUIScrollable::isContentDragActive()) {
				istouchownedbycontrol = true;
				break;
			}
			// The finger position is measured against the visible band, which does
			// not move while scrolling, so the distance stays a plain drag distance
			// and cannot feed back into itself.
			int draggedx = touchstartx - x;
			int draggedy = touchstarty - y;
			if((gRenderer::getMaxScrollX() > 0 &&
							(draggedx >= touchscrollthreshold || draggedx <= -touchscrollthreshold)) ||
					(gRenderer::getMaxScrollY() > 0 &&
							(draggedy >= touchscrollthreshold || draggedy <= -touchscrollthreshold))) {
				if(istouchownedbycontrol) {
					// A control had the gesture and has just run out of content, so
					// the page picks it up from where the finger is now. Without
					// this the page would jump by everything the control already
					// scrolled, since that distance is still in the drag total.
					anchorTouchGesture(x, y);
					istouchownedbycontrol = false;
					break;
				}
				istouchscrolling = true;
				// The gesture turned out to be a scroll, so the press the control is
				// holding is taken back rather than completed. The threshold is only
				// 12 units, so the finger is almost certainly still inside the
				// control it started on and a release at its position would fire it.
				if(isframeset) guimanager->mouseReleased(cancelledreleaseposition, cancelledreleaseposition, 0);
			}
		}
		if(istouchscrolling) {
			// At most one of the two axes can overflow for a given design, and
			// the setters clamp to a range of zero on the other one, so both are
			// applied without needing to pick an axis first.
			int targetx = touchstartscrollx + (touchstartx - x);
			int targety = touchstartscrolly + (touchstarty - y);
			setScrollPosition(targetx, targety);
			// Whatever the setters refused to take is how far the finger has
			// pulled past the end, and that becomes the rubber band.
			applyDragOverscroll(targetx, targety);
			// Measured from the scroll that actually took effect rather than from
			// the finger, so a drag that is already pinned against an edge builds
			// up no speed and cannot fling away from it on release.
			trackTouchVelocity();
		}
		break;
	}
	case ACTIONTYPE_UP:
		if(istouchpressed && !istouchscrolling && isframeset) {
			// A drag a control consumed is not a tap on it either: releasing at
			// the finger's position would select whatever list row it happens to
			// end on. Only a gesture that never became a drag is a real release.
			if(istouchownedbycontrol) {
				guimanager->mouseReleased(cancelledreleaseposition, cancelledreleaseposition, 0);
			} else {
				guimanager->mouseReleased(x, y, 0);
			}
		}
		// The finger has left, so the hover synthesised on the way in is taken
		// back. Left standing it would keep the control looking hovered and, worse,
		// keep it receiving drags belonging to gestures elsewhere on the page.
		if(isframeset) guimanager->mouseMoved(cancelledreleaseposition, cancelledreleaseposition);
		// A flick hands its speed over to the page; a slow drag just stops where
		// it was let go. Only a real release does this, never a cancel: there the
		// system took the gesture away and the user did not throw anything.
		if(istouchscrolling &&
				(std::fabs(flingvelocityx) >= flingminstartspeed ||
				 std::fabs(flingvelocityy) >= flingminstartspeed)) {
			isflinging = true;
			flingpositionx = (float)gRenderer::getScrollX();
			flingpositiony = (float)gRenderer::getScrollY();
		} else {
			flingvelocityx = 0.0f;
			flingvelocityy = 0.0f;
		}
		istouchpressed = false;
		istouchscrolling = false;
		istouchrebaseneeded = false;
		break;
	case ACTIONTYPE_CANCEL:
		// Released so the control does not stay stuck pressed, but from outside
		// itself: the system withdrew the gesture, so this is not a click.
		if(istouchpressed && !istouchscrolling && isframeset) {
			guimanager->mouseReleased(cancelledreleaseposition, cancelledreleaseposition, 0);
		}
		if(isframeset) guimanager->mouseMoved(cancelledreleaseposition, cancelledreleaseposition);
		cancelTouchGesture();
		break;
	default:
		break;
	}
}

void gAppManager::anchorTouchGesture(int touchX, int touchY) {
	touchstartx = touchX;
	touchstarty = touchY;
	touchstartscrollx = gRenderer::getScrollX();
	touchstartscrolly = gRenderer::getScrollY();
	// A finger is on the glass, so it owns the page from here on. Catching a
	// running fling this way is what lets the user stop a long page mid flight.
	isflinging = false;
	flingvelocityx = 0.0f;
	flingvelocityy = 0.0f;
	// The measurement restarts with the gesture: the interval across a re-anchor
	// spans two different fingers and would read as a jump.
	hastouchvelocity = false;
}

void gAppManager::cancelTouchGesture() {
	// The gesture was withdrawn rather than finished: the system claimed it, or
	// the app was sent away in the middle of it. The user never asked for the
	// distance travelled so far, so the page goes back to where the gesture
	// found it. Without this, every trip through the recents switcher leaves the
	// page a notch further down, because the swipe that opens it is delivered
	// here before the system takes it over.
	if(istouchscrolling) {
		setScrollPosition(touchstartscrollx, touchstartscrolly);
	}
	// A withdrawn gesture throws nothing, so any fling it just started is dropped.
	gGUIScrollable::cancelContentFling();
	resetTouchGesture();
}

int gAppManager::measureLayoutContentHeight(int referenceHeight) {
	if(!guimanager || !guimanager->isframeset) return 0;
	gGUIFrame* frame = guimanager->getCurrentFrame();
	if(!frame) return 0;
	gGUISizer* sizer = frame->getSizer();
	if(!sizer) return 0;
	// The reference is handed over rather than left to the sizer to remember. It
	// is what the page would be tall without any growing, so it is also what a row
	// with nothing to say about itself is entitled to - and it is recomputed here
	// on every rotation, which a remembered one never would be.
	sizer->setReferenceHeight(referenceHeight);
	return sizer->getNaturalHeight();
}

void gAppManager::updateLayoutContentHeight() {
	if(!iscontentdrivenheight || !isScrollableLayoutActive()) return;
	if(ungrownlayoutheight <= 0) return;
	// Nothing announces that a row has been added, so the measurement is repeated
	// every frame - a walk of a few dozen slots adding integers. What is expensive
	// is rebuilding the layout, and that only happens when the answer has actually
	// changed.
	int contentheight = measureLayoutContentHeight(ungrownlayoutheight);
	if(contentheight == lastcontentheight) return;
	lastcontentheight = contentheight;
	updateScrollableLayout(renderer->getScreenWidth(), renderer->getScreenHeight());
	if(guimanager && guimanager->isframeset) {
		guimanager->windowResized(renderer->getWidth(), renderer->getHeight());
	}
}

void gAppManager::enableContentDrivenHeight(bool isEnabled) {
	if(iscontentdrivenheight == isEnabled) return;
	iscontentdrivenheight = isEnabled;
	lastcontentheight = -1;
	if(isScrollableLayoutActive()) {
		updateScrollableLayout(renderer->getScreenWidth(), renderer->getScreenHeight());
		if(guimanager && guimanager->isframeset) {
			guimanager->windowResized(renderer->getWidth(), renderer->getHeight());
		}
	}
}

bool gAppManager::isContentDrivenHeightEnabled() const {
	return iscontentdrivenheight;
}

void gAppManager::setOverscroll(float overscrollX, float overscrollY) {
	overscrollx = overscrollX;
	overscrolly = overscrollY;
	// Kept in floats here and handed over rounded, so a slow spring is not
	// rounded away frame after frame and left stuck a unit short of home.
	gRenderer::setOverscroll((int)std::lround(overscrollx), (int)std::lround(overscrolly));
}

float gAppManager::resistOverscroll(float distance) {
	// The further the page is pulled past its end, the less of the finger's
	// movement it gives up: the offset approaches overscrollmax without ever
	// reaching it. Following the finger one to one out here would let the page be
	// dragged clean off the screen and would give nothing to push back against.
	float magnitude = std::fabs(distance);
	float resisted = overscrollmax * magnitude / (magnitude + overscrollmax);
	return distance < 0.0f ? -resisted : resisted;
}

void gAppManager::applyDragOverscroll(int targetX, int targetY) {
	// Only an axis that scrolls at all can be pulled past its end. On one that
	// fits, the whole page would otherwise slide about under any stray drag.
	float overx = 0.0f;
	float overy = 0.0f;
	if(gRenderer::getMaxScrollX() > 0) {
		overx = resistOverscroll((float)(targetX - gRenderer::getScrollX()));
	}
	if(gRenderer::getMaxScrollY() > 0) {
		overy = resistOverscroll((float)(targetY - gRenderer::getScrollY()));
	}
	setOverscroll(overx, overy);
}

void gAppManager::updateOverscroll() {
	// A finger still on the glass is holding the page out on purpose.
	if(istouchpressed && istouchscrolling) return;
	if(overscrollx == 0.0f && overscrolly == 0.0f) return;
	float step = (float)getElapsedTime();
	if(step <= 0.0f) return;
	if(step > flingmaxstep) step = flingmaxstep;

	float decay = std::exp(-overscrollreturnrate * step);
	float newx = overscrollx * decay;
	float newy = overscrolly * decay;
	if(std::fabs(newx) < overscrollstopdistance) newx = 0.0f;
	if(std::fabs(newy) < overscrollstopdistance) newy = 0.0f;
	setOverscroll(newx, newy);
}

void gAppManager::resetTouchGesture() {
	istouchpressed = false;
	istouchscrolling = false;
	istouchrebaseneeded = false;
	istouchownedbycontrol = false;
	isflinging = false;
	flingvelocityx = 0.0f;
	flingvelocityy = 0.0f;
	hastouchvelocity = false;
	// Dropped rather than sprung back: the gesture is gone, and so is any reason
	// to animate the page returning from where it had pulled it.
	setOverscroll(0.0f, 0.0f);
}

void gAppManager::trackTouchVelocity() {
	AppClockTimePoint now = AppClock::now();
	int scrollx = gRenderer::getScrollX();
	int scrolly = gRenderer::getScrollY();
	if(hastouchvelocity) {
		float interval = std::chrono::duration<float>(now - lasttouchmovetime).count();
		if(interval > velocitymininterval && interval < velocitymaxinterval) {
			float instantx = (float)(scrollx - lasttouchscrollx) / interval;
			float instanty = (float)(scrolly - lasttouchscrolly) / interval;
			flingvelocityx = flingvelocityx * (1.0f - velocitysmoothing) + instantx * velocitysmoothing;
			flingvelocityy = flingvelocityy * (1.0f - velocitysmoothing) + instanty * velocitysmoothing;
		}
	}
	lasttouchmovetime = now;
	lasttouchscrollx = scrollx;
	lasttouchscrolly = scrolly;
	hastouchvelocity = true;
}

void gAppManager::updateFling() {
	if(!isflinging) return;
	float step = (float)getElapsedTime();
	if(step <= 0.0f) return;
	if(step > flingmaxstep) step = flingmaxstep;

	flingpositionx += flingvelocityx * step;
	flingpositiony += flingvelocityy * step;
	float decay = std::exp(-flingdecayrate * step);
	flingvelocityx *= decay;
	flingvelocityy *= decay;

	int targetx = (int)std::lround(flingpositionx);
	int targety = (int)std::lround(flingpositiony);
	setScrollPosition(targetx, targety);
	// The setters clamp, so a target beyond the end means that axis has hit the
	// edge. Its speed is dropped and the float position pulled back onto the
	// clamped value; otherwise speed would keep accumulating out of view and the
	// page would sit still for a while before moving again.
	if(gRenderer::getScrollX() != targetx) {
		// What is left of the throw is turned into the bounce instead of being
		// dropped, so arriving at the end fast looks different from arriving slow.
		// Only when nothing is bouncing yet, or a fling grinding along the end
		// would keep topping the offset up and never let the spring finish.
		if(overscrollx == 0.0f) setOverscroll(resistOverscroll(flingvelocityx * overscrollbouncetime), overscrolly);
		flingvelocityx = 0.0f;
		flingpositionx = (float)gRenderer::getScrollX();
	}
	if(gRenderer::getScrollY() != targety) {
		if(overscrolly == 0.0f) setOverscroll(overscrollx, resistOverscroll(flingvelocityy * overscrollbouncetime));
		flingvelocityy = 0.0f;
		flingpositiony = (float)gRenderer::getScrollY();
	}

	if(std::fabs(flingvelocityx) < flingstopspeed && std::fabs(flingvelocityy) < flingstopspeed) {
		isflinging = false;
	}
}

void gAppManager::updateScrollIndicator() {
	int scrollx = gRenderer::getScrollX();
	int scrolly = gRenderer::getScrollY();
	// Shown while the page is moving and while a finger is holding it, so that
	// it is already on screen the moment the drag begins.
	// A stretch counts as movement: the scroll position stops changing the moment
	// the end is reached, and without this the indicator would start fading out in
	// the middle of the bounce.
	bool isstretched = overscrollx != 0.0f || overscrolly != 0.0f;
	if(scrollx != scrollindicatorlastscrollx || scrolly != scrollindicatorlastscrolly ||
			istouchscrolling || isstretched) {
		scrollindicatorlastscrollx = scrollx;
		scrollindicatorlastscrolly = scrolly;
		scrollindicatortimer = scrollindicatorholdtime + scrollindicatorfadetime;
		return;
	}
	if(scrollindicatortimer > 0.0f) {
		scrollindicatortimer -= (float)getElapsedTime();
		if(scrollindicatortimer < 0.0f) scrollindicatortimer = 0.0f;
	}
}

void gAppManager::drawScrollIndicator() {
	if(scrollindicatortimer <= 0.0f) return;
	int maxscrollx = gRenderer::getMaxScrollX();
	int maxscrolly = gRenderer::getMaxScrollY();
	if(maxscrollx <= 0 && maxscrolly <= 0) return;

	float opacity = 1.0f;
	if(scrollindicatortimer < scrollindicatorfadetime) {
		opacity = scrollindicatortimer / scrollindicatorfadetime;
	}

	// The projection maps the visible band, so the band's own coordinates are
	// what keeps the indicator pinned to the screen instead of scrolling away
	// with the page. The rubber band offset is part of where the band actually
	// is, so it has to be included: leaving it out slides the indicator off the
	// edge exactly when the stretch is happening.
	int bandleft = gRenderer::getScrollX() + gRenderer::getOverscrollX();
	int bandtop = gRenderer::getScrollY() + gRenderer::getOverscrollY();
	int bandwidth = gRenderer::getUnitViewportWidth();
	int bandheight = gRenderer::getUnitViewportHeight();

	bool isalpha = renderer->isAlphaBlendingEnabled();
	if(!isalpha) renderer->enableAlphaBlending();
	// Copied by value: getColor() hands out the renderer's own color object, so
	// holding the pointer would restore whatever the indicator just set.
	gColor oldcolor = *renderer->getColor();
	renderer->setColor(0, 0, 0, (int)(scrollindicatoralpha * opacity));

	// When both axes show an indicator they share the bottom-right corner: the
	// vertical bar runs the full right edge and the horizontal one the full bottom
	// edge, so scrolled to the end they cross there. Each bar gives up the other's
	// thickness at that corner, so the two stop short of it instead of overlapping.
	int cornergap = (maxscrollx > 0 && maxscrolly > 0) ? scrollindicatorthickness + scrollindicatormargin : 0;

	if(maxscrolly > 0) {
		int tracklength = bandheight - scrollindicatormargin * 2 - cornergap;
		// getUnitHeight() > 0 is implied by maxscrolly > 0 today, but it is the
		// divisor below and guarding it keeps a future change to the scroll model
		// from turning this into a divide-by-zero.
		if(tracklength > scrollindicatorminlength && renderer->getUnitHeight() > 0) {
			int thumblength = (int)((float)tracklength * bandheight / (float)renderer->getUnitHeight());
			if(thumblength < scrollindicatorminlength) thumblength = scrollindicatorminlength;
			if(thumblength > tracklength) thumblength = tracklength;
			int offset = (int)((tracklength - thumblength) * ((float)gRenderer::getScrollY() / maxscrolly));
			int thumbx = bandleft + bandwidth - scrollindicatormargin - scrollindicatorthickness;
			int thumby = bandtop + scrollindicatormargin + offset;
			renderer->setColor(0, 0, 0, (int)(scrollindicatoralpha * opacity));
			gDrawRectangle(thumbx, thumby, scrollindicatorthickness, thumblength, true);
			// While the page is stretched, the tip on the pulled side darkens over a
			// length that grows with the stretch: it reads as the bar pressing against
			// the end rather than sitting frozen there. Positive overscroll is past the
			// bottom, so it is the bottom tip; negative is the top.
			int overscroll = gRenderer::getOverscrollY();
			int darktip = overscrollTipLength(overscroll, thumblength);
			if(darktip > 0) {
				renderer->setColor(0, 0, 0, (int)(scrollindicatordarktipalpha * opacity));
				int darky = overscroll > 0 ? thumby + thumblength - darktip : thumby;
				gDrawRectangle(thumbx, darky, scrollindicatorthickness, darktip, true);
			}
		}
	}
	if(maxscrollx > 0) {
		int tracklength = bandwidth - scrollindicatormargin * 2 - cornergap;
		// Same guard as the vertical bar: getUnitWidth() is the divisor below.
		if(tracklength > scrollindicatorminlength && renderer->getUnitWidth() > 0) {
			int thumblength = (int)((float)tracklength * bandwidth / (float)renderer->getUnitWidth());
			if(thumblength < scrollindicatorminlength) thumblength = scrollindicatorminlength;
			if(thumblength > tracklength) thumblength = tracklength;
			int offset = (int)((tracklength - thumblength) * ((float)gRenderer::getScrollX() / maxscrollx));
			int thumbx = bandleft + scrollindicatormargin + offset;
			int thumby = bandtop + bandheight - scrollindicatormargin - scrollindicatorthickness;
			renderer->setColor(0, 0, 0, (int)(scrollindicatoralpha * opacity));
			gDrawRectangle(thumbx, thumby, thumblength, scrollindicatorthickness, true);
			// The right/left tip darkens the same way the vertical bar's does.
			int overscroll = gRenderer::getOverscrollX();
			int darktip = overscrollTipLength(overscroll, thumblength);
			if(darktip > 0) {
				renderer->setColor(0, 0, 0, (int)(scrollindicatordarktipalpha * opacity));
				int darkx = overscroll > 0 ? thumbx + thumblength - darktip : thumbx;
				gDrawRectangle(darkx, thumby, darktip, scrollindicatorthickness, true);
			}
		}
	}

	renderer->setColor(oldcolor);
	if(!isalpha) renderer->disableAlphaBlending();
}

bool gAppManager::onTouchEvent(gTouchEvent& event) {
	// Touches never reached the GUI otherwise, so in the GUI app modes they are
	// what drives both the controls and the scrolling of the page they sit on.
	if(isguiapp) handleGUITouch(event);
	if(canvasmanager && getCurrentCanvas()) {
		if (
#if GLIST_ANDROID
            auto* target = dynamic_cast<gAndroidCanvas*>(getCurrentCanvas())
#elif GLIST_IOS
            auto* target = static_cast<gIOSCanvas*>(getCurrentCanvas())
#elif GLIST_WEB
			auto* target = static_cast<gWebCanvas*>(getCurrentCanvas())
#endif
        ) {
			// The action index comes from the platform and could point past the input
			// list on a malformed event; clamp it and skip when there is nothing to
			// read, so getInputs()[inputindex] is never out of bounds. Same guard the
			// GUI touch path uses in handleGUITouch().
			if (event.getInputCount() < 1) return false;
			if (event.getAction() == ACTIONTYPE_POINTER_DOWN || (event.getInputCount() == 1 && event.getAction() == ACTIONTYPE_DOWN)) {
				int inputindex = gClamp(event.getActionIndex(), 0, event.getInputCount() - 1);
				TouchInput& input = event.getInputs()[inputindex];
				int x = input.x;
				int y = input.y;
				if(gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
					x = gRenderer::scaleX(x);
					y = gRenderer::scaleY(y);
				}
				target->touchPressed(x, y, input.fingerid);
			} else if (event.getAction() == ACTIONTYPE_POINTER_UP || (event.getInputCount() == 1 && event.getAction() == ACTIONTYPE_UP)) {
				int inputindex = gClamp(event.getActionIndex(), 0, event.getInputCount() - 1);
				TouchInput& input = event.getInputs()[inputindex];
				int x = input.x;
				int y = input.y;
				if(gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
					x = gRenderer::scaleX(x);
					y = gRenderer::scaleY(y);
				}
				target->touchReleased(x, y, input.fingerid);
			} else if (event.getAction() == ACTIONTYPE_MOVE) {
				int inputindex = gClamp(event.getActionIndex(), 0, event.getInputCount() - 1);
				TouchInput& input = event.getInputs()[inputindex];
				int x = input.x;
				int y = input.y;
				if(gRenderer::getScreenScaling() > G_SCREENSCALING_NONE) {
					x = gRenderer::scaleX(x);
					y = gRenderer::scaleY(y);
				}
				target->touchMoved(x, y, input.fingerid);
			}
		}
	}
	return false;
}

#endif

void gAppManager::updateTime() {
	targettimestep = AppClockDuration(1'000'000'000 / (targetframerate + 1));
}

void gAppManager::submitToMainThread(std::function<void()> fn) {
    std::unique_lock<std::mutex> lock(mainthreadqueuemutex);
    mainthreadqueue.emplace_back(fn);
}

void gAppManager::executeQueue() {
	G_PROFILE_ZONE_SCOPED_N("gAppManager::executeQueue()");
	// The queue is taken over before anything runs, so that the callbacks run
	// without the lock held. Holding it across them would block whoever submits
	// next - on Android that is the UI thread, which must not wait on a frame's
	// worth of GUI callbacks - and would deadlock outright if a callback
	// submitted work of its own.
	std::vector<std::function<void()>> queue;
	{
		std::unique_lock<std::mutex> lock(mainthreadqueuemutex);
		queue.swap(mainthreadqueue);
	}
	for (auto& func : queue) {
		func();
	}
}

void gAppManager::preciseSleep(double seconds) {
	G_PROFILE_ZONE_SCOPED_N("gAppManager::preciseSleep()");
    double estimate = 5e-3;
    double mean = 5e-3;
    double m2 = 0;
    double count = 1;
    double observed = 0;
    double delta = 0;
    double stddev = 0;

    while (seconds > estimate) {
        AppClockTimePoint start = AppClock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        AppClockTimePoint end = AppClock::now();

        observed = (end - start).count() / 1'000'000'000.0;
        seconds -= observed;

        count++;
        delta = observed - mean;
        mean += delta / count;
        m2 += delta * (observed - mean);
        stddev = std::sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    AppClockTimePoint start = AppClock::now();
    while ((AppClock::now() - start).count() / 1'000'000'000.0 < seconds);
}
