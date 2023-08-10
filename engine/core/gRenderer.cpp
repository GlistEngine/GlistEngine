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
		gLogi("gRenderer") << prefix << "OpenGL ERROR at " << func << ", line " << line << ", error code: " << gToHex(error, 4);
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
	gLine linemesh, linemesh2, linemesh3;
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
	gRectangle rectanglemesh;
 	rectanglemesh.draw(x, y, w, h, isFilled);
 	rectanglemesh.clear();
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

gRenderer::gRenderer() {
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

	colorshader = new gShader();
	colorshader->loadProgram(getShaderSrcColorVertex(), getShaderSrcColorFragment());

	textureshader = new gShader();
	textureshader->loadProgram(getShaderSrcTextureVertex(), getShaderSrcTextureFragment());
    textureshader->setMat4("projection", projectionmatrix);
    textureshader->setMat4("view", viewmatrix);

	imageshader = new gShader();
	imageshader->loadProgram(getShaderSrcImageVertex(), getShaderSrcImageFragment());
	imageshader->setMat4("projection", projectionmatrix2d);

	fontshader = new gShader();
	fontshader->loadProgram(getShaderSrcFontVertex(), getShaderSrcFontFragment());

	skyboxshader = new gShader();
	skyboxshader->loadProgram(getShaderSrcSkyboxVertex(), getShaderSrcSkyboxFragment());
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

	globalambientcolor = new gColor();
	globalambientcolor->set(255, 255, 255, 255);

	lightingcolor = new gColor();
	lightingcolor->set(255, 255, 255, 255);
	islightingenabled = false;
	lightingposition = glm::vec3(0.0f);
	li = 0;

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
	delete globalambientcolor;
	delete lightingcolor;
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
	lightingcolor->set(0.0f, 0.0f, 0.0f, 1.0f);
	islightingenabled = true;
}

void gRenderer::disableLighting() {
	globalambientcolor = new gColor();
	globalambientcolor->set(255, 255, 255, 255);
	lightingcolor->set(globalambientcolor->r, globalambientcolor->g, globalambientcolor->b, globalambientcolor->a);
	lightingposition = glm::vec3(0.0f);
	removeAllSceneLights();
	islightingenabled = false;
}

bool gRenderer::isLightingEnabled() {
	return islightingenabled;
}

void gRenderer::setLightingColor(int r, int g, int b, int a) {
	lightingcolor->set(r, g, b, a);
}

void gRenderer::setLightingColor(gColor* color) {
	lightingcolor->set(color);
}

gColor* gRenderer::getLightingColor() {
	return lightingcolor;
}

void gRenderer::setLightingPosition(glm::vec3 lightingPosition) {
	lightingposition = lightingPosition;
}

glm::vec3 gRenderer::getLightingPosition() {
	return lightingposition;
}

void gRenderer::setGlobalAmbientColor(int r, int g, int b, int a) {
	globalambientcolor->set(r, g, b, a);
}

void gRenderer::setGlobalAmbientColor(gColor color) {
	globalambientcolor->set(color.r, color.g, color.b, color.a);
}

gColor* gRenderer::getGlobalAmbientColor() {
	return globalambientcolor;
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
	for (li = 0; li < scenelights.size(); li++) {
		if (light == scenelights[li]) {
			scenelights.erase(scenelights.begin() + li);
			break;
		}
	}
}

void gRenderer::removeAllSceneLights() {
	scenelights.clear();
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

const std::string gRenderer::getShaderSrcColorVertex() {
	const char* shadersource =
#ifdef ANDROID
"	#version 300 es\n"
"	precision highp float;\n"
#else
"	#version 330 core\n"
#endif
"	layout (location = 0) in vec3 aPos; // the position variable has attribute position 0\n"
"	layout (location = 1) in vec3 aNormal;\n"
"	layout (location = 2) in vec2 aTexCoords;\n"
"	layout (location = 3) in vec3 aTangent;\n"
"	layout (location = 4) in vec3 aBitangent;\n"
"	layout (location = 5) in int aUseNormalMap;\n"
"   uniform int aUseShadowMap;\n"
"\n"
"	uniform mat4 model;\n"
"	uniform mat4 view;\n"
"	uniform mat4 projection;\n"
"	uniform vec3 lightPos;\n"
"	uniform vec3 viewPos;\n"
"	uniform mat4 lightMatrix;\n"
"\n"
"	flat out int mUseNormalMap;\n"
"	flat out int mUseShadowMap;\n"
"\n"
"	out vec3 Normal;\n"
"	out vec3 FragPos;\n"
"	out vec2 TexCoords;\n"
"   out vec4 FragPosLightSpace;\n"
"	out vec3 TangentLightPos;\n"
"	out vec3 TangentViewPos;\n"
"	out vec3 TangentFragPos;\n"
"	out vec4 EyePosition;\n"
"\n"
"	void main() {\n"
"    	mUseShadowMap = aUseShadowMap;\n"
"	    FragPos = vec3(model * vec4(aPos, 1.0));\n"
"	    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"	    TexCoords = aTexCoords;\n"
"		FragPosLightSpace = lightMatrix * vec4(FragPos, 1.0);\n"
"	    mUseNormalMap = aUseNormalMap;\n"
"\n"
"	    if (aUseNormalMap > 0) {\n"
"		    mat3 normalMatrix = transpose(inverse(mat3(model)));\n"
"		    vec3 T = normalize(normalMatrix * aTangent);\n"
"		    vec3 N = normalize(normalMatrix * aNormal);\n"
"		    T = normalize(T - dot(T, N) * N);\n"
"		    vec3 B = cross(N, T);\n"
"		    \n"
"		    mat3 TBN = transpose(mat3(T, B, N));    \n"
"		    TangentLightPos = TBN * lightPos;\n"
"		    TangentViewPos  = TBN * viewPos;\n"
"		    TangentFragPos  = TBN * FragPos;\n"
"	    }\n"
"\n"
"    	mat4 modelViewMatrix = view * model;\n"
"    	mat4 projectedMatrix = projection * modelViewMatrix;\n"
"    	vec4 aPosVec4 = vec4(aPos, 1.0);\n"
"    	gl_Position = projectedMatrix * aPosVec4;\n"
"    	EyePosition = modelViewMatrix * aPosVec4;\n"
"	}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcColorFragment() {
	const char* shadersource =
#ifdef ANDROID
"	#version 300 es\n"
"	precision highp float;\n"
#else
"	#version 330 core\n"
#endif
"\n"
"	struct Material {\n"
"	    vec4 ambient;\n"
"	    vec4 diffuse;\n"
"	    vec4 specular;\n"
"	    float shininess;\n"
"	    sampler2D diffusemap;\n"
"	    sampler2D specularmap;\n"
"	    sampler2D normalMap;\n"
"		int useDiffuseMap;\n"
"		int useSpecularMap;\n"
"	};\n"
"\n"
"	struct Light {\n"
"		int type; //0-ambient, 1-directional, 2-point, 3-spot\n"
"	    vec3 position;\n"
"	    vec3 direction;\n"
"	    float cutOff;\n"
"	    float outerCutOff;\n"
"\n"
"	    vec4 ambient;\n"
"	    vec4 diffuse;\n"
"	    vec4 specular;\n"
"		\n"
"	    float constant;\n"
"	    float linear;\n"
"	    float quadratic;\n"
"	};\n"
"\n"
"struct Fog {\n"
"    vec3 color;\n"
"    float linearStart;\n"
"    float linearEnd;\n"
"    float density;\n"
"    float gradient;\n"
"\n"
"    int mode;\n"
"    bool enabled;\n"
"};\n"
"\n"
"	uniform Material material;\n"
"	#define MAX_LIGHTS 8\n"
"	uniform Light lights[MAX_LIGHTS];\n"
"\n"
"	uniform vec4 renderColor;\n"
"	uniform vec3 viewPos;\n"
"   uniform mat4 projection;\n"
" 	uniform mat4 view;\n"
"   uniform bool ssao_enabled;"
"   uniform float ssao_bias;"
"	uniform Fog fog;\n"
"\n"
"	in vec3 Normal;\n"
"	in vec3 FragPos;\n"
"	in vec2 TexCoords;\n"
"   in vec4 FragPosLightSpace;\n"
"	flat in int mUseNormalMap;\n"
"	in vec3 TangentLightPos;\n"
"	in vec3 TangentViewPos;\n"
"	in vec3 TangentFragPos;\n"
"	in vec4 EyePosition;\n"
"\n"
"	flat in int mUseShadowMap;\n"
"	uniform sampler2D shadowMap;\n"
"   uniform vec3 shadowLightPos;\n"
"\n"
"	out vec4 FragColor;\n"
"\n"
"   float calculateShadow(vec4 fragPosLightSpace) {\n"
"       vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
"       projCoords = projCoords * 0.5 + 0.5;\n"
"       float closestDepth = texture(shadowMap, projCoords.xy).r; \n"
"       float currentDepth = projCoords.z;\n"
"       vec3 normal = normalize(Normal);\n"
"       vec3 lightDir = normalize(shadowLightPos - FragPos);\n"
"       float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);\n"
"       float shadow = 0.0;\n"
"       ivec2 texelSize = ivec2(1, 1) / textureSize(shadowMap, 0);\n"
"       for(int x = -1; x <= 1; ++x) {\n"
"           for(int y = -1; y <= 1; ++y) {\n"
"               float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * vec2(texelSize.x, texelSize.y)).r;\n"
"               shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;\n"
"           }\n"
"       }\n"
"       shadow /= 9.0;\n"
"       if(projCoords.z > 1.0) shadow = 0.0;\n"
"       return shadow;\n"
"   }\n"
"\n"
"	vec4 calcAmbLight(Light light, vec4 materialAmbient) {\n"
"		vec4 ambient = light.ambient * materialAmbient;\n"
"		return ambient;\n"
"	}\n"
"\n"
"	vec4 calcDirLight(Light light, vec3 normal, vec3 viewDir, float shadowing, vec4 materialAmbient, vec4 materialDiffuse, vec4 materialSpecular) {\n"
"		vec3 lightDir = normalize(-light.direction);\n"
"		float diff;\n"
"		float spec;\n"
"		if (mUseNormalMap > 0) {\n"
"		    diff = max(dot(lightDir, normal), 0.0);\n"
"			vec3 halfwayDir = normalize(lightDir + viewDir);\n"
"			spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
"		} else {\n"
"			diff = max(dot(normal, lightDir), 0.0);\n"
"			vec3 reflectDir = reflect(-lightDir, normal);\n"
"			spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
"		}\n"
"		vec4 ambient = light.ambient * materialAmbient;\n"
"		vec4 diffuse = light.diffuse * vec4(diff) * materialDiffuse;\n"
"		vec4 specular = light.specular * vec4(spec) * materialSpecular;\n"
"	    if (mUseShadowMap > 0) {\n"
"			diffuse *= vec4(shadowing);\n"
"			specular *= vec4(shadowing);\n"
"	    }"
"		return (ambient + diffuse + specular);"
"	}\n"
"\n"
"	vec4 calcPointLight(Light light, vec3 normal, vec3 viewDir, float shadowing, vec4 materialAmbient, vec4 materialDiffuse, vec4 materialSpecular){\n"
"		vec3 lightDir;\n"
"		float distance;\n"
"		if (mUseNormalMap > 0) {\n"
"		    lightDir = normalize(TangentLightPos - TangentFragPos);\n"
"		    distance = length(TangentLightPos - TangentFragPos);\n"
"		} else {\n"
"			lightDir = normalize(light.position - FragPos);\n"
"			distance = length(light.position - FragPos);\n"
"		}\n"
"		float diff;\n"
"		float spec;\n"
"		if (mUseNormalMap > 0) {\n"
"		    diff = max(dot(lightDir, normal), 0.0);\n"
"			vec3 halfwayDir = normalize(lightDir + viewDir);\n"
"			spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
"		} else {\n"
"			diff = max(dot(normal, lightDir), 0.0);\n"
"			vec3 reflectDir = reflect(-lightDir, normal);\n"
"			spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
"		}\n"
"		vec4 ambient = light.ambient * materialAmbient;\n"
"		vec4 diffuse = light.diffuse * diff * materialDiffuse;\n"
"		vec4 specular = light.specular * spec * materialSpecular;\n"
"		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
"		ambient  *= attenuation;\n"
"    	diffuse  *= attenuation;\n"
"    	specular *= attenuation;\n"
"	    if (mUseShadowMap > 0) {\n"
"			diffuse  *= shadowing;\n"
"			specular *= shadowing;\n"
"	    }"
"		return (ambient + diffuse + specular);\n"
"	}\n"
"\n"
"	vec4 calcSpotLight(Light light, vec3 normal, vec3 viewDir, float shadowing, vec4 materialAmbient, vec4 materialDiffuse, vec4 materialSpecular){\n"
"		vec3 lightDir;\n"
"		float distance;\n"
"		if (mUseNormalMap > 0) {\n"
"		    lightDir = normalize(TangentLightPos - TangentFragPos);\n"
"		    distance = length(TangentLightPos - TangentFragPos);\n"
"		} else {\n"
"			lightDir = normalize(light.position - FragPos);\n"
"			distance = length(light.position - FragPos);\n"
"		}\n"
"		float diff;\n"
"		float spec;\n"
"		if (mUseNormalMap > 0) {\n"
"		    diff = max(dot(lightDir, normal), 0.0);\n"
"			vec3 halfwayDir = normalize(lightDir + viewDir);\n"
"			spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);\n"
"		} else {\n"
"			diff = max(dot(normal, lightDir), 0.0);\n"
"			vec3 reflectDir = reflect(-lightDir, normal);\n"
"			spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
"		}\n"
"		vec4 ambient  = light.ambient  * materialAmbient;\n"
"		vec4 diffuse  = light.diffuse  * diff * materialDiffuse;\n"
"		vec4 specular = light.specular * spec * materialSpecular;\n"
"\n"
"		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
"\n"
"		float theta = dot(lightDir, normalize(-light.direction)); \n"
"		float epsilon = (light.cutOff - light.outerCutOff);\n"
"		float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n"
"		ambient  *= attenuation;\n"
"    	diffuse  *= attenuation * intensity;\n"
"    	specular *= attenuation * intensity;\n"
"	    if (mUseShadowMap > 0) {\n"
"			diffuse *= shadowing;\n"
"			specular *= shadowing;\n"
"	    }"
"		return (ambient + diffuse + specular);\n"
"	}\n"
"\n"
"	float getFogVisibility(Fog fog, float distance) {\n"
"   	 float visibility = 0.0;\n"
"   	 if(fog.mode == 0) { // linear\n"
"      	 	float fogLength = fog.linearEnd - fog.linearStart;\n"
"        	visibility = (fog.linearEnd - distance) / fogLength;\n"
"    	} else if(fog.mode == 1) { // exp\n"
"       	 visibility = exp(-pow((distance * fog.density), fog.gradient));\n"
"    	}\n"
"\n"
"    	visibility = clamp(visibility, 0.0, 1.0);\n"
"    	return visibility;\n"
"	}\n"
"\n"
"	vec4 getSSAO() {\n"
"			const int kernelSize = 16;\n"
"			const float radius = 0.1;\n"
""
"			vec4 fragPos = view * vec4(FragPos, 1.0);\n"
"			vec4 clipPos = projection * fragPos\n;"
"			float ndcDepth = clipPos.z / clipPos.w;\n"
"			float depth = ((ndcDepth + 1.0) / 2.0);\n"
"			vec4 depthMap = vec4(depth, depth, depth, 1.0);\n"
""
"			vec3 tangentNormal = normalize(Normal * 2.0 - 1.0);\n"
"    		vec3 tangentFragPos = FragPos;\n"
""
"			vec3 ambient = vec3(0.0);\n"
""
"			for (int i = 0; i < kernelSize; ++i){\n"
"				vec2 co = vec2(float(i), 0.0);\n"
"				vec2 co2 = vec2(float(i), 1.0);\n"
"				vec3 randomVec = vec3(fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453), fract(sin(dot(co2.xy, vec2(12.9898, 78.233))) * 43758.5453), 0.0);\n"
"				vec3 sampleVec = tangentFragPos + randomVec * radius;\n"
""
"				vec4 offset = projection * view * vec4(sampleVec, 1.0);\n"
"        		offset.xy /= offset.w;\n"
"        		offset.xy = offset.xy * 0.5 + 0.5;\n"
""
"				float sampleDepth = depthMap.r;\n"
"				vec3 samplePos = (view * vec4(sampleVec * sampleDepth, 1.0)).xyz;\n"
""
"				float occlusion = clamp(dot(Normal, normalize(samplePos - FragPos)) - ssao_bias, 0.0, 1.0);\n"
"        		ambient += (1.0 - occlusion);\n"
"			}"
"    		ambient /= float(kernelSize);\n"
"    		ambient = mix(vec3(1.0), ambient, 0.5);\n"
"			return vec4(ambient, 1.0);\n"
"	}\n"
"\n"
""
"	void main() {\n"
"		vec4 result = vec4(0.0);\n"
"		vec3 norm;\n"
"		if (mUseNormalMap > 0) {\n"
"		    norm = normalize(texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0);  // this normal is in tangent space\n"
"		} else {\n"
"			norm = normalize(Normal);\n"
"		}\n"
"		vec3 viewDir;\n"
"		if (mUseNormalMap > 0) {\n"
"			viewDir = normalize(TangentViewPos - TangentFragPos);\n"
"		} else {\n"
"			viewDir = normalize(viewPos - FragPos);\n"
"		}\n"
"		vec4 materialAmbient;\n"
"		vec4 materialDiffuse;\n"
"	    if (material.useDiffuseMap > 0) {\n"
"	        materialAmbient = texture(material.diffusemap, TexCoords).rgba;\n"
"			materialDiffuse = texture(material.diffusemap, TexCoords).rgba;\n"
"	    } else {\n"
"	        materialAmbient = material.ambient;\n"
"			materialDiffuse = material.diffuse;\n"
"	    }\n"
"		if (material.useDiffuseMap > 0 && materialDiffuse.a < 0.5) {\n"
"			discard;\n"
"		}\n"
"		vec4 materialSpecular;\n"
"		if (material.useSpecularMap > 0) {\n"
"			materialSpecular = texture(material.specularmap, TexCoords).rgba;\n"
"		}\n"
"		else {\n"
"			materialSpecular = material.specular;\n"
"		}\n"
"		float shadowing;"
"	    if (mUseShadowMap > 0) {\n"
"			shadowing = 1.0 - calculateShadow(FragPosLightSpace);\n"
"	    }"
"		for (int i = 0; i < MAX_LIGHTS; i++) {\n"
"			if (lights[i].type == 0) {\n"
"				result += calcAmbLight(lights[i], materialAmbient);\n"
"			}\n"
"			else if (lights[i].type == 1) {\n"
"				result += calcDirLight(lights[i], norm, viewDir, shadowing, materialAmbient, materialDiffuse, materialSpecular);\n"
"			}\n"
"			else if (lights[i].type == 2) {\n"
"				result += calcPointLight(lights[i], norm, viewDir, shadowing, materialAmbient, materialDiffuse, materialSpecular);\n"
"			}\n"
"			else if (lights[i].type == 3) {\n"
"				result += calcSpotLight(lights[i], norm, viewDir, shadowing, materialAmbient, materialDiffuse, materialSpecular);\n"
"			}\n"
"		}\n"
"\n"
"		FragColor = result * renderColor;\n"
"\n"
"    	if(fog.enabled) {\n"
"        	float distance = abs(EyePosition.z / EyePosition.w);\n"
"       	FragColor = mix(vec4(fog.color, 1.0), FragColor, getFogVisibility(fog, distance));\n"
"    	}\n"
""
" 		if(ssao_enabled){\n"
"    		FragColor *= getSSAO();\n"
"		}"
"	}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcTextureVertex() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoords;\n"
"\n"
"out vec2 TexCoords;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"void main() {\n"
"    TexCoords = aTexCoords;    \n"
"    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcTextureFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"out vec4 FragColor;\n"
"  \n"
"in vec2 TexCoords;\n"
"\n"
"uniform sampler2D texture_diffuse1;\n"
"\n"
"void main()\n"
"{    \n"
"    FragColor = texture(texture_diffuse1, TexCoords);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcImageVertex() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>\n"
"	\n"
"out vec2 TexCoords;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 projection;\n"
"\n"
"void main()\n"
"{\n"
"    TexCoords = vertex.zw;\n"
"    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcImageFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D image;\n"
"uniform sampler2D maskimage;\n"
"uniform vec4 spriteColor;\n"
"uniform int isAlphaMasking;\n"
"vec4 mask;\n"
"\n"
"void main()\n"
"{    \n"
"	color = spriteColor * texture(image, TexCoords);"
"	if(isAlphaMasking == 1) {\n"
"       mask = texture(maskimage, TexCoords);\n"
"       mask.a = 1.0 - mask.r;\n"
"       mask.r = 1.0;\n"
"       mask.g = 1.0;\n"
"       mask.b = 1.0;\n"
"       color *= mask;\n"
"    }\n"
"    \n"
"} \n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFontVertex() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
"out vec2 TexCoords;\n"
"\n"
"uniform mat4 projection;\n"
"\n"
"void main() {\n"
"    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
"    TexCoords = vertex.zw;\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFontFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D text;\n"
"uniform vec3 textColor;\n"
"\n"
"void main() {    \n"
"    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
"    color = vec4(textColor, 1.0) * sampled;\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcSkyboxVertex() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"layout (location = 0) in vec3 aPos;\n"
"uniform int aIsHDR;\n"
"\n"
"out vec3 TexCoords;\n"
"flat out int mIsHDR;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"\n"
"void main()\n"
"{\n"
"    mIsHDR = aIsHDR;\n"
"    TexCoords = aPos;\n"
"    vec4 pos = projection * view * model * vec4(aPos, 1.0);\n"
"    gl_Position = pos.xyww;\n"
"}";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcSkyboxFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"out vec4 FragColor;\n"
"\n"
"in vec3 TexCoords;\n"
"flat in int mIsHDR;\n"
"vec4 fc;\n"
"\n"
"uniform samplerCube skymap;\n"
"\n"
"void main() {\n"
"    if (mIsHDR == 0) {\n"
"        fc = vec4(1.0, 0.0, 0.0, 1.0);\n"
"        fc = texture(skymap, TexCoords);\n"
"    } else if (mIsHDR == 1) {\n"
"        vec3 envColor = textureLod(skymap, TexCoords, 0.0).rgb;\n" //environmentMap->skymap
"        envColor = envColor / (envColor + vec3(1.0));\n"
"        envColor = pow(envColor, vec3(1.0 / 2.2));\n"
"        fc = vec4(envColor, 1.0);\n"
//"        fc = vec4(0.0, 1.0, 0.0, 1.0);\n"
"    } else {\n"
"        fc = vec4(1.0, 0.0, 0.0, 1.0);\n"
"    }\n"
"    FragColor = fc;\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcShadowmapVertex() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"layout (location = 0) in vec3 aPos;\n"
"\n"
"uniform mat4 lightMatrix;\n"
"uniform mat4 model;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = lightMatrix * model * vec4(aPos, 1.0);\n"
"}";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcShadowmapFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"\n"
"void main() {\n"
"    // gl_FragDepth = gl_FragCoord.z;\n"
"}";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcPbrVertex() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoords;\n"
"\n"
"out vec2 TexCoords;\n"
"out vec3 WorldPos;\n"
"out vec3 Normal;\n"
"\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"uniform mat4 model;\n"
"\n"
"void main()\n"
"{\n"
"    TexCoords = aTexCoords;\n"
"    WorldPos = vec3(model * vec4(aPos, 1.0));\n"
"    Normal = mat3(model) * aNormal;\n"
"\n"
"    gl_Position =  projection * view * vec4(WorldPos, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcPbrFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"out vec4 FragColor;\n"
"in vec2 TexCoords;\n"
"in vec3 WorldPos;\n"
"in vec3 Normal;\n"
"\n"
"// material parameters\n"
"uniform sampler2D albedoMap;\n"
"uniform sampler2D normalMap;\n"
"uniform sampler2D metallicMap;\n"
"uniform sampler2D roughnessMap;\n"
"uniform sampler2D aoMap;\n"
"\n"
"// IBL\n"
"uniform samplerCube irradianceMap;\n"
"uniform samplerCube prefilterMap;\n"
"uniform sampler2D brdfLUT;\n"
"\n"
"// lights\n"
"uniform int lightNum;\n"
"uniform vec3 lightPositions[8];\n"
"uniform vec3 lightColors[8];\n"
"\n"
"uniform vec3 camPos;\n"
"\n"
"const float PI = 3.14159265359;\n"
"// ----------------------------------------------------------------------------\n"
"// Easy trick to get tangent-normals to world-space to keep PBR code simplified.\n"
"// Don't worry if you don't get what's going on; you generally want to do normal \n"
"// mapping the usual way for performance anways; I do plan make a note of this \n"
"// technique somewhere later in the normal mapping tutorial.\n"
"vec3 getNormalFromMap()\n"
"{\n"
"    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;\n"
"\n"
"    vec3 Q1  = dFdx(WorldPos);\n"
"    vec3 Q2  = dFdy(WorldPos);\n"
"    vec2 st1 = dFdx(TexCoords);\n"
"    vec2 st2 = dFdy(TexCoords);\n"
"\n"
"    vec3 N   = normalize(Normal);\n"
"    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);\n"
"    vec3 B  = -normalize(cross(N, T));\n"
"    mat3 TBN = mat3(T, B, N);\n"
"\n"
"    return normalize(TBN * tangentNormal);\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"float DistributionGGX(vec3 N, vec3 H, float roughness)\n"
"{\n"
"    float a = roughness*roughness;\n"
"    float a2 = a*a;\n"
"    float NdotH = max(dot(N, H), 0.0);\n"
"    float NdotH2 = NdotH*NdotH;\n"
"\n"
"    float nom   = a2;\n"
"    float denom = (NdotH2 * (a2 - 1.0) + 1.0);\n"
"    denom = PI * denom * denom;\n"
"\n"
"    return nom / denom;\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"float GeometrySchlickGGX(float NdotV, float roughness)\n"
"{\n"
"    float r = (roughness + 1.0);\n"
"    float k = (r*r) / 8.0;\n"
"\n"
"    float nom   = NdotV;\n"
"    float denom = NdotV * (1.0 - k) + k;\n"
"\n"
"    return nom / denom;\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)\n"
"{\n"
"    float NdotV = max(dot(N, V), 0.0);\n"
"    float NdotL = max(dot(N, L), 0.0);\n"
"    float ggx2 = GeometrySchlickGGX(NdotV, roughness);\n"
"    float ggx1 = GeometrySchlickGGX(NdotL, roughness);\n"
"\n"
"    return ggx1 * ggx2;\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"vec3 fresnelSchlick(float cosTheta, vec3 F0)\n"
"{\n"
"    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)\n"
"{\n"
"    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);\n"
"}  \n"
"// ----------------------------------------------------------------------------\n"
"void main()\n"
"{		\n"
"    // material properties\n"
"    vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));\n"
"    float metallic = texture(metallicMap, TexCoords).r;\n"
"    float roughness = texture(roughnessMap, TexCoords).r;\n"
"    float ao = texture(aoMap, TexCoords).r;\n"
"\n"
"    // input lighting data\n"
"    vec3 N = getNormalFromMap();\n"
"    vec3 V = normalize(camPos - WorldPos);\n"
"    vec3 R = reflect(-V, N);\n"
"\n"
"    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 \n"
"    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)\n"
"    vec3 F0 = vec3(0.04); \n"
"    F0 = mix(F0, albedo, metallic);\n"
"\n"
"    // reflectance equation\n"
"    vec3 Lo = vec3(0.0);\n"
"    for(int i = 0; i < lightNum; ++i) {\n"
"        // calculate per-light radiance\n"
"        vec3 L = normalize(lightPositions[i] - WorldPos);\n"
"        vec3 H = normalize(V + L);\n"
"        float distance = length(lightPositions[i] - WorldPos);\n"
"        float attenuation = 1.0 / (distance * distance);\n"
"        vec3 radiance = lightColors[i] * attenuation;\n"
"\n"
"        // Cook-Torrance BRDF\n"
"        float NDF = DistributionGGX(N, H, roughness);\n"
"        float G   = GeometrySmith(N, V, L, roughness);\n"
"        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);\n"
"\n"
"        vec3 nominator    = NDF * G * F;\n"
"        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.\n"
"        vec3 specular = nominator / denominator;\n"
"\n"
"         // kS is equal to Fresnel\n"
"        vec3 kS = F;\n"
"        // for energy conservation, the diffuse and specular light can't\n"
"        // be above 1.0 (unless the surface emits light); to preserve this\n"
"        // relationship the diffuse component (kD) should equal 1.0 - kS.\n"
"        vec3 kD = vec3(1.0) - kS;\n"
"        // multiply kD by the inverse metalness such that only non-metals\n"
"        // have diffuse lighting, or a linear blend if partly metal (pure metals\n"
"        // have no diffuse light).\n"
"        kD *= 1.0 - metallic;\n"
"\n"
"        // scale light by NdotL\n"
"        float NdotL = max(dot(N, L), 0.0);\n"
"\n"
"        // add to outgoing radiance Lo\n"
"        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again\n"
"    }\n"
"\n"
"    // ambient lighting (we now use IBL as the ambient term)\n"
"    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);\n"
"    \n"
"    vec3 kS = F;\n"
"    vec3 kD = 1.0 - kS;\n"
"    kD *= 1.0 - metallic;\n"
"\n"
"    vec3 irradiance = texture(irradianceMap, N).rgb;\n"
"    vec3 diffuse      = irradiance * albedo;\n"
"\n"
"    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.\n"
"    const float MAX_REFLECTION_LOD = 4.0;\n"
"    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;\n"
"    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;\n"
"    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);\n"
"\n"
"    vec3 ambient = (kD * diffuse + specular) * ao;\n"
"\n"
"    vec3 color = ambient + Lo;\n"
"\n"
"    // HDR tonemapping\n"
"    color = color / (color + vec3(1.0));\n"
"    // gamma correct\n"
"    color = pow(color, vec3(1.0/2.2));\n"
"\n"
"    FragColor = vec4(color , 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcCubemapVertex() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"layout (location = 0) in vec3 aPos;\n"
"\n"
"out vec3 WorldPos;\n"
"\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"\n"
"void main()\n"
"{\n"
"    WorldPos = aPos;\n"
"    gl_Position =  projection * view * vec4(WorldPos, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcEquirectangularFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"out vec4 FragColor;\n"
"in vec3 WorldPos;\n"
"\n"
"uniform sampler2D equirectangularMap;\n"
"\n"
"const vec2 invAtan = vec2(0.1591, 0.3183);\n"
"\n"
"vec2 SampleSphericalMap(vec3 v) {\n"
"    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));\n"
"    uv *= invAtan;\n"
"    uv += 0.5;\n"
"    return uv;\n"
"}\n"
"\n"
"void main() {\n"
"    vec2 uv = SampleSphericalMap(normalize(WorldPos));\n"
"    vec3 color = texture(equirectangularMap, uv).rgb;\n"
" \n"
"    FragColor = vec4(color, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcIrradianceFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"\n"
"in vec3 WorldPos;\n"
"out vec4 FragColor;\n"
"uniform samplerCube environmentMap;\n"
"\n"
"const float PI = 3.14159265359;\n"
"\n"
"// Mirror binary digits about the decimal point\n"
"float radicalInverse_VdC(int bits)\n"
"{\n"
"    bits = (bits << 16) | (bits >> 16);\n"
"    bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);\n"
"    bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);\n"
"    bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);\n"
"    bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);\n"
"    return float(bits) * 2.3283064365386963e-10; // / 0x100000000\n"
"}\n"
"\n"
"// Randomish sequence that has pretty evenly spaced points\n"
"// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html\n"
"vec2 hammersley(int i, int N)\n"
"{\n"
"    return vec2(float(i)/float(N), radicalInverse_VdC(i));\n"
"}\n"
"\n"
"vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)\n"
"{\n"
"    float a = roughness * roughness;\n"
"\n"
"    float phi = 2.0 * PI * Xi.x;\n"
"    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));\n"
"    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);\n"
"\n"
"    // from spherical coordinates to cartesian coordinates\n"
"    vec3 H;\n"
"    H.x = cos(phi) * sinTheta;\n"
"    H.y = sin(phi) * sinTheta;\n"
"    H.z = cosTheta;\n"
"\n"
"    // from tangent-space vector to world-space sample vector\n"
"    vec3 up = abs(N.z) < 0.999 ? vec3 (0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);\n"
"    vec3 tangent = normalize(cross(up, N));\n"
"    vec3 bitangent = cross(N, tangent);\n"
"\n"
"    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;\n"
"    return normalize(sampleVec);\n"
"}\n"
"\n"
"void main()\n"
"{\n"
"    // Not the normal of the cube, but the normal that we're calculating the irradiance for\n"
"    vec3 normal = normalize(WorldPos);\n"
"    vec3 N = normal;\n"
"\n"
"    vec3 irradiance = vec3(0.0);\n"
"\n"
"    vec3 up = vec3(0.0, 1.0, 0.0);\n"
"    vec3 right = cross(up, normal);\n"
"    up = cross(normal, right);\n"
"\n"
"    // ------------------------------------------------------------------------------\n"
"\n"
"    const int SAMPLE_COUNT = 16384;\n"
"    float totalWeight = 0.0;\n"
"    for (int i = 0; i < SAMPLE_COUNT; ++i) {\n"
"        vec2 Xi = hammersley(i, SAMPLE_COUNT);\n"
"        vec3 H = importanceSampleGGX(Xi, N, 1.0);\n"
"\n"
"        // NdotH is equal to cos(theta)\n"
"        float NdotH = max(dot(N, H), 0.0);\n"
"        // With roughness == 1 in the distribution function we get 1/pi\n"
"        float D = 1.0 / PI;\n"
"        float pdf = (D * NdotH / (4.0)) + 0.0001; \n"
"\n"
"        float resolution = 1024.0; // resolution of source cubemap (per face)\n"
"        // with a higher resolution, we should sample coarser mipmap levels\n"
"        float saTexel = 4.0 * PI / (6.0 * resolution * resolution);\n"
"        // as we take more samples, we can sample from a finer mipmap.\n"
"        // And places where H is more likely to be sampled (higher pdf) we\n"
"        // can use a finer mipmap, otherwise use courser mipmap.\n"
"        // The tutorial treats this portion as optional to reduce noise but I think it's\n"
"        // actually necessary for importance sampling to get the correct result\n"
"        float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);\n"
"\n"
"        float mipLevel = 0.5 * log2(saSample / saTexel); \n"
"\n"
"        irradiance += textureLod(environmentMap, H, mipLevel).rgb * NdotH;\n"
"        // irradiance += texture(environmentMap, H).rgb * NdotH;\n"
"        totalWeight += NdotH;\n"
"    }\n"
"    irradiance = (PI * irradiance) / totalWeight;\n"
"\n"
"    // irradiance = texture(environmentMap, normal).rgb;\n"
"    FragColor = vec4(irradiance, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcPrefilterFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"out vec4 FragColor;\n"
"in vec3 WorldPos;\n"
"\n"
"uniform samplerCube environmentMap;\n"
"uniform float roughness;\n"
"\n"
"const float PI = 3.14159265359;\n"
"// ----------------------------------------------------------------------------\n"
"float DistributionGGX(vec3 N, vec3 H, float roughness)\n"
"{\n"
"    float a = roughness*roughness;\n"
"    float a2 = a*a;\n"
"    float NdotH = max(dot(N, H), 0.0);\n"
"    float NdotH2 = NdotH*NdotH;\n"
"\n"
"    float nom   = a2;\n"
"    float denom = (NdotH2 * (a2 - 1.0) + 1.0);\n"
"    denom = PI * denom * denom;\n"
"\n"
"    return nom / denom;\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html\n"
"// efficient VanDerCorpus calculation.\n"
"float RadicalInverse_VdC(uint bits)\n"
"{\n"
"     bits = (bits << 16u) | (bits >> 16u);\n"
"     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);\n"
"     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);\n"
"     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);\n"
"     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);\n"
"     return float(bits) * 2.3283064365386963e-10; // / 0x100000000\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"vec2 Hammersley(uint i, uint N)\n"
"{\n"
"	return vec2(float(i)/float(N), RadicalInverse_VdC(i));\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)\n"
"{\n"
"	float a = roughness*roughness;\n"
"\n"
"	float phi = 2.0 * PI * Xi.x;\n"
"	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));\n"
"	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);\n"
"\n"
"	// from spherical coordinates to cartesian coordinates - halfway vector\n"
"	vec3 H;\n"
"	H.x = cos(phi) * sinTheta;\n"
"	H.y = sin(phi) * sinTheta;\n"
"	H.z = cosTheta;\n"
"\n"
"	// from tangent-space H vector to world-space sample vector\n"
"	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);\n"
"	vec3 tangent   = normalize(cross(up, N));\n"
"	vec3 bitangent = cross(N, tangent);\n"
"\n"
"	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;\n"
"	return normalize(sampleVec);\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"void main()\n"
"{\n"
"    vec3 N = normalize(WorldPos);\n"
"\n"
"    // make the simplyfying assumption that V equals R equals the normal\n"
"    vec3 R = N;\n"
"    vec3 V = R;\n"
"\n"
"    const uint SAMPLE_COUNT = 1024u;\n"
"    vec3 prefilteredColor = vec3(0.0);\n"
"    float totalWeight = 0.0;\n"
"\n"
"    for(uint i = 0u; i < SAMPLE_COUNT; ++i)\n"
"    {\n"
"        // generates a sample vector that's biased towards the preferred alignment direction (importance sampling).\n"
"        vec2 Xi = Hammersley(i, SAMPLE_COUNT);\n"
"        vec3 H = ImportanceSampleGGX(Xi, N, roughness);\n"
"        vec3 L  = normalize(2.0 * dot(V, H) * H - V);\n"
"\n"
"        float NdotL = max(dot(N, L), 0.0);\n"
"        if(NdotL > 0.0)\n"
"        {\n"
"            // sample from the environment's mip level based on roughness/pdf\n"
"            float D   = DistributionGGX(N, H, roughness);\n"
"            float NdotH = max(dot(N, H), 0.0);\n"
"            float HdotV = max(dot(H, V), 0.0);\n"
"            float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;\n"
"\n"
"            float resolution = 512.0; // resolution of source cubemap (per face)\n"
"            float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);\n"
"            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);\n"
"\n"
"            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);\n"
"\n"
"            prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;\n"
"            totalWeight      += NdotL;\n"
"        }\n"
"    }\n"
"\n"
"    prefilteredColor = prefilteredColor / totalWeight;\n"
"\n"
"    FragColor = vec4(prefilteredColor, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcBrdfVertex() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec2 aTexCoords;\n"
"\n"
"out vec2 TexCoords;\n"
"\n"
"void main()\n"
"{\n"
"    TexCoords = aTexCoords;\n"
"	gl_Position = vec4(aPos, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcBrdfFragment() {
	const char* shadersource =
#ifdef ANDROID
"#version 300 es\n"
"precision highp float;\n"
#else
"#version 330 core\n"
#endif
"out vec2 FragColor;\n"
"in vec2 TexCoords;\n"
"\n"
"const float PI = 3.14159265359;\n"
"// ----------------------------------------------------------------------------\n"
"// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html\n"
"// efficient VanDerCorpus calculation.\n"
"float RadicalInverse_VdC(uint bits)\n"
"{\n"
"     bits = (bits << 16u) | (bits >> 16u);\n"
"     bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);\n"
"     bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);\n"
"     bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);\n"
"     bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);\n"
"     return float(bits) * 2.3283064365386963e-10; // / 0x100000000\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"vec2 Hammersley(uint i, uint N)\n"
"{\n"
"	return vec2(float(i)/float(N), RadicalInverse_VdC(i));\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)\n"
"{\n"
"	float a = roughness*roughness;\n"
"	\n"
"	float phi = 2.0 * PI * Xi.x;\n"
"	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));\n"
"	float sinTheta = sqrt(1.0 - cosTheta*cosTheta);\n"
"	\n"
"	// from spherical coordinates to cartesian coordinates - halfway vector\n"
"	vec3 H;\n"
"	H.x = cos(phi) * sinTheta;\n"
"	H.y = sin(phi) * sinTheta;\n"
"	H.z = cosTheta;\n"
"\n"
"	// from tangent-space H vector to world-space sample vector\n"
"	vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);\n"
"	vec3 tangent   = normalize(cross(up, N));\n"
"	vec3 bitangent = cross(N, tangent);\n"
"\n"
"	vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;\n"
"	return normalize(sampleVec);\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"float GeometrySchlickGGX(float NdotV, float roughness)\n"
"{\n"
"    // note that we use a different k for IBL\n"
"    float a = roughness;\n"
"    float k = (a * a) / 2.0;\n"
"\n"
"    float nom   = NdotV;\n"
"    float denom = NdotV * (1.0 - k) + k;\n"
"\n"
"    return nom / denom;\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)\n"
"{\n"
"    float NdotV = max(dot(N, V), 0.0);\n"
"    float NdotL = max(dot(N, L), 0.0);\n"
"    float ggx2 = GeometrySchlickGGX(NdotV, roughness);\n"
"    float ggx1 = GeometrySchlickGGX(NdotL, roughness);\n"
"\n"
"    return ggx1 * ggx2;\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"vec2 IntegrateBRDF(float NdotV, float roughness)\n"
"{\n"
"    vec3 V;\n"
"    V.x = sqrt(1.0 - NdotV*NdotV);\n"
"    V.y = 0.0;\n"
"    V.z = NdotV;\n"
"\n"
"    float A = 0.0;\n"
"    float B = 0.0;\n"
"\n"
"    vec3 N = vec3(0.0, 0.0, 1.0);\n"
"\n"
"    const uint SAMPLE_COUNT = 1024u;\n"
"    for(uint i = 0u; i < SAMPLE_COUNT; ++i)\n"
"    {\n"
"        // generates a sample vector that's biased towards the\n"
"        // preferred alignment direction (importance sampling).\n"
"        vec2 Xi = Hammersley(i, SAMPLE_COUNT);\n"
"        vec3 H = ImportanceSampleGGX(Xi, N, roughness);\n"
"        vec3 L = normalize(2.0 * dot(V, H) * H - V);\n"
"\n"
"        float NdotL = max(L.z, 0.0);\n"
"        float NdotH = max(H.z, 0.0);\n"
"        float VdotH = max(dot(V, H), 0.0);\n"
"\n"
"        if(NdotL > 0.0)\n"
"        {\n"
"            float G = GeometrySmith(N, V, L, roughness);\n"
"            float G_Vis = (G * VdotH) / (NdotH * NdotV);\n"
"            float Fc = pow(1.0 - VdotH, 5.0);\n"
"\n"
"            A += (1.0 - Fc) * G_Vis;\n"
"            B += Fc * G_Vis;\n"
"        }\n"
"    }\n"
"    A /= float(SAMPLE_COUNT);\n"
"    B /= float(SAMPLE_COUNT);\n"
"    return vec2(A, B);\n"
"}\n"
"// ----------------------------------------------------------------------------\n"
"void main()\n"
"{\n"
"    vec2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);\n"
"    FragColor = integratedBRDF;\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFboVertex() {
	const char* shadersource =
#ifdef ANDROID
			"#version 300 es\n"
			"precision highp float;\n"
#else
			"#version 330 core\n"
#endif
			"layout (location = 0) in vec2 aPos;"
			"layout (location = 1) in vec2 aTexCoords;"
			""
			"out vec2 TexCoords;"
			""
			"void main()"
			"{"
			"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
			"    TexCoords = aTexCoords;"
			"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFboFragment() {
	const char* shadersource =
#ifdef ANDROID
			"#version 300 es\n"
			"precision highp float;\n"
#else
			"#version 330 core\n"
#endif
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			""
			"void main()"
			"{ "
			"    FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);"
			"}\n";
	return std::string(shadersource);
}

bool gRenderer::isSSAOEnabled() {
	return isssaoenabled;
}

void gRenderer::enableSSAO() {
	isssaoenabled = true;
	colorshader->setMat4("projection", projectionmatrix);
	colorshader->setMat4("view", viewmatrix);
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
