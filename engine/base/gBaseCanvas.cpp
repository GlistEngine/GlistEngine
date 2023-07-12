/*
 * gBaseCanvas.cpp
 *
 *  Created on: May 6, 2020
 *      Author: noyan
 */

#include "gAppManager.h"
#include "gBaseCanvas.h"

const int gBaseCanvas::MOUSEBUTTON_LEFT = 2;
const int gBaseCanvas::MOUSEBUTTON_RIGHT = 4;
const int gBaseCanvas::MOUSEBUTTON_MIDDLE = 8;


gBaseCanvas::gBaseCanvas(gBaseApp *root) {
	this->root = root;
}

gBaseCanvas::~gBaseCanvas() {
}

void gBaseCanvas::setup() {

}

void gBaseCanvas::update() {

}

void gBaseCanvas::draw() {

}

void gBaseCanvas::keyPressed(int key) {

}

void gBaseCanvas::keyReleased(int key) {

}

void gBaseCanvas::charPressed(unsigned int codepoint) {

}

void gBaseCanvas::mouseMoved(int x, int y ) {

}

void gBaseCanvas::mouseDragged(int x, int y, int button) {

}

void gBaseCanvas::mousePressed(int x, int y, int button) {

}

void gBaseCanvas::mouseReleased(int x, int y, int button) {

}

void gBaseCanvas::mouseScrolled(int x, int y) {

}

void gBaseCanvas::mouseEntered() {

}

void gBaseCanvas::mouseExited() {

}

void gBaseCanvas::windowResized(int w, int h) {

}

void gBaseCanvas::joystickConnected(int joystickId, bool isGamepad, bool isConnected) {

}

void gBaseCanvas::gamepadButtonPressed(int joystickId, int key) {

}

void gBaseCanvas::gamepadButtonReleased(int joystickId, int key) {

}

bool gBaseCanvas::isJoystickConnected(int joystickId) {
	return appmanager->isJoystickConnected(joystickId);

}

bool gBaseCanvas::isGamepadButtonPressed(int joystickId, int buttonId) {
	return appmanager->isGamepadButtonPressed(joystickId, buttonId);
}

void gBaseCanvas::onGuiEvent(int guiObjectId, int eventType, std::string value1, std::string value2) {

}

void gBaseCanvas::onEvent(gEvent& event) {

}

void gBaseCanvas::showNotify() {

}

void gBaseCanvas::hideNotify() {

}

int gBaseCanvas::getWidth() {
	return renderer->getWidth();
}

int gBaseCanvas::getHeight() {
	return renderer->getHeight();
}

void gBaseCanvas::clearBackground() {
	// Clear the background
	clearColor(0, 0, 30, 0);
	setColor(255, 255, 255);
}

void gBaseCanvas::setColor(int r, int g, int b, int a) {
	renderer->setColor(r, g, b, a);
}

void gBaseCanvas::setColor(gColor color) {
	renderer->setColor(color);
}

gColor gBaseCanvas::getColor() {
	return renderer->getColor();
}

void gBaseCanvas::clear() {
	renderer->clear();
}

void gBaseCanvas::clearColor(int r, int g, int b, int a) {
	renderer->clearColor(r, g, b, a);
}

void gBaseCanvas::clearColor(gColor color) {
	renderer->clearColor(color);
}

void gBaseCanvas::enableLighting() {
	renderer->enableLighting();
}

void gBaseCanvas::disableLighting() {
	renderer->disableLighting();
}

bool gBaseCanvas::isLightingEnabled() {
	return renderer->isLightingEnabled();
}

void gBaseCanvas::setLightingColor(int r, int g, int b, int a) {
	renderer->setLightingColor(r, g, b, a);
}

void gBaseCanvas::setLightingColor(gColor* color) {
	renderer->setLightingColor(color);
}

gColor* gBaseCanvas::getLightingColor() {
	return renderer->getLightingColor();
}

void gBaseCanvas::setLightingPosition(glm::vec3 lightingPosition) {
	renderer->setLightingPosition(lightingPosition);
}

glm::vec3 gBaseCanvas::getLightingPosition() {
	return renderer->getLightingPosition();
}

void gBaseCanvas::setGlobalAmbientColor(int r, int g, int b, int a) {
	renderer->setGlobalAmbientColor(r, g, b, a);
}

void gBaseCanvas::setGlobalAmbientColor(gColor color) {
	renderer->setGlobalAmbientColor(color);
}

gColor* gBaseCanvas::getGlobalAmbientColor() {
	return renderer->getGlobalAmbientColor();
}

void gBaseCanvas::addSceneLight(gLight* light) {
	renderer->addSceneLight(light);
}

gLight* gBaseCanvas::getSceneLight(int lightNo) {
	return renderer->getSceneLight(lightNo);
}

int gBaseCanvas::getSceneLightNum() {
	return renderer->getSceneLightNum();
}

void gBaseCanvas::removeSceneLight(gLight* light)  {
	renderer->removeSceneLight(light);
}

void gBaseCanvas::removeAllSceneLights() {
	renderer->removeAllSceneLights();
}

void gBaseCanvas::enableDepthTest() {
	renderer->enableDepthTest();
}

void gBaseCanvas::enableDepthTest(int depthTestType) {
	renderer->enableDepthTest(depthTestType);
}

void gBaseCanvas::setDepthTestFunc(int depthTestType) {
	renderer->setDepthTestFunc(depthTestType);
}

void gBaseCanvas::disableDepthTest() {
	renderer->disableDepthTest();
}

bool gBaseCanvas::isDepthTestEnabled() {
	return renderer->isDepthTestEnabled();
}

int gBaseCanvas::getDepthTestType() {
	return renderer->getDepthTestType();
}


void gBaseCanvas::enableAlphaBlending() {
	renderer->enableAlphaBlending();
}

void gBaseCanvas::disableAlphaBlending() {
	renderer->disableAlphaBlending();
}

void gBaseCanvas::enableAlphaTest() {
	renderer->enableAlphaTest();
}

void gBaseCanvas::disableAlphaTest() {
	renderer->disableAlphaTest();
}

void gBaseCanvas::enableGrid() {
	renderer->enableGrid();
}

void gBaseCanvas::disableGrid() {
	renderer->disableGrid();
}

//set which Grid axis to show or close with xy,yz,xz boolean
void gBaseCanvas::setGridEnableAxis(bool xy, bool yz, bool xz) {
	renderer->setGridEnableAxis(xy, yz, xz);
}

// set Grid XY axis enable or not with xy boolean
void gBaseCanvas::setGridEnableXY(bool xy) {
	renderer->setGridEnableXY(xy);
}

// set Grid XZ axis enable or not with xy boolean
void gBaseCanvas::setGridEnableXZ(bool xz) {
	renderer->setGridEnableXZ(xz);
}

// set Grid YZ axis enable or not with yz boolean
void gBaseCanvas::setGridEnableYZ(bool yz) {
	renderer->setGridEnableYZ(yz);
}

/*
 * set max coordinate to reach for grid. Example: 50 mean (-25 to 25) as coordinate axis
 * @param gridmaxvalue => set max distance for grid.
 */
void gBaseCanvas::setGridMaxLength(float length) {
	renderer->setGridMaxLength(length);
}

/*
 * set color for XZ axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gBaseCanvas::setGridColorofAxisXZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	renderer->setGridColorofAxisXZ(r, g, b, a);
}

/*
 * set color for XZ axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gBaseCanvas::setGridColorofAxisXZ(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	renderer->setGridColorofAxisXZ(color);
}

/*
 * set color for margin of XZ axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gBaseCanvas::setGridColorofAxisWireFrameXZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	renderer->setGridColorofAxisWireFrameXZ(r, g, b, a);
}

/*
 * set color for margin of XZ axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gBaseCanvas::setGridColorofAxisWireFrameXZ(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	renderer->setGridColorofAxisWireFrameXZ(color);
	//rendercolor->set(color);
}


/*
 * set color for XY axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gBaseCanvas::setGridColorofAxisXY(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	renderer->setGridColorofAxisXY(r, g, b, a);
}

/*
 * set color for XY axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gBaseCanvas::setGridColorofAxisXY(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	renderer->setGridColorofAxisXY(color);
	//color->r = 100;
	//rendercolor->set(color);
}

/*
 * set color for margin of XY axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gBaseCanvas::setGridColorofAxisWireFrameXY(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	renderer->setGridColorofAxisWireFrameXY(r, g, b, a);
}

/*
 * set color for margin of XY axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gBaseCanvas::setGridColorofAxisWireFrameXY(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	renderer->setGridColorofAxisWireFrameXY(color);
	//rendercolor->set(color);
}

/*
 * set color for YZ axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gBaseCanvas::setGridColorofAxisYZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	renderer->setGridColorofAxisYZ(r, g, b, a);
}

/*
 * set color for YZ axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gBaseCanvas::setGridColorofAxisYZ(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	renderer->setGridColorofAxisYZ(color);
	//color->r = 100;
	//rendercolor->set(color);
}

/*
 * set color for margin of YZ axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gBaseCanvas::setGridColorofAxisWireFrameYZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	renderer->setGridColorofAxisWireFrameYZ(r, g, b, a);
}

/*
 * set color for margin of YZ axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gBaseCanvas::setGridColorofAxisWireFrameYZ(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	renderer->setGridColorofAxisWireFrameYZ(color);
	//rendercolor->set(color);
}

/*
 * return max length of grid as float
 */
float gBaseCanvas::getGridMaxLength() {
	return renderer->getGridMaxLength();
}

/*
 * set distance between grid lines.
 * @param gridmaxvalue => set max distance for grid.
 */
void gBaseCanvas::setGridLineInterval(float intervalvalue) {
	renderer->setGridLineInterval(intervalvalue);
}

/*
 * return distance between grid lines as float
 */
float gBaseCanvas::getGridLineInterval() {
	return renderer->getGridLineInterval();
}

bool gBaseCanvas::isGridEnabled() {
	return renderer->isGridEnabled();
}

bool gBaseCanvas::isGridXYEnabled() {
	return renderer->isGridXYEnabled();
}

bool gBaseCanvas::isGridXZEnabled() {
	return renderer->isGridXZEnabled();
}

bool gBaseCanvas::isGridYZEnabled() {
	return renderer->isGridYZEnabled();
}



