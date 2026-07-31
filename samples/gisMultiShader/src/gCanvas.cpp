/*
* gCanvas.cpp
*
*  Created on: July 30, 2026
*      Author: Hasan Cem Eren
*/

#include "gCanvas.h"

gCanvas::gCanvas(gApp* root) : gBaseCanvas(root) {
	this->root = root;
	isboxshaderactive = true;
}

gCanvas::~gCanvas() {
}

void gCanvas::setup() {
	font.loadFont("FreeSans.ttf", 24);
	camera.setPosition(0.0f, 5.0f, 12.0f);
	camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));
	box.setPosition(3.0f, 0.5f, 0.0f);
	glowtime = 0.0f;

	// Load shaders
	outlineshader.load(gGetShadersDir() + "outline.vert", gGetShadersDir() + "outline.frag");
	glowshader.load(gGetShadersDir() + "glow.vert", gGetShadersDir() + "glow.frag");
	// Binds an extra shader pass to the box material for multi-pass rendering.
	boxmaterial.addShader(&outlineshader);
	boxmaterial.addShader(&glowshader);
	box.setMaterial(&boxmaterial);

	// 2. Car Model Setup
	carmodel.loadModel("Mercedes-Benz_SLS_AMG/Mercedes_AMG_GT3Clean.obj");
	carmodel.setPosition(-3.0f, 0.0f, 0.0f);

	// Accesses specific car body mesh material to attach extra pass.
	carmodel.getMesh(4).getMaterial()->addShader(&outlineshader);
	carmodel.getMesh(4).getMaterial()->addShader(&glowshader);
}

void gCanvas::update() {
	glowtime += 0.02f;
}

void gCanvas::draw() {
	enableDepthTest();
	enableAlphaTest();
	enableAlphaBlending();

	camera.begin();

	// Send the uniform variable to the shader which need it
	glowshader.use();
	glowshader.setFloat("utime", glowtime);

	// Renders the box mesh and executes attached extra shader passes.
	box.draw();

	// Renders the car model meshes and automatically triggers gMesh::drawExtraShaders().
	carmodel.draw();

	camera.end();

	disableAlphaBlending();
	disableAlphaTest();
	disableDepthTest();
	// 2D DRAW
	std::string statustext = isboxshaderactive ? "Press SPACE to deactivate one of shaders" : "Press SPACE to activate shaders";
	font.drawText(statustext, getWidth() / 2 - font.getStringWidth(statustext) / 2, font.getStringHeight(statustext) * 5);
}

void gCanvas::keyPressed(int key) {
	if(key == G_KEY_SPACE) {
		isboxshaderactive = !isboxshaderactive;
		// gMesh::setMaterial() copies the material by value, so we must modify the mesh's internal copy directly.
		gMaterial* boxmat = box.getMaterial();
		gMaterial* carmat = carmodel.getMesh(4).getMaterial();

		if(isboxshaderactive) {
			// Adding shader to the shaders vector
			boxmat->addShader(&outlineshader);
			boxmat->addShader(&glowshader);
			gLogi("gCanvas") << "Box Extra Shaders Added! Active count: " << boxmat->getShaders().size();
			carmat->addShader(&outlineshader);
		} else {
			// Removing shader from the shaders vector
			boxmat->removeShader(&glowshader);
			gLogi("gCanvas") << "Box Extra Shaders Removed via removeShader()! Active count: " << boxmat->getShaders().size();
			carmat->removeShader(&outlineshader);
		  }
	}
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
