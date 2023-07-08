//
// Created by Metehan Gezer on 6.07.2023.
//

#ifndef GENGINEMANAGER_H
#define GENGINEMANAGER_H

#include "gObject.h"
#include "gBaseWindow.h"
#include "gEvent.h"
#include "gWindowEvents.h"

class gCanvasManager;
class gBaseCanvas;
class gBaseApp;
class gGUIManager;
class gEngine;

/**
 * Sets the app settings for the engine according to given name, mode of window,
 * application window's widh and height. Then calls the runApp method to give
 * output to the screen. GlistEngine has scaling methods. User and developer can
 * have different resolutions, but the engine can adapt to users resolution by
 * scaling. In this function, screen scaling is done automatically
 * by the Engine. If developers want to set the scaling, they are encouraged to
 * use overrided gStartEngine function instead.
 *
 * @param baseApp Current app object.
 *
 * @param appName Name of the application.
 *
 * @param windowMode Type of the application window. This parameter can be one of
 * G_WINDOWMODE_GAME, G_WINDOWMODE_APP, G_WINDOWMODE_FULLSCREEN, G_WINDOWMODE_GUIAPP
 * and G_WINDOWMODE_FULLSCREENGUIAPP. This value can not be G_WINDOWMODE_NONE as
 * the app needs a window to work. The developer who needs a windowless console app
 * should use other gStartEngine function. This function defaults to G_WINDOWMODE_APP
 * in case of no window.
 *
 * @param width application window's width to be shown on the user's computer. User
 * should choose this parameter suitable according to their screen resolution
 * to prevent distorted images. Please note that some operating systems may scale
 * the framebuffer and create a bigger window than the developer's value.
 *
 * @param height application window's width to be shown on the user's computer.
 * User should choose this parameter suitable according to their screen resolution
 * to prevent distorted images. Please note that some operating systems may scale
 * the framebuffer and create a bigger window than the developer's value.
 */
void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int width, int height, bool isResizable = true);

/**
 * Sets the app settings for engine according to given name, mode of window,
 * application window's widh and height. Beside that, this is an overrided
 * method of gStartEngine, user should give scaling and unit parameters
 * addition to the baseApp object, app name, windowmode, width and height.
 *
 * Users are encouraged to use this function if they want to scale the
 * output.
 *
 * @param baseApp Current app object.
 *
 * @param appName Name of the application.
 *
 * @param windowMode Type of the application window. This parameter can be one of
 * G_WINDOWMODE_GAME, G_WINDOWMODE_APP, G_WINDOWMODE_FULLSCREEN, G_WINDOWMODE_GUIAPP
 * and G_WINDOWMODE_FULLSCREENGUIAPP. This value can not be G_WINDOWMODE_NONE as
 * the app needs a window to work. The developer who needs a windowless console app
 * should use other gStartEngine function. This function defaults to G_WINDOWMODE_APP
 * in case of no window.
 *
 * @param unitWidth width of the developer's application window. The resolution can
 * differ from developer to user. GlistEngine has scaling methods. User and
 * developer can have different resolutions, but the engine can adapt to users
 * resolution by scaling. Thus, unitWidth is developer's unit width.
 *
 * @param unitHeight height of the developer's application window. The resolution
 * can differ from developer to user. GlistEngine has scaling methods. User and
 * developer can have different resolutions, but the engine can adapt to users
 * resolution by scaling. Thus, unitHeight is developer's unit height.
 *
 * @param screenScaling how much everything should be enlarged when measured in
 * pixels.
 *
 * @param width application window's width to be shown on the user's computer. User
 * should choose this parameter suitable according to their screen resolution
 * to prevent distorted images. Please note that some operating systems may scale
 * the framebuffer and create a bigger window than the developer's value.
 *
 * @param height application window's width to be shown on the user's computer.
 * User should choose this parameter suitable according to their screen resolution
 * to prevent distorted images. Please note that some operating systems may scale
 * the framebuffer and create a bigger window than the developer's value.
 */
void gStartEngine(gBaseApp* baseApp, const std::string& appName, int windowMode, int unitWidth, int unitHeight, int screenScaling, int width, int height, bool isResizable = true);

/**
 * Creates a windowless console/service app.
 *
 * Developers should not forget to remove all calls to Canvas classes, plugins and
 * other rendering elements from the code and from the CMakeLists.
 *
 * The app's loop mode can be one of G_LOOPMODE_NORMAL and G_LOOPMODE_NONE.
 *
 * If the loopmode is G_LOOPMODE_NONE and if the developer needs a component to update,
 * update() function should be called manually.
 */
void gStartEngine(gBaseApp* baseApp, const std::string& appName, int loopMode);

class gEngine : public gObject {
public:
    gEngine(const std::string& appName, gBaseApp *baseApp, int width, int height, int windowMode, int unitWidth, int unitHeight, int screenScaling, bool isResizable, int loopMode);
    ~gEngine();

    void setup();
    void initialize();
    void loop();
    void stop();

    /**
     * Sets screen size by given width and height. GlistEngine scales the process.
     *
     * @param width new width value of screen.
     *
     * @param height new height value of screen.
     */
    void setScreenSize(int width, int height);
    void setCurrentCanvas(gBaseCanvas* canvas);
    gBaseCanvas* getCurrentCanvas() const;
    void setTargetFramerate(int framerate);
    int getTargetFramerate() const;
    int getFramerate() const;
    void enableVsync();
    void disableVsync();
    double getElapsedTime() const;

    void setClipboardString(const std::string& clipboard);
    std::string getClipboardString() const;

    int getWindowMode() const;

    void setCursor(int cursorId);
    void setCursorMode(int cursorMode);

    bool isJoystickConnected(int joystickId);
    int getJoystickAxesCount(int joystickId);
    const float* getJoystickAxes(int joystickId);
    bool isGamepadEnabled() { return isjoystickenabled; }
    bool isJoystickButtonPressed(int joystickId, int buttonId);
    int getMaxJoystickNum() { return maxjoysticknum; }
    int getMaxJoystickButtonNum() { return maxjoystickbuttonnum; }

    gBaseWindow* getWindow() const { return window; }

    /**
     * Submits a function to run on the main loop.
     * Queue is executed after every tick.
     *
     * @param fn Function to submit
     */
    void submitToMainThread(std::function<void()> fn);

    static gEngine* get() { return engine; }
    static void set(gEngine* enginePtr) { gEngine::engine = enginePtr; }
private:
    static const int maxjoysticknum = 4;
    static const int maxjoystickbuttonnum = 15;
    static const int maxmousebuttonnum = 3;

    using AppClock = std::chrono::steady_clock;
    using AppClockDuration = AppClock::duration;
    using AppClockTimePoint = AppClock::time_point;

    gCanvasManager* canvasmanager;
    gGUIManager* guimanager;

    EventHandlerFn eventhandler;
    gBaseWindow* window;

    std::string appname;
    gBaseApp* app;
    int width;
    int height;
    int windowmode;
    int unitwidth;
    int unitheight;
    int screenscaling;
    bool isresizable;
    int loopmode;
    bool initialized;
    bool initializedbefore;
    bool iswindowfocused;

    bool ismouseentered;
    bool mousebuttonpressed[maxmousebuttonnum];
    int mousebuttonstate;

    AppClockTimePoint starttime, endtime;
    AppClockDuration deltatime;
    AppClockDuration targettimestep;
    uint64_t totaltime;

    int totalupdates;
    int totaldraws;
    int targetframerate;
    bool iscanvasset;

    bool isjoystickenabled;
    bool joystickconnected[maxjoysticknum];
    bool joystickbuttonstate[maxjoysticknum][maxjoystickbuttonnum];
    int joystickaxecount;

    std::vector<std::function<void()>> mainthreadqueue;
    std::mutex mainthreadqueuemutex;

private:
    static gEngine* engine;

    void tick();
    void onEvent(gEvent& event);

    bool onWindowResizedEvent(gWindowResizeEvent&);
    bool onCharTypedEvent(gCharTypedEvent&);
    bool onKeyPressedEvent(gKeyPressedEvent&);
    bool onKeyReleasedEvent(gKeyReleasedEvent&);
    bool onMouseMovedEvent(gMouseMovedEvent&);
    bool onMouseButtonPressedEvent(gMouseButtonPressedEvent&);
    bool onMouseButtonReleasedEvent(gMouseButtonReleasedEvent&);
    bool onWindowMouseEnterEvent(gWindowMouseEnterEvent&);
    bool onWindowMouseExitEvent(gWindowMouseExitEvent&);
    bool onMouseScrolledEvent(gMouseScrolledEvent&);
    bool onWindowFocusEvent(gWindowFocusEvent&);
    bool onWindowLoseFocusEvent(gWindowLoseFocusEvent&);
    bool onJoystickConnectEvent(gJoystickConnectEvent&);
    bool onJoystickDisconnectEvent(gJoystickDisconnectEvent&);

    void updateTime();
    void executeQueue();

    static void preciseSleep(double seconds);
};

#endif //GENGINEMANAGER_H
