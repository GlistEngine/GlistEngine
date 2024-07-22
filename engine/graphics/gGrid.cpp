/*
 * gGrid.cpp
 *
 *  Created on: Jul 11, 2024
 *      Author: usif
 */

#include "gGrid.h"

gGrid::gGrid() {

	gridvertices =
	{
		// positions
		 1.0f,  1.0f,  0.0f,
		-1.0f, -1.0f,  0.0f,
		-1.0f,  1.0f,  0.0f,

		-1.0f, -1.0f,  0.0f,
		 1.0f,  1.0f,  0.0f,
		 1.0f, -1.0f,  0.0f,
	};

	gridshader = nullptr;
	camera = nullptr;
	spacing = 10.0f;
	farclip = 1000.0f;
	nearclip = 0.01f;
	isautoclipenabled = false;

	isenabled = false;
	isgridxyenabled = true;
	isgridyzenabled = true;
	isgridxzenabled = true;
	isaxisxenabled = true;
	isaxisyenabled = true;
	isaxiszenabled = true;
	iswireframexyenabled = true;
	iswireframeyzenabled = true;
	iswireframexzenabled = true;

	coloraxisx.set(  0,200,  0,175); //0, 200, 0, 175
	coloraxisy.set(100,100,200,175); //100, 100, 200, 175
	coloraxisz.set(200,  0,  0,175);
	colorwireframexy.set(150, 30, 30,100); //150, 30, 30, 100
	colorwireframeyz.set(150, 30, 30,100);
	colorwireframexz.set( 30,150, 30,100);
}

gGrid::~gGrid() {
}

void gGrid::clear() {
	vbo.clear();
}

void gGrid::drawStart() {
	if(!vbo.isVertexDataAllocated()){
		vbo.setVertexData(gridvertices.data(), 3, gridvertices.size() /3, GL_STATIC_DRAW);
	}
	//grid options not implemented in the shader yet
	//main settings
	//	near and far clip setting
	if(isautoclipenabled && renderer->getCamera() != nullptr){
		camera = renderer->getCamera();
		nearclip = camera->getNearClip();
		farclip = camera->getFarClip();
	}
	// spacing setting
	// shader->setFloat("scale",something) not implemented yet

	//xy grid
	if(isgridxyenabled)
	{
		vbo.bind();
		gridshader = renderer->getGridShader();
		gridshader->use();

		if(!isaxisxenabled){
		}
		if(!isaxisyenabled){
		}
		if(!iswireframexyenabled){
		}

		vbo.unbind();
	}
	//yz grid
	if(isgridyzenabled)
	{
		vbo.bind();
		gridshader = renderer->getGridShader();
		gridshader->use();

		if(!isaxisyenabled){
		}
		if(!isaxiszenabled){
		}
		if(!iswireframeyzenabled){
		}

		vbo.unbind();
	}
	//xz grid
	if(isgridxzenabled)
	{
		vbo.bind();
		gridshader = renderer->getGridShader();
		gridshader->use();

		if(!isaxisxenabled){
		}
		if(!isaxiszenabled){
		}
		if(!iswireframexzenabled){
		}

		gridshader->setMat4("view",renderer->getViewMatrix());
		gridshader->setMat4("projection",renderer->getProjectionMatrix());
		gridshader->setFloat("near",nearclip);
		gridshader->setFloat("far",farclip);
		vbo.unbind();
	}
}

void gGrid::drawVbo() {
	if(isgridxyenabled || isgridyzenabled || isgridxzenabled){
		vbo.bind();
		G_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, vbo.getVerticesNum()));
		vbo.unbind();
	}
}

void gGrid::drawEnd() {
	// set everything back to defaults if necessary
}

void gGrid::draw() {
	if(!isenabled) return;
	renderer->enableAlphaBlending();
	renderer->enableDepthTest();
	drawStart();
	drawVbo();
	drawEnd();
	renderer->disableDepthTest();
	renderer->disableAlphaBlending();
}

void gGrid::drawYZ() {
	bool oldgridstates[3], oldenablestate;
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	oldenablestate = isenabled;
	enable();
	setEnableGrid(false, true, false);
	draw();
	disable();
	isenabled =	oldenablestate;
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::drawXY() {
	bool oldgridstates[3], oldenablestate;
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	oldenablestate = isenabled;
	enable();
	setEnableGrid(true, false, false);
	draw();
	disable();
	isenabled =	oldenablestate;
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::drawXZ() {
	bool oldgridstates[3], oldenablestate;
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	oldenablestate = isenabled;
	enable();
	setEnableGrid(false, false, true);
	draw();
	disable();
	isenabled =	oldenablestate;
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::enable() {
	isenabled = true;
}

void gGrid::disable() {
	isenabled = false;
}

bool gGrid::isEnabled() const {
	return isenabled;
}

void gGrid::setEnableGrid(bool xy, bool yz, bool xz) {
	isgridxyenabled = xy;
	isgridyzenabled = yz;
	isgridxzenabled = xz;
}
void gGrid::setEnableYZ(bool yz) {
	isgridyzenabled = yz;
}
void gGrid::setEnableXY(bool xy) {
	isgridxyenabled = xy;
}
void gGrid::setEnableXZ(bool xz) {
	isgridxzenabled = xz;
}

bool gGrid::isYZEnabled() const {
	return isgridyzenabled;
}
bool gGrid::isXYEnabled() const {
	return isgridxyenabled;
}
bool gGrid::isXZEnabled() const {
	return isgridxzenabled;
}

void gGrid::drawAxisY() {
	bool oldaxisstate, oldwireframestate;
	oldaxisstate = isaxisyenabled;
	oldwireframestate = iswireframeyzenabled;
	setEnableAxisY(true);
	setEnableWireFrameYZ(false);
	drawYZ();
	setEnableWireFrameYZ(oldwireframestate);
	setEnableAxisY(oldaxisstate);
}

void gGrid::drawAxisX() {
	bool oldaxisstate, oldwireframestate;
	oldaxisstate = isaxisxenabled;
	oldwireframestate = iswireframexyenabled;
	setEnableAxisX(true);
	setEnableWireFrameXY(false);
	drawXY();
	setEnableWireFrameXY(oldwireframestate);
	setEnableAxisX(oldaxisstate);
}

void gGrid::drawAxisZ() {
	bool oldaxisstate, oldwireframestate;
	oldaxisstate = isaxiszenabled;
	oldwireframestate = iswireframexzenabled;
	setEnableAxisZ(true);
	setEnableWireFrameXZ(false);
	drawXZ();
	setEnableWireFrameXZ(oldwireframestate);
	setEnableAxisZ(oldaxisstate);
}

void gGrid::setEnableAxis(bool x, bool y, bool z) {
	isaxisxenabled = x;
	isaxisyenabled = y;
	isaxiszenabled = z;
}

void gGrid::setEnableAxisY(bool y) {
	isaxisyenabled = y;
}

void gGrid::setEnableAxisX(bool x) {
	isaxisxenabled = x;
}

void gGrid::setEnableAxisZ(bool z) {
	isaxiszenabled = z;
}

bool gGrid::isAxisYEnabled() const {
	return isaxisyenabled;
}

bool gGrid::isAxisXEnabled() const {
	return isaxisxenabled;
}

bool gGrid::isAxisZEnabled() const {
	return isaxiszenabled;
}

void gGrid::drawWireFrameYZ() {
	bool oldaxisstates[2], oldwireframestate;
	oldaxisstates[0] = isaxisyenabled;
	oldaxisstates[1] = isaxiszenabled;
	oldwireframestate = iswireframeyzenabled;
	setEnableAxisY(false);
	setEnableAxisZ(false);
	setEnableWireFrameYZ(true);
	drawYZ();
	setEnableWireFrameYZ(oldwireframestate);
	setEnableAxisY(oldaxisstates[0]);
	setEnableAxisZ(oldaxisstates[1]);
}

void gGrid::drawWireFrameXY() {
	bool oldaxisstates[2], oldwireframestate;
	oldaxisstates[0] = isaxisxenabled;
	oldaxisstates[1] = isaxisyenabled;
	oldwireframestate = iswireframexyenabled;
	setEnableAxisX(false);
	setEnableAxisY(false);
	setEnableWireFrameXY(true);
	drawXY();
	setEnableWireFrameXY(oldwireframestate);
	setEnableAxisX(oldaxisstates[0]);
	setEnableAxisY(oldaxisstates[1]);
}

void gGrid::drawWireFrameXZ() {
	bool oldaxisstates[2], oldwireframestate;
	oldaxisstates[0] = isaxisxenabled;
	oldaxisstates[1] = isaxiszenabled;
	oldwireframestate = iswireframexzenabled;
	setEnableAxisX(false);
	setEnableAxisZ(false);
	setEnableWireFrameXZ(true);
	drawXZ();
	setEnableWireFrameXZ(oldwireframestate);
	setEnableAxisX(oldaxisstates[0]);
	setEnableAxisZ(oldaxisstates[1]);
}

void gGrid::setEnableWireFrame(bool xy, bool yz, bool xz) {
	iswireframexyenabled = xy;
	iswireframeyzenabled = yz;
	iswireframexzenabled = xz;
}

void gGrid::setEnableWireFrameYZ(bool yz) {
	iswireframeyzenabled = yz;
}

void gGrid::setEnableWireFrameXY(bool xy) {
	iswireframexyenabled = xy;
}

void gGrid::setEnableWireFrameXZ(bool xz) {
	iswireframexzenabled = xz;
}

bool gGrid::isWireFrameYZEnabled() const {
	return iswireframeyzenabled;
}

bool gGrid::isWireFrameXYEnabled() const {
	return iswireframexyenabled;
}

bool gGrid::isWireFrameXZEnabled() const {
	return iswireframexzenabled;
}

void gGrid::setColorAxisY(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	coloraxisy.r = r;
	coloraxisy.g = g;
	coloraxisy.b = b;
	coloraxisy.a = a;
}
void gGrid::setColorAxisY(gColor *color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	coloraxisy.r = color->r;
	coloraxisy.g = color->g;
	coloraxisy.b = color->b;
	coloraxisy.a = color->a;
}

void gGrid::setColorAxisX(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	coloraxisx.r = r;
	coloraxisx.g = g;
	coloraxisx.b = b;
	coloraxisx.a = a;
}
void gGrid::setColorAxisX(gColor *color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	coloraxisx.r = color->r;
	coloraxisx.g = color->g;
	coloraxisx.b = color->b;
	coloraxisx.a = color->a;
}

void gGrid::setColorAxisZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	coloraxisz.r = r;
	coloraxisz.g = g;
	coloraxisz.b = b;
	coloraxisz.a = a;
}
void gGrid::setColorAxisZ(gColor *color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	coloraxisz.r = color->r;
	coloraxisz.g = color->g;
	coloraxisz.b = color->b;
	coloraxisz.a = color->a;
}

void gGrid::setColorWireFrameYZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	colorwireframeyz.r = r;
	colorwireframeyz.g = g;
	colorwireframeyz.b = b;
	colorwireframeyz.a = a;
}
void gGrid::setColorWireFrameYZ(gColor *color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	colorwireframeyz.r = color->r;
	colorwireframeyz.g = color->g;
	colorwireframeyz.b = color->b;
	colorwireframeyz.a = color->a;

}

void gGrid::setColorWireFrameXY(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	colorwireframexy.r = r;
	colorwireframexy.g = g;
	colorwireframexy.b = b;
	colorwireframexy.a = a;
}
void gGrid::setColorWireFrameXY(gColor *color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	colorwireframexy.r = color->r;
	colorwireframexy.g = color->g;
	colorwireframexy.b = color->b;
	colorwireframexy.a = color->a;
}

void gGrid::setColorWireFrameXZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	colorwireframexz.r = r;
	colorwireframexz.g = g;
	colorwireframexz.b = b;
	colorwireframexz.a = a;
}
void gGrid::setColorWireFrameXZ(gColor *color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	colorwireframexz.r = color->r;
	colorwireframexz.g = color->g;
	colorwireframexz.b = color->b;
	colorwireframexz.a = color->a;
}

void gGrid::enableAutoClip() {
	isautoclipenabled = true;
}

void gGrid::disableAutoClip() {
	isautoclipenabled = false;
}

bool gGrid::isAutoClipEnabled() const {
	return isautoclipenabled;
}

void gGrid::setNearClip(float nearClip) {
	nearclip = glm::clamp(nearClip, 0.001f, 0.999f);
}

void gGrid::setFarClip(float farClip) {
	farclip = glm::clamp(farClip, 1.0f, 10000.0f);
}

float gGrid::getNearClip() const {
	return nearclip;
}

float gGrid::getFarClip() const {
	return farclip;
}

void gGrid::setLineSpacing(float spacing) {
	this->spacing = spacing;
}

float gGrid::getLineSpacing() {
	return spacing;
}
