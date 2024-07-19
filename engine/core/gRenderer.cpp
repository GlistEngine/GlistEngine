/*
 * gRenderManager.cpp
 *
 *  Created on: 4 Ara 2020
 *      Author: Acer
 */

#include "gRenderer.h"

#include "gCross.h"
#include "gArc.h"
#include "gLight.h"
#include "gLine.h"
#include "gCircle.h"
#include "gRectangle.h"
#include "gBox.h"
#include "gSphere.h"
#include "gTriangle.h"
#include "gRoundedRectangle.h"
#include "gCylinder.h"
#include "gCone.h"
#include "gTube.h"
#include "gUbo.h"
#include "gShader.h"
#include "gCamera.h"

//screenShot Related includes
#include "stb/stb_image_write.h"
#include "gBaseApp.h"
#include "gImage.h"

const int gRenderer::SCREENSCALING_NONE = 0;
const int gRenderer::SCREENSCALING_MIPMAP = 1;
const int gRenderer::SCREENSCALING_AUTO = 2;

const int gRenderer::DEPTHTESTTYPE_LESS = 0;
const int gRenderer::DEPTHTESTTYPE_ALWAYS = 1;

const int gRenderer::FOGMODE_LINEAR = 0;
const int gRenderer::FOGMODE_EXP = 1;

int gRenderer::width;
int gRenderer::height;
int gRenderer::unitwidth;
int gRenderer::unitheight;
int gRenderer::screenscaling;
int gRenderer::currentresolution;
int gRenderer::unitresolution;

void gCheckGLErrorAndPrint(const std::string& prefix, const std::string& func, int line) {
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		gLogi("gRenderer") << prefix << "OpenGL ERROR at " << func << ", line " << line << ", error: " << gToHex(error, 4);
	}
}

void gEnableCulling() {
	glEnable(GL_CULL_FACE);
}

void gDisableCulling() {
	glDisable(GL_CULL_FACE);
}

bool gIsCullingEnabled() {
	return glIsEnabled(GL_CULL_FACE);
}

void gCullFace(int cullingFace) {
	glCullFace(cullingFace);
}

int gGetCullFace() {
	GLint i;
	glGetIntegerv(GL_CULL_FACE_MODE, &i);
	return i;
}

void gSetCullingDirection(int cullingDirection) {
	glFrontFace(cullingDirection);
}

int gGetCullingDirection() {
	GLint i;
	glGetIntegerv(GL_FRONT_FACE, &i);
	return i;
}

/*
 * enable to show grid
 */
void gRenderer::enableGrid() {
	isgridenable = true;
}
/*
 * set which Grid axis will be showned by set xy, xz, yz
 * xy => xy axis
 * xz => xz axis
 * yz => yz axis
 */
void gRenderer::setGridEnableAxis(bool xy, bool yz, bool xz) {
	isgridxzenable = xz;
	isgridxyenable = xy;
	isgridyzenable = yz;
}

/*
 * set Grid XY axis enable or not with xy boolean
 */
void gRenderer::setGridEnableXY(bool xy) {
	isgridxyenable = xy;
}

/*
 * set Grid XZ axis enable or not with xy boolean
 */
void gRenderer::setGridEnableXZ(bool xz) {
	isgridxzenable = xz;
}

/*
 * set Grid YZ axis enable or not with yz boolean
 */
void gRenderer::setGridEnableYZ(bool yz) {
	isgridyzenable = yz;
}

/*
 * set max coordinate to reach for grid. Example: 50 mean (-25 to 25) as coordinate axis
 * @param gridmaxvalue => set max distance for grid.
 */
void gRenderer::setGridMaxLength(float length) {
	gridmaxvalue = length;
}

/*
 * return max length of grid as float
 */
float gRenderer::getGridMaxLength() {
	return gridmaxvalue;
}

/*
 * set distance between grid lines.
 * @param gridmaxvalue => set max distance for grid.
 */
void gRenderer::setGridLineInterval(float intervalvalue) {
	gridlineinterval = intervalvalue;
}

/*
 * return distance between grid lines as float
 */
float gRenderer::getGridLineInterval() {
	return gridlineinterval;
}

/*
 * disable grid
 */
void gRenderer::disableGrid() {
	isgridenable = false;
}

//return if Grid Draw or not
bool gRenderer::isGridEnabled() {
	return isgridenable;
}

//return if GridXY axis
bool gRenderer::isGridXYEnabled() {
	return isgridxyenable;
}

//return if GridXZ axis
bool gRenderer::isGridXZEnabled() {
	return isgridxzenable;
}

//return if GridYZ axis
bool gRenderer::isGridYZEnabled() {
	return isgridyzenable;
}
/*
 * draw grid lines if each axis valuables are enable(true)
 * @param isgridenable for showing grid lines
 * @param isgridxzenable, isgridxyenable, isgridyzenable for which axis of grid lines will be draw
 */
void gRenderer::drawGrid() {
	if(!isgridenable) return;
	enableDepthTest();
	if(isgridxzenable)drawGridXZ();
	if(isgridxyenable)drawGridXY();
	if(isgridyzenable)drawGridYZ();
	disableDepthTest();
}

/*
 * drawing Grid XZ axis
 * @row - which coordinate for line to draw
 * @gridmaxvalue => how many lines will draw (can count as max grid lenght)
 * @gridlineinterval => distance between lines
 */
void gRenderer::drawGridXZ() {
	//color saved temp
	gColor oldcolor;
	oldcolor.set(rendercolor->r, rendercolor->g, rendercolor->b, rendercolor->a);
	//grid
	for (float row = cameraposition.z + gridmaxvalue / 2; row > cameraposition.z - gridmaxvalue / 2; row -= gridlineinterval) {
		//row(z)
		//line color
		if(((int)row % 10) == 0)rendercolor->set(gridxzcolor.r, gridxzcolor.g, gridxzcolor.b, gridxzcolor.a);else rendercolor->set(gridxzmargincolor.r, gridxzmargincolor.g, gridxzmargincolor.b, gridxzmargincolor.a);
		gDrawLine(cameraposition.x - gridmaxvalue / 2, 0.0f, row, cameraposition.x + gridmaxvalue / 2, 0.0f, row);
	}
	//grid
	for (float column = cameraposition.x - gridmaxvalue / 2; column <= cameraposition.x + gridmaxvalue / 2; column += gridlineinterval) {
		//row(z)
		//line color
		if(((int)column % 10) == 0)rendercolor->set(gridxzcolor.r, gridxzcolor.g, gridxzcolor.b, gridxzcolor.a);else rendercolor->set(gridxzmargincolor.r, gridxzmargincolor.g, gridxzmargincolor.b, gridxzmargincolor.a);
		gDrawLine(column, 0.0f, cameraposition.z - gridmaxvalue / 2, column, 0.0f, cameraposition.z + gridmaxvalue / 2);
	}
	//line color reset
	rendercolor->set(oldcolor.r, oldcolor.g, oldcolor.b, oldcolor.a);
}

void gRenderer::drawGridYZ() {
	//color saved temp
	gColor oldcolor;
	oldcolor.set(rendercolor->r, rendercolor->g, rendercolor->b, rendercolor->a);
	//grid
	for (float row = cameraposition.z + gridmaxvalue / 2; row > cameraposition.z - gridmaxvalue / 2; row -= gridlineinterval) {
		//row(z)
		//line color
		if(((int)row % 10) == 0)rendercolor->set(gridyzcolor.r, gridyzcolor.g, gridyzcolor.b, gridyzcolor.a);else rendercolor->set(gridyzmargincolor.r, gridyzmargincolor.g, gridyzmargincolor.b, gridyzmargincolor.a);
		gDrawLine(0.0f, cameraposition.y - gridmaxvalue / 2, row, 0.0f, cameraposition.y + gridmaxvalue / 2, row);
	}
	//grid
	for (float column = cameraposition.y - gridmaxvalue / 2; column <= cameraposition.y + gridmaxvalue / 2; column += gridlineinterval) {
		//row(z)
		//line color
		if(((int)column % 10) == 0)rendercolor->set(gridyzcolor.r, gridyzcolor.g, gridyzcolor.b, gridyzcolor.a);else rendercolor->set(gridyzmargincolor.r, gridyzmargincolor.g, gridyzmargincolor.b, gridyzmargincolor.a);
		gDrawLine(0.0f, column, cameraposition.z - gridmaxvalue / 2, 0.0f, column, cameraposition.z + gridmaxvalue / 2);
	}
	//line color reset
	rendercolor->set(oldcolor.r, oldcolor.g, oldcolor.b, oldcolor.a);
}
/*
 * drawing Grid XY axis
 * @row - which coordinate for line to draw
 * @gridmaxvalue => how many lines will draw (can count as max grid lenght)
 * @gridlineinterval => distance between lines
 */
void gRenderer::drawGridXY() {
	//color saved temp
	gColor oldcolor;
	oldcolor.set(rendercolor->r, rendercolor->g, rendercolor->b, rendercolor->a);
	if(!isgridenable) return;
	//grid
	for (float row = cameraposition.x - gridmaxvalue / 2; row <= cameraposition.x + gridmaxvalue / 2; row += gridlineinterval) {
		//row(z)
		//line color
		if(((int)row % 10) == 0)rendercolor->set(gridxycolor.r, gridxycolor.g, gridxycolor.b, gridxycolor.a);else rendercolor->set(gridxymargincolor.r, gridxymargincolor.g, gridxymargincolor.b, gridxymargincolor.a);
		gDrawLine(row, cameraposition.y - gridmaxvalue / 2, 0.0f, row, cameraposition.y + gridmaxvalue / 2, 0.0f);
	}
	//grid
	for (float column = cameraposition.y - gridmaxvalue / 2; column <= cameraposition.y + gridmaxvalue / 2; column += gridlineinterval) {
		//row(z)
		//line color
		if(((int)column % 10) == 0)rendercolor->set(gridxycolor.r, gridxycolor.g, gridxycolor.b, gridxycolor.a);else rendercolor->set(gridxymargincolor.r, gridxymargincolor.g, gridxymargincolor.b, gridxymargincolor.a);
		gDrawLine(cameraposition.x - gridmaxvalue / 2, column, 0.0f, cameraposition.x + gridmaxvalue / 2, column, 0.0f);
	}
	//line color reset
	rendercolor->set(oldcolor.r, oldcolor.g, oldcolor.b, oldcolor.a);
}


/*
 * set color for XZ axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gRenderer::setGridColorofAxisXZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	gridxzcolor.r = r;
	gridxzcolor.g = g;
	gridxzcolor.b = b;
	gridxzcolor.a = a;
}

/*
 * set color for XZ axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gRenderer::setGridColorofAxisXZ(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	gridxzcolor.r = color->r;
	gridxzcolor.g = color->g;
	gridxzcolor.b = color->b;
	gridxzcolor.a = color->a;
	//color->r = 100;
	//rendercolor->set(color);
}

/*
 * set color for margin of XZ axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gRenderer::setGridColorofAxisWireFrameXZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	gridxzmargincolor.r = r;
	gridxzmargincolor.g = g;
	gridxzmargincolor.b = b;
	gridxzmargincolor.a = a;
}

/*
 * set color for margin of XZ axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gRenderer::setGridColorofAxisWireFrameXZ(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	gridxzmargincolor.r = color->r;
	gridxzmargincolor.g = color->g;
	gridxzmargincolor.b = color->b;
	gridxzmargincolor.a = color->a;
	//rendercolor->set(color);
}


/*
 * set color for XY axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gRenderer::setGridColorofAxisXY(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	gridxycolor.r = r;
	gridxycolor.g = g;
	gridxycolor.b = b;
	gridxycolor.a = a;
}

/*
 * set color for XY axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gRenderer::setGridColorofAxisXY(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	gridxycolor.r = color->r;
	gridxycolor.g = color->g;
	gridxycolor.b = color->b;
	gridxycolor.a = color->a;
	//color->r = 100;
	//rendercolor->set(color);
}

/*
 * set color for margin of XY axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gRenderer::setGridColorofAxisWireFrameXY(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	gridxymargincolor.r = r;
	gridxymargincolor.g = g;
	gridxymargincolor.b = b;
	gridxymargincolor.a = a;
}

/*
 * set color for margin of XY axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gRenderer::setGridColorofAxisWireFrameXY(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	gridxymargincolor.r = color->r;
	gridxymargincolor.g = color->g;
	gridxymargincolor.b = color->b;
	gridxymargincolor.a = color->a;
	//rendercolor->set(color);
}

/*
 * set color for YZ axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gRenderer::setGridColorofAxisYZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	gridyzcolor.r = r;
	gridyzcolor.g = g;
	gridyzcolor.b = b;
	gridyzcolor.a = a;
}

/*
 * set color for YZ axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gRenderer::setGridColorofAxisYZ(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	gridyzcolor.r = color->r;
	gridyzcolor.g = color->g;
	gridyzcolor.b = color->b;
	gridyzcolor.a = color->a;
	//color->r = 100;
	//rendercolor->set(color);
}

/*
 * set color for margin of YZ axis of grid r:red, g:green, b:blue, a:transparency(0 => full transparancy)
 */
void gRenderer::setGridColorofAxisWireFrameYZ(int r, int g, int b, int a) {
	if(r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255 || a < 0 || a > 255)return;
	gridyzmargincolor.r = r;
	gridyzmargincolor.g = g;
	gridyzmargincolor.b = b;
	gridyzmargincolor.a = a;
}

/*
 * set color for margin of YZ axis of grid with gColor a:transparency(0 => full transparancy)
 * @param color => send direct color with gColor function
 */
void gRenderer::setGridColorofAxisWireFrameYZ(gColor* color) {
	if(color->r < 0 || color->r > 255 || color->g < 0 || color->g > 255 || color->b < 0 || color->b > 255 || color->a < 0 || color->a > 255)return;
	gridyzmargincolor.r = color->r;
	gridyzmargincolor.g = color->g;
	gridyzmargincolor.b = color->b;
	gridyzmargincolor.a = color->a;
	//rendercolor->set(color);
}

void gDrawLine(float x1, float y1, float x2, float y2, float thickness) {
	gLine linemesh;
	linemesh.setThickness(thickness);
	linemesh.draw(x1, y1, x2, y2);
	linemesh.clear();
}

void gDrawLine(float x1, float y1, float z1, float x2, float y2, float z2, float thickness) {
	gLine linemesh;
	linemesh.setThickness(thickness);
	linemesh.draw(x1, y1, z1, x2, y2, z2);
	linemesh.clear();
}

void gDrawTriangle(float px, float py, float qx, float qy, float rx, float ry, bool is_filled) {
	gTriangle trianglemesh;
	trianglemesh.draw(px, py, qx, qy, rx, ry, is_filled);
	trianglemesh.clear();
}

void gDrawCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides) {
	gCircle circlemesh;
	circlemesh.draw(xCenter, yCenter, radius, isFilled, numberOfSides);
	circlemesh.clear();
}

void gDrawCross(float x, float y, float width, float height, float thickness, bool isFilled) {
	gCross crossmesh;
	crossmesh.draw(x, y, width, height, thickness, isFilled);
	crossmesh.clear();
}

void gDrawArc(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides, float degree, float rotate) {
	gArc arcmesh;
	arcmesh.draw(xCenter, yCenter, radius, isFilled, numberOfSides, degree, rotate);
	arcmesh.clear();
}

void gDrawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle) {
	static gLine linemesh, linemesh2, linemesh3;
	float x2, y2;
	x2 = x1 + std::cos(gDegToRad(angle)) * length;
	y2 = y1 + std::sin(gDegToRad(angle)) * length;;
	linemesh.draw(x2, y2, x1, y1);
	linemesh2.draw(x1, y1, x1 + std::cos(gDegToRad(angle) - gDegToRad(tipAngle)) * tipLength, y1 + std::sin(gDegToRad(angle) - gDegToRad(tipAngle)) * tipLength);
	linemesh3.draw(x1, y1, x1 + (std::cos(gDegToRad(angle) + gDegToRad(tipAngle)) * tipLength) , y1 + std::sin(gDegToRad(angle) + gDegToRad(tipAngle)) * tipLength);
	linemesh.clear();
	linemesh2.clear();
	linemesh3.clear();
}

void gDrawRectangle(float x, float y, float w, float h, bool isFilled) {
	static gRectangle rectanglemesh;
 	rectanglemesh.draw(x, y, w, h, isFilled);
}

void gDrawRoundedRectangle(float x, float y, float w, float h, int radius, bool isFilled) {
	gRoundedRectangle roundedrectanglemesh;
	roundedrectanglemesh.draw(x, y, w, h, radius, isFilled);
	roundedrectanglemesh.clear();
}

void gDrawBox(float x, float y, float z, float w, float h, float d, bool isFilled) {
	gBox boxmesh;
	if(!isFilled) boxmesh.setDrawMode(gMesh::DRAWMODE_LINELOOP);
	boxmesh.setPosition(x, y, z);
	boxmesh.scale(w, h, d);
	boxmesh.draw();
	boxmesh.clear();
}

void gDrawBox(glm::mat4 transformationMatrix, bool isFilled) {
	gBox boxmesh;
	if(!isFilled) boxmesh.setDrawMode(gMesh::DRAWMODE_LINELOOP);
	boxmesh.setTransformationMatrix(transformationMatrix);
	boxmesh.draw();
	boxmesh.clear();
}

void gDrawSphere(float xPos, float yPos, float zPos, glm::vec3 scale, int xSegmentNum, int ySegmentNum, bool isFilled) {
	gSphere spheremesh(xSegmentNum, ySegmentNum, isFilled);
	spheremesh.setPosition(xPos, yPos, zPos);
	spheremesh.scale(scale.x, scale.y, scale.z);
	spheremesh.draw();
	spheremesh.clear();
}

void gDrawCylinder(float x, float y, float z, int r, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	gCylinder cylindermesh(r, r, h, glm::vec2(0.0f, 0.0f), segmentnum, isFilled);
	cylindermesh.setPosition(x, y, z);
	cylindermesh.scale(scale.x, scale.y, scale.z);
	cylindermesh.draw();
	cylindermesh.clear();
}

void gDrawCylinderOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	gCylinder cylindermesh(r, r, h, shiftdistance, segmentnum, isFilled);
	cylindermesh.setPosition(x, y, z);
	cylindermesh.scale(scale.x, scale.y, scale.z);
	cylindermesh.draw();
	cylindermesh.clear();
}

void gDrawCylinderTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	gCylinder cylindermesh(r1, r2, h, glm::vec2(0.0f, 0.0f), segmentnum, isFilled);
	cylindermesh.setPosition(x, y, z);
	cylindermesh.scale(scale.x, scale.y, scale.z);
	cylindermesh.draw();
	cylindermesh.clear();
}

void gDrawCylinderObliqueTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	gCylinder cylindermesh(r1, r2, h, shiftdistance, segmentnum, isFilled);
	cylindermesh.setPosition(x, y, z);
	cylindermesh.scale(scale.x, scale.y, scale.z);
	cylindermesh.draw();
	cylindermesh.clear();
}

void gDrawCone(float x, float y, float z, int r, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	gCone conemesh(r, h, glm::vec2(0.0f, 0.0f), segmentnum, isFilled);
	conemesh.setPosition(x, y, z);
	conemesh.scale(scale.x, scale.y, scale.z);
	conemesh.draw();
	conemesh.clear();
}

void gDrawConeOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	gCone conemesh(r, h, shiftdistance, segmentnum, isFilled);
	conemesh.setPosition(x, y, z);
	conemesh.scale(scale.x, scale.y, scale.z);
	conemesh.draw();
	conemesh.clear();
}

void gDrawPyramid(float x, float y, float z, int r, int h, glm::vec3 scale, int numberofsides, bool isFilled) {
	gCone conemesh(r, h, glm::vec2(0.0f, 0.0f), numberofsides, isFilled);
	conemesh.setPosition(x, y, z);
	conemesh.scale(scale.x, scale.y, scale.z);
	conemesh.draw();
	conemesh.clear();
}

void gDrawPyramidOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int numberofsides, bool isFilled) {
	gCone conemesh(r, h, shiftdistance, numberofsides, isFilled);
	conemesh.setPosition(x, y, z);
	conemesh.scale(scale.x, scale.y, scale.z);
	conemesh.draw();
	conemesh.clear();
}

void gDrawTube(float x, float y, float z, int outerradius, int innerradious, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	gTube tubemesh(outerradius,innerradious ,outerradius,innerradious, h, glm::vec2(0.0f, 0.0f), segmentnum, isFilled);
	tubemesh.setPosition(x, y, z);
	tubemesh.scale(scale.x, scale.y, scale.z);
	tubemesh.draw();
	tubemesh.clear();
}

void gDrawTubeOblique(float x, float y, float z, int outerradius,
		int innerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale,
		int segmentnum, bool isFilled) {
	gTube tubemesh(outerradius,innerradious ,outerradius,innerradious, h, shiftdistance, segmentnum, isFilled);
	tubemesh.setPosition(x, y, z);
	tubemesh.scale(scale.x, scale.y, scale.z);
	tubemesh.draw();
	tubemesh.clear();
}

void gDrawTubeTrapezodial(float x, float y, float z, int topouterradius,
		int topinnerradious, int buttomouterradious, int buttominnerradious,
		int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	gTube tubemesh(topouterradius,topinnerradious , buttomouterradious,buttominnerradious, h, glm::vec2(0.0f, 0.0f), segmentnum, isFilled);
	tubemesh.setPosition(x, y, z);
	tubemesh.scale(scale.x, scale.y, scale.z);
	tubemesh.draw();
	tubemesh.clear();
}

void gDrawTubeObliqueTrapezodial(float x, float y, float z, int topouterradius,
		int topinnerradious, int buttomouterradious, int buttominnerradious,
		int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum,
		bool isFilled) {
	gTube tubemesh(topouterradius,topinnerradious , buttomouterradious,buttominnerradious, h, shiftdistance, segmentnum, isFilled);
	tubemesh.setPosition(x, y, z);
	tubemesh.scale(scale.x, scale.y, scale.z);
	tubemesh.draw();
	tubemesh.clear();
}

gRenderer::gRenderer() {
}

void gRenderer::init() {
	width = gDefaultWidth();
	height = gDefaultHeight();
	unitwidth = gDefaultUnitWidth();
	unitheight = gDefaultUnitHeight();

	// TODO Check matrix maths
	projectionmatrix = glm::mat4(1.0f);
	projectionmatrix = glm::perspective(glm::radians(60.0f), (float)width / height, 0.0f, 1000.0f);
	projectionmatrixold = projectionmatrix;
	projectionmatrix2d = glm::ortho(0.0f, (float)unitwidth, (float)unitheight, 0.0f, -1.0f, 1.0f);
	viewmatrix = glm::mat4(1.0f);
	viewmatrixold = viewmatrix;
	cameraposition = glm::vec3(0.0f);

	// This changes pack and unpack alignments
	// Fixes alignment issues with 3 channel images
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	globalambientcolor.set(255, 255, 255, 255);
	isglobalambientcolorchanged = true;
	islightingenabled = true;

	lightsubo = new gUbo<gSceneLights>(0);

	colorshader = new gShader();
	colorshader->loadProgram(getShaderSrcColorVertex(), getShaderSrcColorFragment());
	colorshader->attachUbo("Lights", lightsubo);

	textureshader = new gShader();
	textureshader->loadProgram(getShaderSrcTextureVertex(), getShaderSrcTextureFragment());
	textureshader->use();
    textureshader->setMat4("projection", projectionmatrix);
    textureshader->setMat4("view", viewmatrix);

	imageshader = new gShader();
	imageshader->loadProgram(getShaderSrcImageVertex(), getShaderSrcImageFragment());
	imageshader->use();
	imageshader->setMat4("projection", projectionmatrix2d);

	fontshader = new gShader();
	fontshader->loadProgram(getShaderSrcFontVertex(), getShaderSrcFontFragment());

	skyboxshader = new gShader();
	skyboxshader->loadProgram(getShaderSrcSkyboxVertex(), getShaderSrcSkyboxFragment());
	skyboxshader->use();
	skyboxshader->setMat4("projection", projectionmatrix);
	skyboxshader->setMat4("view", viewmatrix);

	shadowmapshader = new gShader();
	shadowmapshader->loadProgram(getShaderSrcShadowmapVertex(), getShaderSrcShadowmapFragment());

	pbrshader = new gShader();
	pbrshader->loadProgram(getShaderSrcPbrVertex(), getShaderSrcPbrFragment());

	equirectangularshader = new gShader();
	equirectangularshader->loadProgram(getShaderSrcCubemapVertex(), getShaderSrcEquirectangularFragment());

	irradianceshader = new gShader();
	irradianceshader->loadProgram(getShaderSrcCubemapVertex(), getShaderSrcIrradianceFragment());

	prefiltershader = new gShader();
	prefiltershader->loadProgram(getShaderSrcCubemapVertex(), getShaderSrcPrefilterFragment());

	brdfshader = new gShader();
	brdfshader->loadProgram(getShaderSrcBrdfVertex(), getShaderSrcBrdfFragment());

	fboshader = new gShader();
	fboshader->loadProgram(getShaderSrcFboVertex(), getShaderSrcFboFragment());

	rendercolor = new gColor();
	rendercolor->set(255, 255, 255, 255);

	isfogenabled = false;
	fogno = -1;
	fogcolor.set(0.3f, 0.3f, 0.3f);
	fogmode = gRenderer::FOGMODE_EXP;
	fogdensity = 0.3f;
	foggradient = 2.0f;
	foglinearstart = 0.0f;
	foglinearend = 1.0f;

	isdepthtestenabled = false;
	depthtesttype = 0;
	depthtesttypeid[0] = GL_LESS;
	depthtesttypeid[1] = GL_ALWAYS;

	isalphablendingenabled = false;
	isalphatestenabled = false;
	//grid
	gridlineinterval = 1.0f;
	gridmaxvalue = 50;
	isgridenable = false;
	isgridxzenable = true;
	isgridxyenable = true;
	isgridyzenable = false;
	//bayrak2
	//xz init
	gridxzcolor.set(200, 0, 0, 175);
	gridxzmargincolor.set(30, 150, 30, 100); //0, 200, 0, 175
	//xy init
	gridxycolor.set(0, 200, 0, 175);
	gridxymargincolor.set(150, 30, 30, 100);//150, 30, 30, 100
	//yz init 100, 100, 200, 175
	gridyzcolor.set(100, 100, 200, 175);
	gridyzmargincolor.set(150, 30, 30, 100);//100, 100, 200, 175
}

gRenderer::~gRenderer() {
	delete colorshader;
	delete textureshader;
	delete imageshader;
	delete fontshader;
	delete skyboxshader;
	delete shadowmapshader;
	delete pbrshader;
	delete equirectangularshader;
	delete irradianceshader;
	delete prefiltershader;
	delete brdfshader;
	delete fboshader;
	delete rendercolor;
	delete lightsubo;
}

gShader* gRenderer::getColorShader() {
	return colorshader;
}

gShader* gRenderer::getTextureShader() {
	return textureshader;
}

gShader* gRenderer::getFontShader() {
	return fontshader;
}

gShader* gRenderer::getImageShader() {
	return imageshader;
}

gShader* gRenderer::getSkyboxShader() {
	return skyboxshader;
}

gShader* gRenderer::getShadowmapShader() {
	return shadowmapshader;
}

gShader* gRenderer::getPbrShader() {
	return pbrshader;
}

gShader* gRenderer::getEquirectangularShader() {
	return equirectangularshader;
}

gShader* gRenderer::getIrradianceShader() {
	return irradianceshader;
}

gShader* gRenderer::getPrefilterShader() {
	return prefiltershader;
}

gShader* gRenderer::getBrdfShader() {
	return brdfshader;
}

gShader* gRenderer::getFboShader() {
	return fboshader;
}

void gRenderer::setProjectionMatrix(glm::mat4 projectionMatrix) {
	projectionmatrix = projectionMatrix;
}

void gRenderer::setProjectionMatrix2d(glm::mat4 projectionMatrix2d) {
	projectionmatrix2d = projectionMatrix2d;
}

void gRenderer::setViewMatrix(glm::mat4 viewMatrix) {
	viewmatrix = viewMatrix;
}

void gRenderer::setCameraPosition(glm::vec3 cameraPosition) {
	cameraposition = cameraPosition;
}

void gRenderer::setCamera(gCamera* camera) {
	this->camera = camera;
}

const glm::mat4& gRenderer::getProjectionMatrix() const {
	return projectionmatrix;
}

const glm::mat4& gRenderer::getProjectionMatrix2d() const {
	return projectionmatrix2d;
}

const glm::mat4& gRenderer::getViewMatrix() const {
	return viewmatrix;
}

const glm::vec3& gRenderer::getCameraPosition() const {
	return cameraposition;
}

const gCamera* gRenderer::getCamera() const {
	return camera;
}

void gRenderer::backupMatrices() {
	projectionmatrixold = projectionmatrix;
	viewmatrixold = viewmatrix;
}

void gRenderer::restoreMatrices() {
	projectionmatrix = projectionmatrixold;
	viewmatrix = viewmatrixold;
}

void gRenderer::setScreenSize(int screenWidth, int screenHeight) {
	width = screenWidth;
	height = screenHeight;
	setCurrentResolution(getResolution(screenWidth, screenHeight));
}

void gRenderer::setUnitScreenSize(int unitWidth, int unitHeight) {
	unitwidth = unitWidth;
	unitheight = unitHeight;
	setUnitResolution(getResolution(unitWidth, unitHeight));
}

void gRenderer::setScreenScaling(int screenScaling) {
	screenscaling = screenScaling;
	gObject::setCurrentResolution(screenscaling, currentresolution);
}

int gRenderer::getWidth() {
	if (screenscaling >= G_SCREENSCALING_AUTO) return unitwidth;
	return width;
}

int gRenderer::getHeight() {
	if (screenscaling >= G_SCREENSCALING_AUTO) return unitheight;
	return height;
}

int gRenderer::getScreenWidth() {
	return width;
}

int gRenderer::getScreenHeight() {
	return height;
}

int gRenderer::getUnitWidth() {
	return unitwidth;
}

int gRenderer::getUnitHeight() {
	return unitheight;
}

int gRenderer::getScreenScaling() {
	return screenscaling;
}

void gRenderer::setCurrentResolution(int resolution) {
	currentresolution = resolution;
}

void gRenderer::setCurrentResolution(int screenWidth, int screenHeight) {
	setCurrentResolution(getResolution(screenWidth, screenHeight));
	gObject::setCurrentResolution(screenscaling, currentresolution);
}

void gRenderer::setUnitResolution(int resolution) {
	unitresolution = resolution;
}

void gRenderer::setUnitResolution(int screenWidth, int screenHeight) {
	setUnitResolution(getResolution(screenWidth, screenHeight));
}

int gRenderer::getResolution(int screenWidth, int screenHeight) {
	int resolutions[8][2] = {
			{7680, 4320}, //8k
			{3840, 2160}, //4k
			{2560, 1440}, //qhd
			{1920, 1080}, //fullhd
			{1280, 720},  //hd
			{960, 540},   //qfhd
			{800, 480},   //wvga
			{480, 320}    //hvga
	};

	int res = 0;
	for(int i = 0; i < 8; i++) {
		if (screenWidth >= resolutions[i][0] * 0.9f && screenHeight >= resolutions[i][1] * 0.9f) {
			res =i;
			break;
		}
	}

	return res;
}

int gRenderer::getCurrentResolution() {
	return currentresolution;
}

int gRenderer::getUnitResolution() {
	return unitresolution;
}

int gRenderer::scaleX(int x) {
	return (x * unitwidth) / width;
}

int gRenderer::scaleY(int y) {
	return (y * unitheight) / height;
}




void gRenderer::setColor(int r, int g, int b, int a) {
	rendercolor->set((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255);
}

void gRenderer::setColor(float r, float g, float b, float a) {
	rendercolor->set(r, g, b, a);
}

void gRenderer::setColor(gColor color) {
	rendercolor->set(color.r, color.g, color.b, color.a);
}

void gRenderer::setColor(gColor* color) {
	rendercolor->set(color->r, color->g, color->b, color->a);
}

gColor* gRenderer::getColor() {
	return rendercolor;
}

void gRenderer::clear() {
	G_CHECK_GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
	G_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gRenderer::clearColor(int r, int g, int b, int a) {
//    glBindFramebuffer(GL_FRAMEBUFFER, gFbo::defaultfbo);
	G_CHECK_GL(glClearColor((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255));
	G_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gRenderer::clearColor(gColor color) {
	G_CHECK_GL(glClearColor(color.r, color.g, color.b, color.a));
	G_CHECK_GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void gRenderer::enableFog() {
	isfogenabled = true;
}

void gRenderer::disableFog() {
	isfogenabled = false;
	fogno = -1;
}

void gRenderer::setFogNo(int no) {
	fogno = no;
}

void gRenderer::setFogColor(float r, float g, float b) {
	fogcolor.set(r, g, b);
}

void gRenderer::setFogColor(const gColor& color) {
	fogcolor.set(color.r, color.g, color.b);
}

void gRenderer::setFogMode(int mode) {
	fogmode = mode;
}

void gRenderer::setFogDensity(float value) {
	fogdensity = value;
}

void gRenderer::setFogGradient(float value) {
	foggradient = value;
}

void gRenderer::setFogLinearStart(float value) {
	foglinearstart = value;
}

void gRenderer::setFogLinearEnd(float value) {
	foglinearend = value;
}

bool gRenderer::isFogEnabled() {
	return isfogenabled;
}

int gRenderer::getFogNo() const {
	return fogno;
}

const gColor& gRenderer::getFogColor() const {
	return fogcolor;
}

int gRenderer::getFogMode() const {
	return fogmode;
}

float gRenderer::getFogDensity() const {
	return fogdensity;
}

float gRenderer::getFogGradient() const {
	return foggradient;
}

float gRenderer::getFogLinearStart() const {
	return foglinearstart;
}

float gRenderer::getFogLinearEnd() const {
	return foglinearend;
}

void gRenderer::enableLighting() {
	islightingenabled = true;
	updateLights();
}

void gRenderer::disableLighting() {
	islightingenabled = false;
	updateLights();
}

bool gRenderer::isLightingEnabled() {
	return islightingenabled;
}

void gRenderer::setLightingColor(int r, int g, int b, int a) {
	lightingcolor.set(r, g, b, a);
}

gColor* gRenderer::getLightingColor() {
	return &lightingcolor;
}

void gRenderer::setLightingPosition(glm::vec3 lightingPosition) {
	lightingposition = lightingPosition;
}

glm::vec3 gRenderer::getLightingPosition() {
	return lightingposition;
}

void gRenderer::setGlobalAmbientColor(int r, int g, int b, int a) {
	globalambientcolor.set(r, g, b, a);
	isglobalambientcolorchanged = true;
}

gColor* gRenderer::getGlobalAmbientColor() {
	return &globalambientcolor;
}

void gRenderer::addSceneLight(gLight* light) {
	scenelights.push_back(light);
}

gLight* gRenderer::getSceneLight(int lightNo) {
	return scenelights[lightNo];
}

int gRenderer::getSceneLightNum() {
	return scenelights.size();
}

void gRenderer::removeSceneLight(gLight* light) {
	if (scenelights.size() < 1) {
		return; // no lights to remove
	}
	scenelights.erase(std::remove_if(scenelights.begin(), scenelights.end(), [light](gLight* l) {
		return l == light;
	}), scenelights.end());
}

void gRenderer::removeAllSceneLights() {
	scenelights.clear();
}

void gRenderer::updateLights() {
	gSceneLights* data = lightsubo->getData();
	int previouslightnum = data->lightnum;
	data->lightnum = std::min((int) scenelights.size(), GLIST_MAX_LIGHTS);
	bool ischanged = false;
	bool isenabledchanged = false;
	for (int i = 0; i < data->lightnum; ++i) {
		const auto& item = scenelights[i];
		if (item->isChanged()) {
			data->lights[i].type = item->getType();
			data->lights[i].position = item->getPosition();
			data->lights[i].direction = item->getDirection();
			data->lights[i].ambient = item->getAmbientColor()->asVec4();
			data->lights[i].diffuse = item->getDiffuseColor()->asVec4();
			data->lights[i].specular = item->getSpecularColor()->asVec4();
			data->lights[i].constant = item->getAttenuationConstant();
			data->lights[i].linear = item->getAttenuationLinear();
			data->lights[i].quadratic = item->getAttenuationQuadratic();
			data->lights[i].spotcutoffangle = item->getSpotCutOffAngle();
			data->lights[i].spotoutercutoffangle = item->getSpotOuterCutOffAngle();
			item->setChanged(false);
			ischanged = true;
		}
		int bit = (1 << i);
		bool previous = data->enabledlights & bit;
		bool isenabled = islightingenabled && item->isEnabled();
		if (previous != isenabled) {
			isenabledchanged = true;

			// ~ flips the value bitwise
			// &= applies bitwise and
			// x << n shifts x by n amount bits to the left, for example shifting 0b0001 (1) by 4, results in binary 0b1000
			if (item->isEnabled()) {
				data->enabledlights |= bit; // Set the bit at the given index
			} else {
				data->enabledlights &= ~bit; // Clear the bit at the given index
			}
		}
	}
	if (isenabledchanged || ischanged) {
		int lastindex = scenelights.size() - 1;
		lightingcolor.set(scenelights[lastindex]->getAmbientColor());
		lightingposition = scenelights[lastindex]->getPosition();
	}
	if (ischanged) {
		lightsubo->update(0, offsetof(gSceneLights, lights[data->lightnum]) + sizeof(gSceneLightData));
	} else if (previouslightnum != data->lightnum) {
		lightsubo->update(0, sizeof(gSceneLights::lightnum));
	}
	if (isenabledchanged) {
		lightsubo->update(offsetof(gSceneLights, enabledlights), 1);
	}
	if (isglobalambientcolorchanged) {
		data->globalambientcolor = globalambientcolor.asVec4();
		lightsubo->update(offsetof(gSceneLights, globalambientcolor), sizeof(glm::vec4));
		isglobalambientcolorchanged = false;
	}
}

void gRenderer::enableDepthTest() {
	enableDepthTest(DEPTHTESTTYPE_LESS);
}

void gRenderer::enableDepthTest(int depthTestType) {
	G_CHECK_GL(glEnable(GL_DEPTH_TEST));
	G_CHECK_GL(glDepthFunc(depthtesttypeid[depthTestType]));
	isdepthtestenabled = true;
	depthtesttype = depthTestType;
}

void gRenderer::setDepthTestFunc(int depthTestType) {
	G_CHECK_GL(glDepthFunc(depthtesttypeid[depthTestType]));
	depthtesttype = depthTestType;
}

void gRenderer::disableDepthTest() {
	G_CHECK_GL(glDisable(GL_DEPTH_TEST));
	isdepthtestenabled = false;
}

bool gRenderer::isDepthTestEnabled() {
	return isdepthtestenabled;
}

int gRenderer::getDepthTestType() {
	return depthtesttype;
}

void gRenderer::enableAlphaBlending() {
    G_CHECK_GL(glEnable(GL_BLEND));
    G_CHECK_GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    isalphablendingenabled = true;
}

void gRenderer::disableAlphaBlending() {
	G_CHECK_GL(glDisable(GL_BLEND));
    isalphablendingenabled = false;
}

bool gRenderer::isAlphaBlendingEnabled() {
	return isalphablendingenabled;
}

void gRenderer::enableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
	glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1);
    isalphatestenabled = true;
#endif
}

void gRenderer::disableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
    glDisable(GL_ALPHA_TEST);
    isalphatestenabled = false;
#endif
}

bool gRenderer::isAlphaTestEnabled() {
	return isalphatestenabled;
}

#include "graphics/shaders/color_vert.h"
#include "graphics/shaders/color_frag.h"
#include "graphics/shaders/texture_vert.h"
#include "graphics/shaders/texture_frag.h"
#include "graphics/shaders/image_vert.h"
#include "graphics/shaders/image_frag.h"
#include "graphics/shaders/font_vert.h"
#include "graphics/shaders/font_frag.h"
#include "graphics/shaders/skybox_vert.h"
#include "graphics/shaders/skybox_frag.h"
#include "graphics/shaders/shadowmap_vert.h"
#include "graphics/shaders/shadowmap_frag.h"
#include "graphics/shaders/pbr_vert.h"
#include "graphics/shaders/pbr_frag.h"
#include "graphics/shaders/cubemap_vert.h"
#include "graphics/shaders/equirectangular_frag.h"
#include "graphics/shaders/irradiance_frag.h"
#include "graphics/shaders/prefilter_frag.h"
#include "graphics/shaders/brdf_vert.h"
#include "graphics/shaders/brdf_frag.h"
#include "graphics/shaders/fbo_vert.h"
#include "graphics/shaders/fbo_frag.h"

const std::string& gRenderer::getShaderSrcColorVertex() {
	static std::string str{shader_color_vert.data(), shader_color_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcColorFragment() {
	static std::string str{shader_color_frag.data(), shader_color_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcTextureVertex() {
	static std::string str{shader_texture_vert.data(), shader_texture_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcTextureFragment() {
	static std::string str{shader_texture_frag.data(), shader_texture_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcImageVertex() {
	static std::string str{shader_image_vert.data(), shader_image_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcImageFragment() {
	static std::string str{shader_image_frag.data(), shader_image_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcFontVertex() {
	static std::string str{shader_font_vert.data(), shader_font_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcFontFragment() {
	static std::string str{shader_font_frag.data(), shader_font_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcSkyboxVertex() {
	static std::string str{shader_skybox_vert.data(), shader_skybox_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcSkyboxFragment() {
	static std::string str{shader_skybox_frag.data(), shader_skybox_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcShadowmapVertex() {
	static std::string str{shader_shadowmap_vert.data(), shader_shadowmap_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcShadowmapFragment() {
	static std::string str{shader_shadowmap_frag.data(), shader_shadowmap_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcPbrVertex() {
	static std::string str{shader_pbr_vert.data(), shader_pbr_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcPbrFragment() {
	static std::string str{shader_pbr_frag.data(), shader_pbr_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcCubemapVertex() {
	static std::string str{shader_cubemap_vert.data(), shader_cubemap_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcEquirectangularFragment() {
	static std::string str{shader_equirectangular_frag.data(), shader_equirectangular_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcIrradianceFragment() {
	static std::string str{shader_irradiance_frag.data(), shader_irradiance_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcPrefilterFragment() {
	static std::string str{shader_prefilter_frag.data(), shader_prefilter_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcBrdfVertex() {
	static std::string str{shader_brdf_vert.data(), shader_brdf_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcBrdfFragment() {
	static std::string str{shader_brdf_frag.data(), shader_brdf_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcFboVertex() {
	static std::string str{shader_fbo_vert.data(), shader_fbo_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcFboFragment() {
	static std::string str{shader_fbo_frag.data(), shader_fbo_frag.size()};
	return str;
}

/*
 * Rotates The Pixel Data upside down. Hence rotates flips the image upside down
 */
void flipVertically(unsigned char* pixelData, int width, int height, int numChannels) {
    int rowsize = width * numChannels;
    unsigned char* temprow = new unsigned char[rowsize];

    for (int row = 0; row < height / 2; ++row) {
        // Calculate the corresponding row from the bottom
        int bottomrow = height - row - 1;

        // Swap the rows
        memcpy(temprow, pixelData + row * rowsize, rowsize);
        memcpy(pixelData + row * rowsize, pixelData + bottomrow * rowsize, rowsize);
        memcpy(pixelData + bottomrow * rowsize, temprow, rowsize);
    }

    delete[] temprow;
}

gImage gRenderer::takeScreenshot(int x, int y, int width, int height) {
	unsigned char* pixeldata = new unsigned char[width * height * 4];
	glReadPixels(x, getHeight() - y - height, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixeldata);
	flipVertically(pixeldata, width, height, 4);
	gImage screenshot;
	screenshot.setImageData(pixeldata, width, height, 4);
	//std::string imagePath = "output.png";   USE IT TO SAVE THE IMAGE
	// screenShot->saveImage(imagePath);  USE IT TO SAVE THE IMAGE
	return screenshot;
}

gImage gRenderer::takeScreenshot() {
	int height = gBaseApp::getAppManager()->getWindow()->getHeight();
	int width = gBaseApp::getAppManager()->getWindow()->getWidth();
	unsigned char* pixeldata = new unsigned char[width * height * 4];
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixeldata);
	flipVertically(pixeldata, width, height, 4);
	gImage screenshot;
	screenshot.setImageData(pixeldata, width, height, 4);
	//std::string imagePath = "output.png";   USE IT TO SAVE THE IMAGE
	// screenShot->saveImage(imagePath);  USE IT TO SAVE THE IMAGE
	return screenshot;
}

bool gRenderer::isSSAOEnabled() {
	return isssaoenabled;
}

void gRenderer::enableSSAO() {
	isssaoenabled = true;
}

void gRenderer::disableSSAO() {
	isssaoenabled = false;
}

void gRenderer::setSSAOBias(float value) {
	ssaobias = value;
}

float gRenderer::getSSAOBias() {
	return ssaobias;
}
