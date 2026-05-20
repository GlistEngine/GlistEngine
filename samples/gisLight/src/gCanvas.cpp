/*
* gCanvas.cpp
*
*  Created on: May 6, 2020
*      Author: Noyan Culum
*/


#include "gCanvas.h"


gCanvas::gCanvas(gApp* root) : gBaseCanvas(root) {
	this->root = root;
}

gCanvas::~gCanvas() {
}

void gCanvas::setup() {
	//	logi("setup");
    enableAlphaBlending();

    box.scale(0.4f);
	box.setPosition(0.0f, 0.0f, -2.0f);
	gMaterial redplastic;
	redplastic.setAmbientColor(1.0f, 0.0f, 0.0f);
	redplastic.setDiffuseColor(0.5f, 0.0f, 0.0f);
	redplastic.setSpecularColor(0.7f, 0.6f, 0.6f);
	redplastic.setShininess(1.0f);
	box.setMaterial(&redplastic);
	boxdiffusemap.load(gGetImagesDir() + "container2.png");
	box.getMaterial()->setDiffuseMap(&boxdiffusemap);
	boxspecularmap.load(gGetImagesDir() + "container2_specular.jpg");
	box.getMaterial()->setSpecularMap(&boxspecularmap);
//	boxnormalmap.load(gGetImagesDir() + "container2_normal.jpg");
//	box.getMaterial()->setNormalMap(&boxnormalmap);
	box.getMaterial()->setDiffuseMapEnabled(true);
//	boxface.load(gGetImagesDir() + "container2.png");
//	box.addTexture(boxface);

	lightbox.scale(0.01f);
	lightbox.setPosition(0.2f, 0.2f, -1.0f);

	light = gLight(gLight::LIGHTTYPE_SPOT);
	light.setPosition(lightbox.getPosition());
	light.setAmbientColor(128, 128, 128);
	logi("light type:" + gToStr(light.getType()));

	font.load(gGetFontsDir() + "FreeSans.ttf", 12);

#ifdef _WIN64
   //define something for Windows (64-bit)
#elif _WIN32
   //define something for Windows (32-bit)
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
        // define something for simulator
    #elif TARGET_OS_IPHONE
        // define something for iphone
    #else
        #define TARGET_OS_OSX 1
        // define something for OSX
    #endif
#elif __ANDROID__
    // Android
#elif __linux
    // Linux
#elif __unix // all unices not caught above
    // Unix
#elif __posix
    // POSIX
#endif

}

void gCanvas::update() {
	//	logi("giLightSample update");
}

void gCanvas::draw() {
//	logi("giLightSample draw");

	enableDepthTest();
	camera.begin();

	lightbox.rotateAround(0.01f, glm::vec3(0.0f, 1.0f, 0.0f), box.getPosition());
	light.setPosition(lightbox.getPosition());
	light.enable();
	box.rotate(0.005f, 0.75f, -1.0f, 0.0f);
	box.draw();
	light.disable();

	lightbox.draw();

	camera.end();
	disableDepthTest();

	font.drawText("FPS: " + gToStr(root->getFramerate()), 10, 22);
}

void gCanvas::keyPressed(int key) {
//	gLogi("gCanvas") << "keyPressed:" << key;
}

void gCanvas::keyReleased(int key) {
//	gLogi("gCanvas") << "keyReleased:" << key;
}

void gCanvas::charPressed(unsigned int codepoint) {
//	gLogi("gCanvas") << "charPressed:" << gCodepointToStr(codepoint);
}

void gCanvas::mouseMoved(int x, int y) {
//	gLogi("gCanvas") << "mouseMoved" << ", x:" << x << ", y:" << y;
}

void gCanvas::mouseDragged(int x, int y, int button) {
//	gLogi("gCanvas") << "mouseDragged" << ", x:" << x << ", y:" << y << ", b:" << button;
}

void gCanvas::mousePressed(int x, int y, int button) {
//	gLogi("gCanvas") << "mousePressed" << ", x:" << x << ", y:" << y << ", b:" << button;
}

void gCanvas::mouseReleased(int x, int y, int button) {
//	gLogi("gCanvas") << "mouseReleased" << ", button:" << button;
}

void gCanvas::mouseScrolled(int x, int y) {
//	gLogi("gCanvas") << "mouseScrolled" << ", x:" << x << ", y:" << y;
}

void gCanvas::mouseEntered() {

}

void gCanvas::mouseExited() {

}

void gCanvas::windowResized(int w, int h) {

}

void gCanvas::showNotify() {

}

void gCanvas::hideNotify() {

}

