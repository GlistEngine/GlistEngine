/*
 * gBaseCanvas.h
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#ifndef ENGINE_BASE_GBASECANVAS_H_
#define ENGINE_BASE_GBASECANVAS_H_

#include "gBaseApp.h"
#include "gRenderObject.h"


class gBaseCanvas : public gRenderObject {
public:
	static const int MOUSEBUTTON_LEFT, MOUSEBUTTON_RIGHT, MOUSEBUTTON_MIDDLE;

	gBaseCanvas(gBaseApp *root);
	virtual ~gBaseCanvas();

	virtual void setup();
	virtual void update();
	virtual void draw();

	virtual void keyPressed(int key);
	virtual void keyReleased(int key);
	virtual void charPressed(unsigned int codepoint);
	virtual void mouseMoved(int x, int y );
	virtual void mouseDragged(int x, int y, int button);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void mouseScrolled(int x, int y);
	virtual void mouseEntered();
	virtual void mouseExited();
	virtual void windowResized(int w, int h);
	virtual void joystickConnected(int joystickId, bool isGamepad, bool isConnected);
	virtual void gamepadButtonPressed(int joystickId, int key);
	virtual void gamepadButtonReleased(int joystickId, int key);
	virtual void onGuiEvent(int guiObjectId, int eventType, std::string value1 = "", std::string value2 = "");

	virtual void onEvent(gEvent& event);

#ifdef ANDROID
  virtual void onDeviceOrientationChange(DeviceOrientation deviceorientation);
#endif

	virtual void showNotify();
	virtual void hideNotify();

	void clearBackground();

protected:
	gBaseApp *root;

	int getWidth();
	int getHeight();

	void setColor(int r, int g, int b, int a = 255);
	void setColor(gColor color);
	gColor getColor();

	void clear();
	void clearColor(int r, int g, int b, int a = 255);
	void clearColor(gColor color);

	void enableLighting();
	void disableLighting();
	bool isLightingEnabled();
	void setLightingColor(int r, int g, int b, int a = 255);
	void setLightingColor(gColor* color);
	gColor* getLightingColor();
	void setLightingPosition(glm::vec3 lightingPosition);
	glm::vec3 getLightingPosition();
	void setGlobalAmbientColor(int r, int g, int b, int a = 255);
	void setGlobalAmbientColor(gColor color);
	gColor* getGlobalAmbientColor();

	void addSceneLight(gLight* light);
	gLight* getSceneLight(int lightNo);
	int getSceneLightNum();
	void removeSceneLight(gLight* light);
	void removeAllSceneLights();

	void enableDepthTest();
	void enableDepthTest(int depthTestType);
	void setDepthTestFunc(int depthTestType);
	void disableDepthTest();
	bool isDepthTestEnabled();
	int getDepthTestType();

	void enableAlphaBlending();
	void disableAlphaBlending();
	void enableAlphaTest();
	void disableAlphaTest();

	void enableGrid();
	void disableGrid();
	void setGridEnableAxis(bool xy, bool yz, bool xz);
	void setGridEnableXY(bool xy), setGridEnableYZ(bool yz), setGridEnableXZ(bool xz);
	void setGridMaxLength(float length);
	float getGridMaxLength();
	void setGridLineInterval(float intervalvalue);
	float getGridLineInterval();
	void setGridColorofAxisXZ(int r, int g, int b, int a);
	void setGridColorofAxisYZ(int r, int g, int b, int a);
	void setGridColorofAxisXY(int r, int g, int b, int a);
	void setGridColorofAxisXZ(gColor* color);
	void setGridColorofAxisYZ(gColor* color);
	void setGridColorofAxisXY(gColor* color);
	void setGridColorofAxisWireFrameXZ(int r, int g, int b, int a);
	void setGridColorofAxisWireFrameYZ(int r, int g, int b, int a);
	void setGridColorofAxisWireFrameXY(int r, int g, int b, int a);
	void setGridColorofAxisWireFrameXZ(gColor* color);
	void setGridColorofAxisWireFrameYZ(gColor* color);
	void setGridColorofAxisWireFrameXY(gColor* color);
	bool isGridEnabled(), isGridXYEnabled(), isGridYZEnabled(), isGridXZEnabled();

	bool isJoystickConnected(int joystickId);
	bool isGamepadButtonPressed(int joystickId, int buttonId);
};

#endif /* ENGINE_BASE_GBASECANVAS_H_ */





