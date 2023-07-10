//
// Created by Metehan Gezer on 6.07.2023.
//

#include "gBaseComponent.h"
#include "gBasePlugin.h"
#include "gAppManager.h"
#include "gBaseApp.h"
#include "gCanvasManager.h"
#include "gGUIManager.h"
#include <thread>
#include <mutex>

// Platform specific window implementation
#if defined(WIN32) || defined(LINUX) || defined(APPLE)
#include "gGLFWWindow.h"
#elif defined(ANDROID)
#include "gAndroidWindow.h"
#endif

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int width, int height, bool isResizable) {
    gStartEngine(baseApp, appName, windowMode, width, height, G_SCREENSCALING_AUTO, width, height, isResizable);
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int unitWidth, int unitHeight, int screenScaling, int width, int height, bool isResizable) {
    if(windowMode == G_WINDOWMODE_NONE) windowMode = G_WINDOWMODE_APP;
    if(windowMode == G_WINDOWMODE_FULLSCREENGUIAPP || windowMode == G_WINDOWMODE_GUIAPP) screenScaling = G_SCREENSCALING_NONE;
#ifndef ANDROID
    gEngine engine(appName, baseApp, width, height, windowMode, unitWidth, unitHeight, screenScaling, isResizable, G_LOOPMODE_NORMAL);
    engine.setup();
    engine.initialize();
    engine.loop();
    engine.stop();
#else
    new gAppManager(appName, baseApp, width, height, windowMode, unitWidth, unitHeight, screenScaling, isResizable, G_LOOPMODE_NORMAL);
#endif
}

void gStartEngine(gBaseApp* baseApp, const std::string& appName, int loopMode) {
#ifndef ANDROID
    gEngine engine(appName, baseApp, 0, 0, G_WINDOWMODE_NONE, 0, 0, G_SCREENSCALING_NONE, false, loopMode);
    engine.setup();
    engine.initialize();
    engine.loop();
    engine.stop();
#else
    new gAppManager(appName, baseApp, 0, 0, G_WINDOWMODE_NONE, 0, 0, G_SCREENSCALING_NONE, false, loopMode);
#endif
}

int pow(int x, int p) {
    int i = 1;
    for (int j = 1; j <= p; j++) i *= x;
    return i;
}

gAppManager* gAppManager::appmanager = nullptr;

gAppManager::gAppManager(const std::string& appName, gBaseApp *baseApp, int width, int height,
                         int windowMode, int unitWidth, int unitHeight, int screenScaling,
                         bool isResizable, int loopMode) : appname(appName), app(baseApp), width(width), height(height),
                                                                 windowmode(windowMode), unitwidth(unitWidth), unitheight(unitHeight), screenscaling(screenScaling),
                                                                 isresizable(isResizable), loopmode(loopMode) {
    gAppManager::set(this);
    initialized = false;
    initializedbefore = false;
    iscanvasset = false;
    isrunning = false;
    isrendering = false;

    canvasmanager = new gCanvasManager();
    guimanager = new gGUIManager(baseApp, width, height);
    // Mouse
    ismouseentered = false;
    mousebuttonstate = 0;
    for(int i = 0; i < maxmousebuttonnum; i++) {
        mousebuttonpressed[i] = false;
    }
    targetframerate = 60;
    updateTime();
    starttime = AppClock::now();
    endtime = starttime;
    deltatime = AppClockDuration(0);
    totaltime = 0;
    totalupdates = 0;
    totaldraws = 0;
    iswindowfocused = false;

    // Joystick
    isjoystickenabled = false;
    for(int i = 0; i < maxjoysticknum; i++) {
        for(int j = 0; j < maxjoystickbuttonnum; j++) joystickbuttonstate[i][j] = false;
        joystickconnected[i] = false;
    }
    joystickaxecount = 0;

    eventhandler = G_BIND_FUNCTION(onEvent);

#ifdef ANDROID
    window = new gAndroidWindow();
#else
    window = new gGLFWWindow();
#endif
    window->setEventHandler(eventhandler);
    if (appname.empty()) {
        appname = "GlistApp";
    }
    window->setTitle(appname);
}

gAppManager::~gAppManager() {
    delete canvasmanager;
    delete guimanager;
    delete window;
    gRenderObject::destroyRenderer();
}

void gAppManager::setup() {
    app->setup();
}

void gAppManager::initialize() {
    if(initialized || loopmode == G_LOOPMODE_NONE) {
        return;
    }
    deltatime = AppClockDuration(0);

    window->initialize(width, height, windowmode, isresizable);
    // Update size
    width = window->getWidth();
    height = window->getHeight();
    if(unitwidth == 0) {
        unitwidth = this->width;
    }
    if(unitheight == 0) {
        unitheight = this->height;
    }
    // Create renderer and reallocate (if available) all gpu resources.
    gRenderObject::createRenderer();
    if(canvasmanager->getCurrentCanvas() || canvasmanager->getTempCanvas()) {
        gBaseCanvas::setScreenSize(width, height);
        gBaseCanvas::setUnitScreenSize(unitwidth, unitheight);
        gBaseCanvas::setScreenScaling(screenscaling);
    }
    if(initializedbefore) {
        gAllocatableBase::reallocateAll();
        if(eventhandler) {
            gReallocateRenderDataEvent event{};
            eventhandler(event);
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
    if(window) {
        assert(gRenderObject::getRenderer());
    }
#endif
    gLogi("gAppManager") << "starting loop";
    isrunning = true;
    while (isrunning && (!window || !window->getShouldClose())) {
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

        if(!window || !window->vsync) {
            double sleepTime = (targettimestep - (AppClock::now() - starttime)).count() / 1'000'000'000;
            if(sleepTime > 0) {
                preciseSleep(sleepTime);
            }
        }
    }
    gLogi("gAppManager") << "stopping loop";
    app->stop();
    gRenderObject::destroyRenderer();
    if(window) {
        window->close();
    }
    initialized = false;
}

void gAppManager::stop() {
    isrunning = false;
}

void gAppManager::setScreenSize(int width, int height) {
    canvasmanager->getCurrentCanvas()->setScreenSize(width, height);
    if(iscanvasset) canvasmanager->getCurrentCanvas()->windowResized(width, height);
    if(iscanvasset && guimanager->isframeset) guimanager->windowResized(width, height);
}

void gAppManager::setCurrentCanvas(gBaseCanvas* canvas) {
    canvasmanager->setCurrentCanvas(canvas);
    iscanvasset = true;
}

gBaseCanvas* gAppManager::getCurrentCanvas() const {
    return canvasmanager->getCurrentCanvas();
}

void gAppManager::setTargetFramerate(int framerate) {
    targetframerate = framerate;
}

int gAppManager::getTargetFramerate() const {
    return targetframerate;
}

int gAppManager::getFramerate() const {
    return (uint32_t)(1'000'000'000 / deltatime.count());
}

void gAppManager::enableVsync() {
    window->setVsync(false);
}

void gAppManager::disableVsync() {
    window->setVsync(false);
}

double gAppManager::getElapsedTime() const {
    return deltatime.count() / 1'000'000'000.0;
}

void gAppManager::setClipboardString(const std::string &clipboard) {
    window->setClipboardString(clipboard);
}

std::string gAppManager::getClipboardString() const {
    return window->getClipboardString();
}

int gAppManager::getWindowMode() const {
    return windowmode;
}

void gAppManager::setCursor(int cursorId) {
    window->setCursor(cursorId);
}

void gAppManager::setCursorMode(int cursorMode) {
    window->setCursorMode(cursorMode);
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

void gAppManager::tick() {
    totalupdates++;
    if(!window) {
        app->update();
        for(int i = 0; i < gBaseComponent::usedcomponents.size(); i++) {
            gBaseComponent::usedcomponents[i]->update();
        }
		executeQueue();
        return;
    }

    // todo joystick
    canvasmanager->update();
    guimanager->update();
    app->update();
    for(int i = 0; i < gBaseComponent::usedcomponents.size(); i++) {
        gBaseComponent::usedcomponents[i]->update();
    }
    for(int i = 0; i < gBasePlugin::usedplugins.size(); i++) {
        gBasePlugin::usedplugins[i]->update();
    }

    gBaseCanvas* canvas = canvasmanager->getCurrentCanvas();
    if(canvas) {
        canvas->update();
    }

    if(isrendering) {
        if(canvas) {
            canvas->clearBackground();
            for (int i = 0; i < renderpassnum; i++) {
                renderpassno = i;
                canvas->draw();
            }
        }

        guimanager->draw();
        totaldraws++;
    }
    window->update();
    if(!window->isRendering() && isrendering) {
        isrendering = false; // If window has lost the context, we should stop rendering.
		app->pause();
    }
	executeQueue();
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
#ifdef ANDROID
    dispatcher.dispatch<gAppPauseEvent>(G_BIND_FUNCTION(onAppPauseEvent));
    dispatcher.dispatch<gAppResumeEvent>(G_BIND_FUNCTION(onAppResumeEvent));
#endif
    if(canvasmanager && canvasmanager->getCurrentCanvas()) canvasmanager->getCurrentCanvas()->onEvent(event);
    // todo pass event to app and plugins
}

bool gAppManager::onWindowResizedEvent(gWindowResizeEvent& event) {
    if (!canvasmanager || !canvasmanager->getCurrentCanvas() || !initialized) return true;
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
    if (mousebuttonstate) {
        if(guimanager->isframeset) guimanager->mouseDragged(xpos, ypos, mousebuttonstate);
        for(gBasePlugin*& plugin : gBasePlugin::usedplugins) plugin->mouseDragged(xpos, ypos, mousebuttonstate);
        canvasmanager->getCurrentCanvas()->mouseDragged(xpos, ypos, mousebuttonstate);
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
    mousebuttonstate |= pow(2, event.getMouseButton() + 1);
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
    mousebuttonstate &= ~pow(2, event.getMouseButton() + 1);
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

bool gAppManager::onAppPauseEvent(gAppPauseEvent& ) {
    submitToMainThread([this]() {
		if(!isrendering) {
			return;
		}
        isrendering = false;
        app->pause();
    });
    return false;
}

bool gAppManager::onAppResumeEvent(gAppResumeEvent &) {
    submitToMainThread([this]() {
		if(isrendering) {
			return;
		}
        isrendering = true;
        app->resume();
    });
    return false;
}

void gAppManager::updateTime() {
    targettimestep = AppClockDuration(1'000'000'000 / (targetframerate + 1));
}

void gAppManager::submitToMainThread(std::function<void()> fn) {
    // todo use scoped_lock when switched to c++17
    std::unique_lock<std::mutex> lock(mainthreadqueuemutex);
    mainthreadqueue.emplace_back(fn);
}

void gAppManager::executeQueue() {
    // todo use scoped_lock when switched to c++17
    std::unique_lock<std::mutex> lock(mainthreadqueuemutex);
    for (auto& func : mainthreadqueue) {
        func();
    }
    mainthreadqueue.clear();
}

void gAppManager::preciseSleep(double seconds) {
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

        observed = (end - start).count() / 1'000'000'000;
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
    while ((AppClock::now() - start).count() / 1'000'000'000 < seconds);
}


#ifdef ANDROID
extern "C" {

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onCreate(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onCreate";
    androidMain();
    gAppManager* engine = gAppManager::get();
#ifdef DEBUG
    assert(engine); // engine should not be null after androidMain();
#endif
    engine->setup();
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onDestroy(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onDestroy";
    gAppManager* engine = gAppManager::get();
    delete engine;
    gAppManager::set(nullptr);
}

std::unique_ptr<std::thread> thread;
JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onStart(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onStart";
    if(thread) {
        throw std::runtime_error("cannot call onStart without calling onStop first");
    }
    thread = std::make_unique<std::thread>([]() {
        gAppManager* engine = gAppManager::get();
        engine->initialize();
        engine->loop();
    });
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onStop(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onStop";
    gAppManager* engine = gAppManager::get();
    if(engine) {
        engine->stop();
    }
    if(thread) {
        thread->join(); // wait for shutdown
        thread = nullptr;
    }
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onPause(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onPause";
    gAppManager* engine = gAppManager::get();
    if(engine) {
        gAppPauseEvent event{};
        engine->getEventHandler()(event);
    }
}

JNIEXPORT void JNICALL Java_dev_glist_android_lib_GlistNative_onResume(JNIEnv *env, jclass clazz) {
    gLogi("GlistNative") << "onResume";
    gAppManager* engine = gAppManager::get();
    if(engine) {
        gAppResumeEvent event{};
        engine->getEventHandler()(event);
    }
}
}
#endif