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
	isaxisxyenabled = true;
	isaxisyzenabled = true;
	isaxisxzenabled = true;
	iswireframexyenabled = true;
	iswireframeyzenabled = true;
	iswireframexzenabled = true;

	coloraxisxy.set(  0,200,  0,175); //0, 200, 0, 175
	coloraxisyz.set(100,100,200,175); //100, 100, 200, 175
	coloraxisxz.set(200,  0,  0,175);
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
	if(isautoclipenabled){
		camera = renderer->getCamera();
		nearclip = camera->getNearClip();
		farclip = camera->getFarClip();
	}
	// spacing setting

	//xy grid settings
	if(isgridxyenabled)
	{
		if(!isaxisxyenabled){
		}
		if(!iswireframexyenabled){
		}
	}
	//yz grid settings
	if(isgridyzenabled)
	{
		if(!isaxisyzenabled){
		}
		if(!iswireframeyzenabled){
		}
	}
	//xz grid settings
	if(isgridxzenabled)
	{
		if(!isaxisxzenabled){
		}
		if(!iswireframexzenabled){
		}
	}

	vbo.bind();
	gridshader = renderer->getGridShader();
	gridshader->use();
	gridshader->setMat4("view",renderer->getViewMatrix());
	gridshader->setMat4("projection",renderer->getProjectionMatrix());
	gridshader->setFloat("near",nearclip);
	gridshader->setFloat("far",farclip);
	vbo.unbind();
}

void gGrid::drawVbo() {
    vbo.bind();
	G_CHECK_GL(glDrawArrays(GL_TRIANGLES, 0, vbo.getVerticesNum()));
    vbo.unbind();
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
	bool oldgridstates[3];
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	setEnableGrid(false, true, false);
	draw();
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::drawXY() {
	bool oldgridstates[3];
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	setEnableGrid(true, false, false);
	draw();
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::drawXZ() {
	bool oldgridstates[3];
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	setEnableGrid(false, false, true);
	draw();
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

void gGrid::drawAxisYZ() {
	bool oldgridstates[3], oldaxisstate, oldwireframestate;
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	setEnableGrid(false, true, false);
	oldaxisstate = isaxisyzenabled;
	oldwireframestate = iswireframeyzenabled;
	setEnableAxisYZ(true);
	setEnableWireFrameYZ(false);
	draw();
	setEnableWireFrameYZ(oldwireframestate);
	setEnableAxisYZ(oldaxisstate);
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::drawAxisXY() {
	bool oldgridstates[3], oldaxisstate, oldwireframestate;
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	setEnableGrid(true, false, false);
	oldaxisstate = isaxisxyenabled;
	oldwireframestate = iswireframexyenabled;
	setEnableAxisXY(true);
	setEnableWireFrameXY(false);
	draw();
	setEnableWireFrameXY(oldwireframestate);
	setEnableAxisXY(oldaxisstate);
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::drawAxisXZ() {
	bool oldgridstates[3], oldaxisstate, oldwireframestate;
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	setEnableGrid(false, false, true);
	oldaxisstate = isaxisxzenabled;
	oldwireframestate = iswireframexzenabled;
	setEnableAxisXZ(true);
	setEnableWireFrameXZ(false);
	draw();
	setEnableWireFrameXZ(oldwireframestate);
	setEnableAxisXZ(oldaxisstate);
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::setEnableAxis(bool xy, bool yz, bool xz) {
	isaxisxyenabled = xy;
	isaxisyzenabled = yz;
	isaxisxzenabled = xz;
}

void gGrid::setEnableAxisYZ(bool yz) {
	isaxisyzenabled = yz;
}

void gGrid::setEnableAxisXY(bool xy) {
	isaxisxyenabled = xy;
}

void gGrid::setEnableAxisXZ(bool xz) {
	isaxisxzenabled = xz;
}

bool gGrid::isAxisYZEnabled() const {
	return isaxisyzenabled;
}

bool gGrid::isAxisXYEnabled() const {
	return isaxisxyenabled;
}

bool gGrid::isAxisXZEnabled() const {
	return isaxisxzenabled;
}

void gGrid::drawWireFrameYZ() {
	bool oldgridstates[3], oldaxisstate, oldwireframestate;
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	setEnableGrid(false, true, false);
	oldaxisstate = isaxisyzenabled;
	oldwireframestate = iswireframeyzenabled;
	setEnableAxisYZ(false);
	setEnableWireFrameYZ(true);
	draw();
	setEnableWireFrameYZ(oldwireframestate);
	setEnableAxisYZ(oldaxisstate);
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::drawWireFrameXY() {
	bool oldgridstates[3], oldaxisstate, oldwireframestate;
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	setEnableGrid(true, false, false);
	oldaxisstate = isaxisxyenabled;
	oldwireframestate = iswireframexyenabled;
	setEnableAxisXY(false);
	setEnableWireFrameXY(true);
	draw();
	setEnableWireFrameXY(oldwireframestate);
	setEnableAxisXY(oldaxisstate);
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
}

void gGrid::drawWireFrameXZ() {
	bool oldgridstates[3], oldaxisstate, oldwireframestate;
	oldgridstates[XY] = isgridxyenabled;
	oldgridstates[YZ] = isgridyzenabled;
	oldgridstates[XZ] = isgridxzenabled;
	setEnableGrid(false, false, true);
	oldaxisstate = isaxisxzenabled;
	oldwireframestate = iswireframexzenabled;
	setEnableAxisXZ(false);
	setEnableWireFrameXZ(true);
	draw();
	setEnableWireFrameXZ(oldwireframestate);
	setEnableAxisXZ(oldaxisstate);
	setEnableGrid(oldgridstates[XY], oldgridstates[YZ], oldgridstates[XZ]);
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

void gGrid::setColorAxisYZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	coloraxisyz.r = r;
	coloraxisyz.g = g;
	coloraxisyz.b = b;
	coloraxisyz.a = a;
}
void gGrid::setColorAxisYZ(gColor *color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	coloraxisyz.r = color->r;
	coloraxisyz.g = color->g;
	coloraxisyz.b = color->b;
	coloraxisyz.a = color->a;
}

void gGrid::setColorAxisXY(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	coloraxisxy.r = r;
	coloraxisxy.g = g;
	coloraxisxy.b = b;
	coloraxisxy.a = a;
}
void gGrid::setColorAxisXY(gColor *color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	coloraxisxy.r = color->r;
	coloraxisxy.g = color->g;
	coloraxisxy.b = color->b;
	coloraxisxy.a = color->a;
}

void gGrid::setColorAxisXZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	coloraxisxz.r = r;
	coloraxisxz.g = g;
	coloraxisxz.b = b;
	coloraxisxz.a = a;
}
void gGrid::setColorAxisXZ(gColor *color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	coloraxisxz.r = color->r;
	coloraxisxz.g = color->g;
	coloraxisxz.b = color->b;
	coloraxisxz.a = color->a;
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

void gGrid::setNearClip(float near) {
	nearclip = near;
}

void gGrid::setFarClip(float far) {
	farclip = far;
}

void gGrid::setLineSpacing(float spacing) {
	this->spacing = spacing;
}

float gGrid::getLineSpacing() {
	return spacing;
}

