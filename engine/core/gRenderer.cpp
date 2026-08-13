/*
 * gRenderManager.cpp
 *
 *  Created on: 4 Ara 2020
 *      Author: Acer
 */

#include "gRenderer.h"
#include "gFbo.h"

#include <cstdlib>
#include <algorithm>

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
//#include "gArrow.h"
#include "gUbo.h"
#include "gShader.h"
#include "gCamera.h"
#include "gGrid.h"
#include "gNode.h"

// screenShot Related includes
#include "stb/stb_image_write.h"
#include "gBaseApp.h"
#include "gImage.h"
#include "gTracy.h"

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

// --- Global Draw Functions ---

void gDrawLine(float x1, float y1, float x2, float y2) {
	G_PROFILE_ZONE_SCOPED_N("gDrawLine()");
	gRenderObject::getRenderer()->drawLine(x1, y1, x2, y2);
}

void gDrawLine(float x1, float y1, float x2, float y2, float thickness) {
	G_PROFILE_ZONE_SCOPED_N("gDrawLine()");
	gRenderObject::getRenderer()->drawLine(x1, y1, x2, y2, thickness);
}

void gDrawLine(float x1, float y1, float z1, float x2, float y2, float z2, float thickness) {
	G_PROFILE_ZONE_SCOPED_N("gDrawLine()");
	gRenderObject::getRenderer()->drawLine(x1, y1, z1, x2, y2, z2, thickness);
}

void gDrawLine(float x1, float y1, float x2, float y2, float thickness, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gDrawLine()");
	gRenderObject::getRenderer()->drawLine(x1, y1, x2, y2, thickness, rotateAngle, pivotx, pivoty);
}

void gDrawTriangle(float px, float py, float qx, float qy, float rx, float ry, bool is_filled, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gDrawTriangle()");
	gRenderObject::getRenderer()->drawTriangle(px, py, qx, qy, rx, ry, is_filled, rotateAngle, pivotx, pivoty);
}

void gDrawCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCircle()");
	gRenderObject::getRenderer()->drawCircle(xCenter, yCenter, radius, isFilled, numberOfSides, rotateAngle, pivotx, pivoty);
}

void gDrawCross(float x, float y, float width, float height, float thickness, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCross()");
	gRenderObject::getRenderer()->drawCross(x, y, width, height, thickness, isFilled, rotateAngle, pivotx, pivoty);
}

void gDrawArc(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides, float degree, float rotate, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gDrawArc()");
	gRenderObject::getRenderer()->drawArc(xCenter, yCenter, radius, isFilled, numberOfSides, degree, rotate, rotateAngle, pivotx, pivoty);
}

void gDrawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle, float thickness, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gDrawArrow()");
	gRenderObject::getRenderer()->drawArrow(x1, y1, length, angle, tipLength, tipAngle, thickness, rotateAngle, pivotx, pivoty);
}

void gDrawRectangle(float x, float y, float w, float h, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gDrawRectangle()");
	gRenderObject::getRenderer()->drawRectangle(x, y, w, h, isFilled, rotateAngle, pivotx, pivoty);
}

void gDrawRoundedRectangle(float x, float y, float w, float h, int radius, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gDrawRoundedRectangle()");
	gRenderObject::getRenderer()->drawRoundedRectangle(x, y, w, h, radius, isFilled, rotateAngle, pivotx, pivoty);
}

void gDrawBox(float x, float y, float z, float w, float h, float d, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawBox()");
	gRenderObject::getRenderer()->drawBox(x, y, z, w, h, d, isFilled);
}

void gDrawBox(float x, float y, float z,float w, float h, float d, float rotateAngle, float axisX, float axisY, float axisZ, bool isFilled) {
    G_PROFILE_ZONE_SCOPED_N("gDrawBox()");
    gRenderObject::getRenderer()->drawBox(x, y, z, w, h, d, rotateAngle, axisX, axisY, axisZ, isFilled);
}

void gDrawBox(glm::mat4 transformationMatrix, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawBox()");
	gRenderObject::getRenderer()->drawBox(transformationMatrix, isFilled);
}

void gDrawSphere(float xPos, float yPos, float zPos, glm::vec3 scale, int xSegmentNum, int ySegmentNum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawSphere()");
	gRenderObject::getRenderer()->drawSphere(xPos, yPos, zPos, scale, xSegmentNum, ySegmentNum, isFilled);
}

void gDrawCylinder(float x, float y, float z, int r, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCylinder()");
	gRenderObject::getRenderer()->drawCylinder(x, y, z, r, h, scale, segmentnum, isFilled);
}

void gDrawCylinderOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCylinderOblique()");
	gRenderObject::getRenderer()->drawCylinderOblique(x, y, z, r, h, shiftdistance, scale, segmentnum, isFilled);
}

void gDrawCylinderTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCylinderTrapezodial()");
	gRenderObject::getRenderer()->drawCylinderTrapezodial(x, y, z, r1, r2, h, scale, segmentnum, isFilled);
}

void gDrawCylinderObliqueTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCylinderObliqueTrapezodial()");
	gRenderObject::getRenderer()->drawCylinderObliqueTrapezodial(x, y, z, r1, r2, h, shiftdistance, scale, segmentnum, isFilled);
}

void gDrawCone(float x, float y, float z, int r, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCone()");
	gRenderObject::getRenderer()->drawCone(x, y, z, r, h, scale, segmentnum, isFilled);
}

void gDrawConeOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawConeOblique()");
	gRenderObject::getRenderer()->drawConeOblique(x, y, z, r, h, shiftdistance, scale, segmentnum, isFilled);
}

void gDrawPyramid(float x, float y, float z, int r, int h, glm::vec3 scale, int numberofsides, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawPyramid()");
	gRenderObject::getRenderer()->drawPyramid(x, y, z, r, h, scale, numberofsides, isFilled);
}

void gDrawPyramidOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int numberofsides, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawPyramidOblique()");
	gRenderObject::getRenderer()->drawPyramidOblique(x, y, z, r, h, shiftdistance, scale, numberofsides, isFilled);
}

void gDrawTube(float x, float y, float z, int outerradius, int innerradious, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawTube()");
	gRenderObject::getRenderer()->drawTube(x, y, z, outerradius, innerradious, h, scale, segmentnum, isFilled);
}

void gDrawTubeOblique(float x, float y, float z, int outerradius, int innerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawTubeOblique()");
	gRenderObject::getRenderer()->drawTubeOblique(x, y, z, outerradius, innerradious, h, shiftdistance, scale, segmentnum, isFilled);
}

void gDrawTubeTrapezodial(float x, float y, float z, int topouterradius, int topinnerradious, int buttomouterradious, int buttominnerradious, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawTubeTrapezodial()");
	gRenderObject::getRenderer()->drawTubeTrapezodial(x, y, z, topouterradius, topinnerradious, buttomouterradious, buttominnerradious, h, scale, segmentnum, isFilled);
}

void gDrawTubeObliqueTrapezodial(float x, float y, float z, int topouterradius, int topinnerradious, int buttomouterradious, int buttominnerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
    G_PROFILE_ZONE_SCOPED_N("gDrawTubeObliqueTrapezodial()");
    gRenderObject::getRenderer()->drawTubeObliqueTrapezodial(x, y, z, topouterradius, topinnerradious, buttomouterradious, buttominnerradious, h, shiftdistance, scale, segmentnum, isFilled);
}

// --- gRenderer Class Member Implementations ---

gRenderer::~gRenderer() {
}

void gRenderer::init() {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::init()");
	width = gDefaultWidth();
	height = gDefaultHeight();
	unitwidth = gDefaultUnitWidth();
	unitheight = gDefaultUnitHeight();

	projectionmatrix = glm::mat4(1.0f);
	projectionmatrix = glm::perspective(glm::radians(60.0f), (float)width / height, 0.0f, 1000.0f);
	projectionmatrixold = projectionmatrix;
	projectionmatrix2d = glm::ortho(0.0f, (float)unitwidth, (float)unitheight, 0.0f, -1.0f, 1.0f);
	viewmatrix = glm::mat4(1.0f);
	viewmatrixold = viewmatrix;
	cameraposition = glm::vec3(0.0f);
	camera = nullptr;

	updatePackUnpackAlignment(1);

	globalambientcolor.set(255, 255, 255, 255);
	isglobalambientcolorchanged = true;
	islightingenabled = true;

	lightsubo = new gUbo<gSceneLights>(0);
	sceneubo = new gUbo<gSceneData>(1);

	colorshader = new gShader();
	colorshader->loadProgram(getShaderSrcColorVertex(), getShaderSrcColorFragment());
	colorshader->attachUbo("Lights", lightsubo);
	colorshader->attachUbo("Scene", sceneubo);

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
	pbrshader->attachUbo("Lights", lightsubo);

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
	fogmode = FOGMODE_EXP;
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

	boundframebuffer = gFbo::defaultfbo;
	viewportx = 0;
	viewporty = 0;
	viewportwidth = getScreenWidth();
	viewportheight = getScreenHeight();

	isssaoenabled = false;
	ssaobias = 0.025f;
	ssaoradius = 0.5f;
	ssaostrength = 1.0f;
	isssaoallocated = false;
	isssaorendering = false;
	isssaodebug = false;
	ssaofbo = nullptr;
	ssaoresultfbo = nullptr;
	ssaoshader = nullptr;
	ssaoblurshader = nullptr;
	ssaorealdefaultfbo = 0;
	ssaoprevframebuffer = 0;
	ssaoprevviewport[0] = 0;
	ssaoprevviewport[1] = 0;
	ssaoprevviewport[2] = 0;
	ssaoprevviewport[3] = 0;
	isgammacorrectionenabled = false;
	ishdrenabled = false;
	issoftshadowsenabled = false;

	gridshader = new gShader();
	gridshader->loadProgram(getShaderSrcGridVertex(), getShaderSrcGridFragment());
	originalgrid = new gGrid();
	grid = originalgrid;

	fullscreenquadvao = 0;
	fullscreenquadvbo = 0;
	createFullscreenQuad(fullscreenquadvao, fullscreenquadvbo);

	createPrimitiveMeshes();

	enableSSAO();
	enableSoftShadows();
}

void gRenderer::createPrimitiveMeshes() {
	linemesh = std::make_unique<gLine>();
	linemesh2 = std::make_unique<gLine>();
	linemesh3 = std::make_unique<gLine>();
	trianglemesh = std::make_unique<gTriangle>();
	circlemesh = std::make_unique<gCircle>();
	crossmesh = std::make_unique<gCross>();
	arcmesh = std::make_unique<gArc>();
	rectanglemesh = std::make_unique<gRectangle>();
	roundedrectanglemesh = std::make_unique<gRoundedRectangle>();
	boxmesh = std::make_unique<gBox>();
	//arrowmesh = std::make_unique<gArrow>();
}

void gRenderer::destroyPrimitiveMeshes() {
	linemesh = nullptr;
	linemesh2 = nullptr;
	linemesh3 = nullptr;
	trianglemesh = nullptr;
	circlemesh = nullptr;
	crossmesh = nullptr;
	arcmesh = nullptr;
	rectanglemesh = nullptr;
	roundedrectanglemesh = nullptr;
	boxmesh = nullptr;
	//arrowmesh = nullptr;
}

void gRenderer::cleanup() {
	destroyPrimitiveMeshes();
	cleanupSSAOResources();

	delete colorshader;
	delete textureshader;
	delete fontshader;
	delete imageshader;
	delete skyboxshader;
	delete shadowmapshader;
	delete pbrshader;
	delete equirectangularshader;
	delete irradianceshader;
	delete prefiltershader;
	delete brdfshader;
	delete gridshader;
	delete fboshader;

	delete rendercolor;
	delete lightsubo;
	delete sceneubo;

	colorshader = nullptr;
	textureshader = nullptr;
	fontshader = nullptr;
	imageshader = nullptr;
	skyboxshader = nullptr;
	shadowmapshader = nullptr;
	pbrshader = nullptr;
	equirectangularshader = nullptr;
	irradianceshader = nullptr;
	prefiltershader = nullptr;
	brdfshader = nullptr;
	gridshader = nullptr;
	fboshader = nullptr;

	rendercolor = nullptr;
	lightsubo = nullptr;
	sceneubo = nullptr;

	deleteFullscreenQuad(fullscreenquadvao, &fullscreenquadvbo);
	fullscreenquadvao = 0;
	fullscreenquadvbo = 0;
}

// --- Primitive Drawing Member Functions ---

void gRenderer::drawLine(float x1, float y1, float x2, float y2) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawLine()");
	if (linemesh) linemesh->draw(x1, y1, x2, y2, 0.0f, 0.5f, 0.5f);
}

void gRenderer::drawLine(float x1, float y1, float x2, float y2, float thickness) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawLine()");
	if (linemesh) {
		linemesh->setThickness(thickness);
		linemesh->draw(x1, y1, x2, y2, 0.0f, 0.5f, 0.5f);
	}
}

void gRenderer::drawLine(float x1, float y1, float z1, float x2, float y2, float z2, float thickness) {
    G_PROFILE_ZONE_SCOPED_N("gRenderer::drawLine3D()");
    if (linemesh3) linemesh3->draw(x1, y1, z1, x2, y2, z2, 0.0f, 0.5f, 0.5f);
}

void gRenderer::drawLine(float x1, float y1, float x2, float y2, float thickness, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawLine()");
	if (linemesh) {
		linemesh->setThickness(thickness);
		linemesh->draw(x1, y1, x2, y2, rotateAngle, pivotx, pivoty);
	}
}

void gRenderer::drawTriangle(float px, float py, float qx, float qy, float rx, float ry, bool is_filled, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawTriangle()");
	if (trianglemesh) trianglemesh->draw(px, py, qx, qy, rx, ry, is_filled, rotateAngle, pivotx, pivoty);
}

void gRenderer::drawCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawCircle()");
	if (circlemesh) circlemesh->draw(xCenter, yCenter, radius, isFilled, numberOfSides, rotateAngle, pivotx, pivoty);
}

void gRenderer::drawCross(float x, float y, float width, float height, float thickness, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawCross()");
	if (crossmesh) crossmesh->draw(x, y, width, height, thickness, isFilled, rotateAngle, pivotx, pivoty);
}

void gRenderer::drawArc(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides, float degree, float rotate, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawArc()");
	if (arcmesh) arcmesh->draw(xCenter, yCenter, radius, isFilled, numberOfSides, degree, rotate, rotateAngle, pivotx, pivoty);
}

void gRenderer::drawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle, float thickness, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawArrow()");
	//if (arrowmesh) arrowmesh->draw(x1, y1, length, angle, tipLength, tipAngle, thickness, rotateAngle, pivotx, pivoty);
}

void gRenderer::drawRectangle(float x, float y, float w, float h, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawRectangle()");
	if (rectanglemesh) rectanglemesh->draw(x, y, w, h, isFilled, rotateAngle, pivotx, pivoty);
}

void gRenderer::drawRoundedRectangle(float x, float y, float w, float h, int radius, bool isFilled, float rotateAngle, float pivotx, float pivoty) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawRoundedRectangle()");
	if (roundedrectanglemesh) roundedrectanglemesh->draw(x, y, w, h, radius, isFilled, rotateAngle, pivotx, pivoty);
}

void gRenderer::drawBox(float x, float y, float z, float w, float h, float d, bool isFilled) {
    G_PROFILE_ZONE_SCOPED_N("gRenderer::drawBox()");
    if (boxmesh) {
        boxmesh->setPosition(x, y, z);
        boxmesh->setScale(w, h, d);
        boxmesh->setOrientation(
        glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
        );

        boxmesh->draw();
    }
}

void gRenderer::drawBox(float x, float y, float z, float w, float h, float d, float rotateAngle, float axisX, float axisY, float axisZ, bool isFilled) {
    G_PROFILE_ZONE_SCOPED_N("gRenderer::drawBox()");
    if (boxmesh) {
        boxmesh->setPosition(x, y, z);
        boxmesh->setScale(w, h, d);
        glm::vec3 axis(axisX, axisY, axisZ);

        if (glm::length(axis) > 0.0f) {

            axis = glm::normalize(axis);

            glm::quat rotation =
                glm::angleAxis(rotateAngle, axis);

            rotation = glm::normalize(rotation);

            boxmesh->setOrientation(rotation);

        } else {

            boxmesh->setOrientation(
                glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
            );
        }

        boxmesh->draw();
    }
}

void gRenderer::drawBox(glm::mat4 transformationMatrix, bool isFilled) {
    G_PROFILE_ZONE_SCOPED_N("gRenderer::drawBox()");
    if (boxmesh) {
        boxmesh->setTransformationMatrix(transformationMatrix);
        boxmesh->draw();
    }
}

void gRenderer::drawSphere(float xPos, float yPos, float zPos, glm::vec3 scale, int xSegmentNum, int ySegmentNum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawSphere()");
}

void gRenderer::drawCylinder(float x, float y, float z, int r, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawCylinder()");
}

void gRenderer::drawCylinderOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawCylinderOblique()");
}

void gRenderer::drawCylinderTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawCylinderTrapezodial()");
}

void gRenderer::drawCylinderObliqueTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawCylinderObliqueTrapezodial()");
}

void gRenderer::drawCone(float x, float y, float z, int r, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawCone()");
}

void gRenderer::drawConeOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawConeOblique()");
}

void gRenderer::drawPyramid(float x, float y, float z, int r, int h, glm::vec3 scale, int numberofsides, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawPyramid()");
}

void gRenderer::drawPyramidOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int numberofsides, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawPyramidOblique()");
}

void gRenderer::drawTube(float x, float y, float z, int outerradius, int innerradious, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawTube()");
}

void gRenderer::drawTubeOblique(float x, float y, float z, int outerradius, int innerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawTubeOblique()");
}

void gRenderer::drawTubeTrapezodial(float x, float y, float z, int topouterradius, int topinnerradious, int buttomouterradious, int buttominnerradious, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawTubeTrapezodial()");
}

void gRenderer::drawTubeObliqueTrapezodial(float x, float y, float z, int topouterradius, int topinnerradious, int buttomouterradious, int buttominnerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::drawTubeObliqueTrapezodial()");
}

// --- Getter / Setter / Matrices ---

unsigned int gRenderer::getFullscreenQuadVAO() const {
	return fullscreenquadvao;
}

gShader* gRenderer::getColorShader() { return colorshader; }
gShader* gRenderer::getTextureShader() { return textureshader; }
gShader* gRenderer::getFontShader() { return fontshader; }
gShader* gRenderer::getImageShader() { return imageshader; }
gShader* gRenderer::getSkyboxShader() { return skyboxshader; }
gShader* gRenderer::getShadowmapShader() { return shadowmapshader; }
gShader* gRenderer::getPbrShader() { return pbrshader; }
gShader* gRenderer::getEquirectangularShader() { return equirectangularshader; }
gShader* gRenderer::getIrradianceShader() { return irradianceshader; }
gShader* gRenderer::getPrefilterShader() { return prefiltershader; }
gShader* gRenderer::getBrdfShader() { return brdfshader; }
gShader* gRenderer::getFboShader() { return fboshader; }
gShader* gRenderer::getGridShader() { return gridshader; }

GLuint gRenderer::getBoundFramebuffer() const {
	return boundframebuffer;
}

void gRenderer::getViewport(int& x, int& y, int& width, int& height) const {
	x = viewportx;
	y = viewporty;
	width = viewportwidth;
	height = viewportheight;
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

const glm::mat4& gRenderer::getProjectionMatrix() const { return projectionmatrix; }
const glm::mat4& gRenderer::getProjectionMatrix2d() const { return projectionmatrix2d; }
const glm::mat4& gRenderer::getViewMatrix() const { return viewmatrix; }
const glm::vec3& gRenderer::getCameraPosition() const { return cameraposition; }
const gCamera* gRenderer::getCamera() const { return camera; }

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
	updateProjectionMatrix2d();
}

void gRenderer::setUnitScreenSize(int unitWidth, int unitHeight) {
	unitwidth = unitWidth;
	unitheight = unitHeight;
	setUnitResolution(getResolution(unitWidth, unitHeight));
	updateProjectionMatrix2d();
}

void gRenderer::setScreenScaling(int screenScaling) {
	screenscaling = screenScaling;
	gObject::setCurrentResolution(screenscaling, currentresolution);
	updateProjectionMatrix2d();
}

void gRenderer::updateProjectionMatrix2d() {
	gRenderer* r = gRenderObject::getRenderer();
	if(!r) return;
	const int projectionwidth = screenscaling >= G_SCREENSCALING_AUTO ? unitwidth : width;
	const int projectionheight = screenscaling >= G_SCREENSCALING_AUTO ? unitheight : height;
	if(projectionwidth <= 0 || projectionheight <= 0) return;
	r->projectionmatrix2d = glm::ortho(0.0f, (float)projectionwidth,
			(float)projectionheight, 0.0f, -1.0f, 1.0f);
}

int gRenderer::getWidth() {
	if (screenscaling >= G_SCREENSCALING_AUTO) {
		return unitwidth;
	}
	return width;
}

int gRenderer::getHeight() {
	if (screenscaling >= G_SCREENSCALING_AUTO) {
		return unitheight;
	}
	return height;
}

int gRenderer::getScreenWidth() { return width; }
int gRenderer::getScreenHeight() { return height; }
int gRenderer::getUnitWidth() { return unitwidth; }
int gRenderer::getUnitHeight() { return unitheight; }
int gRenderer::getScreenScaling() { return screenscaling; }

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
			res = i;
			break;
		}
	}
	return res;
}

int gRenderer::getCurrentResolution() { return currentresolution; }
int gRenderer::getUnitResolution() { return unitresolution; }

float gRenderer::getScaleMultiplier() {
	return width / (float)unitwidth;
}

int gRenderer::scaleX(int x) { return (x * unitwidth) / width; }
int gRenderer::scaleY(int y) { return (y * unitheight) / height; }

int gRenderer::unscaleX(int x) {
	float scale = width / (float)unitwidth;
	return x * scale;
}

int gRenderer::unscaleY(int y) {
	float scale = height / (float)unitheight;
	return y * scale;
}

void gRenderer::setColor(int r, int g, int b, int a) {
	rendercolor->set((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255);
	updateScene();
}

void gRenderer::setColor(float r, float g, float b, float a) {
	rendercolor->set(r, g, b, a);
	updateScene();
}

void gRenderer::setColor(const gColor& color) {
	rendercolor->set(color.r, color.g, color.b, color.a);
	updateScene();
}

void gRenderer::setColor(gColor* color) {
	rendercolor->set(color->r, color->g, color->b, color->a);
	updateScene();
}

gColor* gRenderer::getColor() { return rendercolor; }

void gRenderer::enableFog() { isfogenabled = true; updateScene(); }
void gRenderer::disableFog() { isfogenabled = false; updateScene(); }
void gRenderer::setFogNo(int no) { fogno = no; }
void gRenderer::setFogColor(float r, float g, float b) { fogcolor.set(r, g, b); }
void gRenderer::setFogColor(const gColor& color) { fogcolor.set(color.r, color.g, color.b); }
void gRenderer::setFogMode(int mode) { fogmode = mode; }
void gRenderer::setFogDensity(float value) { fogdensity = value; }
void gRenderer::setFogGradient(float value) { foggradient = value; }
void gRenderer::setFogLinearStart(float value) { foglinearstart = value; }
void gRenderer::setFogLinearEnd(float value) { foglinearend = value; }

bool gRenderer::isFogEnabled() { return isfogenabled; }
int gRenderer::getFogNo() const { return fogno; }
const gColor& gRenderer::getFogColor() const { return fogcolor; }
int gRenderer::getFogMode() const { return fogmode; }
float gRenderer::getFogDensity() const { return fogdensity; }
float gRenderer::getFogGradient() const { return foggradient; }
float gRenderer::getFogLinearStart() const { return foglinearstart; }
float gRenderer::getFogLinearEnd() const { return foglinearend; }

void gRenderer::enableLighting() { islightingenabled = true; updateLights(); }
void gRenderer::disableLighting() { islightingenabled = false; updateLights(); }
bool gRenderer::isLightingEnabled() { return islightingenabled; }

void gRenderer::setLightingColor(int r, int g, int b, int a) {
	lightingcolor.set(r, g, b, a);
}

gColor* gRenderer::getLightingColor() { return &lightingcolor; }

void gRenderer::setLightingPosition(glm::vec3 lightingPosition) {
	lightingposition = lightingPosition;
}

glm::vec3 gRenderer::getLightingPosition() { return lightingposition; }

void gRenderer::setGlobalAmbientColor(int r, int g, int b, int a) {
	globalambientcolor.set(r, g, b, a);
	isglobalambientcolorchanged = true;
}

gColor* gRenderer::getGlobalAmbientColor() { return &globalambientcolor; }

void gRenderer::addSceneLight(gLight* light) { scenelights.push_back(light); }
gLight* gRenderer::getSceneLight(int lightNo) { return scenelights[lightNo]; }
int gRenderer::getSceneLightNum() { return scenelights.size(); }

void gRenderer::removeSceneLight(gLight* light) {
	if (scenelights.empty()) return;
	scenelights.erase(std::remove_if(scenelights.begin(), scenelights.end(), [light](gLight* l) {
		return l == light;
	}), scenelights.end());
}

void gRenderer::removeAllSceneLights() { scenelights.clear(); }

void gRenderer::updateLights() {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::updateLights()");
	if(lightsubo == nullptr) return;
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
		int bit = 1 << i;
		bool previous = data->enabledlights & bit;
		bool isenabled = islightingenabled && item->isEnabled();
		if (previous != isenabled) {
			isenabledchanged = true;
			if (isenabled) {
				data->enabledlights |= bit;
			} else {
				data->enabledlights &= ~bit;
			}
		}
	}
	if (isenabledchanged || ischanged) {
		int lastindex = scenelights.size() - 1;
		lightingcolor.set(scenelights[lastindex]->getAmbientColor());
		lightingposition = scenelights[lastindex]->getPosition();
	}
	if (isglobalambientcolorchanged) {
		data->globalambientcolor = globalambientcolor.asVec4();
	}
	if (ischanged) {
		lightsubo->update(0, sizeof(gSceneLights));
		isglobalambientcolorchanged = false;
		return;
	}

	if (previouslightnum != data->lightnum) {
		lightsubo->update(0, sizeof(gSceneLights::lightnum));
	}
	if (isenabledchanged) {
		lightsubo->update(offsetof(gSceneLights, enabledlights), 1);
	}
	if (isglobalambientcolorchanged) {
		lightsubo->update(offsetof(gSceneLights, globalambientcolor), sizeof(glm::vec4));
		isglobalambientcolorchanged = false;
	}
}

void gRenderer::updateScene() {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::updateScene()");
	if(sceneubo == nullptr) return;
	gSceneData* data = sceneubo->getData();
	bool ischanged = false;

	if (data->rendercolor != rendercolor) {
		data->rendercolor = rendercolor;
		ischanged = true;
	}

	if (data->viewpos != cameraposition) {
		data->viewpos = cameraposition;
		ischanged = true;
	}

	if (data->viewmatrix != viewmatrix) {
		data->viewmatrix = viewmatrix;
		ischanged = true;
	}

	int previousflags = data->flags;
	data->flags = 0;

	if (isssaoenabled) data->flags |= ENABLE_SSAO;
	if (isfogenabled) data->flags |= ENABLE_FOG;
	if (isgammacorrectionenabled) data->flags |= ENABLE_GAMMA;
	if (ishdrenabled) data->flags |= ENABLE_HDR;
	if (issoftshadowsenabled) data->flags |= ENABLE_SOFT_SHADOWS;

	bool flagschanged = previousflags != data->flags;

	bool fogChanged = false;
	if (isFogEnabled()) {
		gSceneFogData newfog{};
		newfog.color = fogcolor.asVec3();
		newfog.linearStart = foglinearstart;
		newfog.linearEnd = foglinearend;
		newfog.density = fogdensity;
		newfog.gradient = foggradient;
		newfog.mode = fogmode;

		if (memcmp(&data->fog, &newfog, sizeof(gSceneFogData)) != 0) {
			data->fog = newfog;
			fogChanged = true;
		}
	}

	if (ischanged || flagschanged || fogChanged) {
		sceneubo->update(0, sizeof(gSceneData));
	}
}

void gRenderer::gPushMatrix() {
    gNode::ismatrixpushing = true;
}

void gRenderer::gPopMatrix() {
    if (gNode::matrixpopmeshptr) {
        gNode::matrixpopmeshptr->popMatrix();
        gNode::matrixpopmeshptr = nullptr;
    }
}

// --- Shader Sources ---

#include "graphics/shaders/grid_vert.h"
#include "graphics/shaders/grid_frag.h"
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
#include "graphics/shaders/ssao_vert.h"
#include "graphics/shaders/ssao_frag.h"
#include "graphics/shaders/ssao_blur_frag.h"

const std::string& gRenderer::getShaderSrcGridVertex() {
	static std::string str{shader_grid_vert.data(), shader_grid_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcGridFragment() {
	static std::string str{shader_grid_frag.data(), shader_grid_frag.size()};
	return str;
}

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

const std::string& gRenderer::getShaderSrcSSAOVertex() {
	static std::string str{shader_ssao_vert.data(), shader_ssao_vert.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcSSAOFragment() {
	static std::string str{shader_ssao_frag.data(), shader_ssao_frag.size()};
	return str;
}

const std::string& gRenderer::getShaderSrcSSAOBlurFragment() {
	static std::string str{shader_ssao_blur_frag.data(), shader_ssao_blur_frag.size()};
	return str;
}

// --- SSAO & Soft Shadows Implementations ---

void gRenderer::enableSSAO() {
	isssaoenabled = true;
	updateScene();
}

void gRenderer::disableSSAO() {
	isssaoenabled = false;
	updateScene();
}

bool gRenderer::isSSAOEnabled() {
	return isssaoenabled;
}

bool gRenderer::isSSAOAllocated() {
	return isssaoallocated;
}

void gRenderer::enableSoftShadows() {
	issoftshadowsenabled = true;
	updateScene();
}

void gRenderer::disableSoftShadows() {
	issoftshadowsenabled = false;
	updateScene();
}

bool gRenderer::isSoftShadowsEnabled() {
	return issoftshadowsenabled;
}

// These six were declared in the header and never defined, so anything calling one
// failed to link. Both backends' shaders have always carried the flag - ENABLE_GAMMA
// and ENABLE_HDR in color_frag.glsl, the same bits in the Vulkan scene block - and
// updateScene() has always published them; the only missing piece was a way to ask.
void gRenderer::enableGammaCorrection() {
	isgammacorrectionenabled = true;
	updateScene();
}

void gRenderer::disableGammaCorrection() {
	isgammacorrectionenabled = false;
	updateScene();
}

bool gRenderer::isGammaCorrectionEnabled() {
	return isgammacorrectionenabled;
}

void gRenderer::enableHDR() {
	ishdrenabled = true;
	updateScene();
}

void gRenderer::disableHDR() {
	ishdrenabled = false;
	updateScene();
}

bool gRenderer::isHDREnabled() {
	return ishdrenabled;
}

void gRenderer::cleanupSSAOResources() {
	delete ssaofbo;
	delete ssaoresultfbo;
	delete ssaoshader;
	delete ssaoblurshader;
	ssaofbo = nullptr;
	ssaoresultfbo = nullptr;
	ssaoshader = nullptr;
	ssaoblurshader = nullptr;
	isssaoallocated = false;
}

void gRenderer::beginSSAO() {
	if (!isssaoenabled) return;
	isssaorendering = true;
}

void gRenderer::endSSAO() {
	if (!isssaoenabled) return;
	isssaorendering = false;
}

// --- Grid Implementations ---

bool gRenderer::isGridEnabled() {
	if (grid) return grid->isEnabled();
	return false;
}

void gRenderer::enableGrid() {
	if (grid) grid->enable();
}

void gRenderer::disableGrid() {
	if (grid) grid->disable();
}

void gRenderer::drawGrid() {
	if (grid && grid->isEnabled()) {
		grid->draw();
	}
}

void gRenderer::setGridEnableAxis(bool x, bool y, bool z) {
	if (grid) grid->setEnableAxis(x, y, z);
}

void gRenderer::setGridEnableXY(bool enable) {
	if (grid) grid->setEnableXY(enable);
}

void gRenderer::setGridEnableXZ(bool enable) {
	if (grid) grid->setEnableXZ(enable);
}

void gRenderer::setGridEnableYZ(bool enable) {
	if (grid) grid->setEnableYZ(enable);
}

bool gRenderer::isGridXYEnabled() {
	if (grid) return grid->isXYEnabled();
	return false;
}

bool gRenderer::isGridXZEnabled() {
	if (grid) return grid->isXZEnabled();
	return false;
}

bool gRenderer::isGridYZEnabled() {
	if (grid) return grid->isYZEnabled();
	return false;
}

void gRenderer::setGridMaxLength(float length) {
}

float gRenderer::getGridMaxLength() {
	return 0.0f;
}

void gRenderer::setGridLineInterval(float interval) {
}

float gRenderer::getGridLineInterval() {
	return 0.0f;
}

void gRenderer::setGridColorofAxisXY(int r, int g, int b, int a) {
}

void gRenderer::setGridColorofAxisXY(gColor* color) {
}

void gRenderer::setGridColorofAxisWireFrameXY(int r, int g, int b, int a) {
}

void gRenderer::setGridColorofAxisWireFrameXY(gColor* color) {
}

void gRenderer::setGridColorofAxisXZ(int r, int g, int b, int a) {
}

void gRenderer::setGridColorofAxisXZ(gColor* color) {
}

void gRenderer::setGridColorofAxisWireFrameXZ(int r, int g, int b, int a) {
}

void gRenderer::setGridColorofAxisWireFrameXZ(gColor* color) {
}

void gRenderer::setGridColorofAxisYZ(int r, int g, int b, int a) {
}

void gRenderer::setGridColorofAxisYZ(gColor* color) {
}

void gRenderer::setGridColorofAxisWireFrameYZ(int r, int g, int b, int a) {
}

void gRenderer::setGridColorofAxisWireFrameYZ(gColor* color) {
}
