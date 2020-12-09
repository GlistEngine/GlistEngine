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
	virtual void mouseMoved(int x, int y );
	virtual void mouseDragged(int x, int y, int button);
	virtual void mousePressed(int x, int y, int button);
	virtual void mouseReleased(int x, int y, int button);
	virtual void mouseEntered();
	virtual void mouseExited();

	virtual void showNotify();
	virtual void hideNotify();

	void clearBackground();

protected:
	gBaseApp *root;

	int getWidth();
	int getHeight();
	int getScreenWidth();
	int getScreenHeight();

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
};

#endif /* ENGINE_BASE_GBASECANVAS_H_ */
