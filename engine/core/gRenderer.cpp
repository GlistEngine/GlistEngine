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
#include "gGrid.h"

//screenShot Related includes
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

void gDrawLine(float x1, float y1, float x2, float y2, float thickness) {
	G_PROFILE_ZONE_SCOPED_N("gDrawLine()");
	gLine linemesh;
	linemesh.setThickness(thickness);
	linemesh.draw(x1, y1, x2, y2);
	linemesh.clear();
}

void gDrawLine(float x1, float y1, float z1, float x2, float y2, float z2, float thickness) {
	G_PROFILE_ZONE_SCOPED_N("gDrawLine()");
	gLine linemesh;
	linemesh.setThickness(thickness);
	linemesh.draw(x1, y1, z1, x2, y2, z2);
	linemesh.clear();
}

void gDrawTriangle(float px, float py, float qx, float qy, float rx, float ry, bool is_filled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawTriangle()");
	gTriangle trianglemesh;
	trianglemesh.draw(px, py, qx, qy, rx, ry, is_filled);
	trianglemesh.clear();
}

void gDrawCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCircle()");
	gCircle circlemesh;
	circlemesh.draw(xCenter, yCenter, radius, isFilled, numberOfSides);
	circlemesh.clear();
}

void gDrawCross(float x, float y, float width, float height, float thickness, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCross()");
	gCross crossmesh;
	crossmesh.draw(x, y, width, height, thickness, isFilled);
	crossmesh.clear();
}

void gDrawArc(float xCenter, float yCenter, float radius, bool isFilled, int numberOfSides, float degree, float rotate) {
	G_PROFILE_ZONE_SCOPED_N("gDrawArc()");
	gArc arcmesh;
	arcmesh.draw(xCenter, yCenter, radius, isFilled, numberOfSides, degree, rotate);
	arcmesh.clear();
}

void gDrawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle) {
	G_PROFILE_ZONE_SCOPED_N("gDrawArrow()");
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
	G_PROFILE_ZONE_SCOPED_N("gDrawRectangle()");
	static gRectangle rectanglemesh;
 	rectanglemesh.draw(x, y, w, h, isFilled);
}

void gDrawRoundedRectangle(float x, float y, float w, float h, int radius, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawRoundedRectangle()");
	gRoundedRectangle roundedrectanglemesh;
	roundedrectanglemesh.draw(x, y, w, h, radius, isFilled);
	roundedrectanglemesh.clear();
}

void gDrawBox(float x, float y, float z, float w, float h, float d, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawBox()");
	gBox boxmesh;
	if(!isFilled) boxmesh.setDrawMode(gMesh::DRAWMODE_LINELOOP);
	boxmesh.setPosition(x, y, z);
	boxmesh.scale(w, h, d);
	boxmesh.draw();
	boxmesh.clear();
}

void gDrawBox(glm::mat4 transformationMatrix, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawBox()");
	gBox boxmesh;
	if(!isFilled) boxmesh.setDrawMode(gMesh::DRAWMODE_LINELOOP);
	boxmesh.setTransformationMatrix(transformationMatrix);
	boxmesh.draw();
	boxmesh.clear();
}

void gDrawSphere(float xPos, float yPos, float zPos, glm::vec3 scale, int xSegmentNum, int ySegmentNum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawSphere()");
	gSphere spheremesh(xSegmentNum, ySegmentNum, isFilled);
	spheremesh.setPosition(xPos, yPos, zPos);
	spheremesh.scale(scale.x, scale.y, scale.z);
	spheremesh.draw();
	spheremesh.clear();
}

void gDrawCylinder(float x, float y, float z, int r, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCylinder()");
	gCylinder cylindermesh(r, r, h, glm::vec2(0.0f, 0.0f), segmentnum, isFilled);
	cylindermesh.setPosition(x, y, z);
	cylindermesh.scale(scale.x, scale.y, scale.z);
	cylindermesh.draw();
	cylindermesh.clear();
}

void gDrawCylinderOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCylinderOblique()");
	gCylinder cylindermesh(r, r, h, shiftdistance, segmentnum, isFilled);
	cylindermesh.setPosition(x, y, z);
	cylindermesh.scale(scale.x, scale.y, scale.z);
	cylindermesh.draw();
	cylindermesh.clear();
}

void gDrawCylinderTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCylinderTrapezodial()");
	gCylinder cylindermesh(r1, r2, h, glm::vec2(0.0f, 0.0f), segmentnum, isFilled);
	cylindermesh.setPosition(x, y, z);
	cylindermesh.scale(scale.x, scale.y, scale.z);
	cylindermesh.draw();
	cylindermesh.clear();
}

void gDrawCylinderObliqueTrapezodial(float x, float y, float z, int r1, int r2, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCylinderObliqueTrapezodial()");
	gCylinder cylindermesh(r1, r2, h, shiftdistance, segmentnum, isFilled);
	cylindermesh.setPosition(x, y, z);
	cylindermesh.scale(scale.x, scale.y, scale.z);
	cylindermesh.draw();
	cylindermesh.clear();
}

void gDrawCone(float x, float y, float z, int r, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawCone()");
	gCone conemesh(r, h, glm::vec2(0.0f, 0.0f), segmentnum, isFilled);
	conemesh.setPosition(x, y, z);
	conemesh.scale(scale.x, scale.y, scale.z);
	conemesh.draw();
	conemesh.clear();
}

void gDrawConeOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawConeOblique()");
	gCone conemesh(r, h, shiftdistance, segmentnum, isFilled);
	conemesh.setPosition(x, y, z);
	conemesh.scale(scale.x, scale.y, scale.z);
	conemesh.draw();
	conemesh.clear();
}

void gDrawPyramid(float x, float y, float z, int r, int h, glm::vec3 scale, int numberofsides, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawPyramid()");
	gCone conemesh(r, h, glm::vec2(0.0f, 0.0f), numberofsides, isFilled);
	conemesh.setPosition(x, y, z);
	conemesh.scale(scale.x, scale.y, scale.z);
	conemesh.draw();
	conemesh.clear();
}

void gDrawPyramidOblique(float x, float y, float z, int r, int h, glm::vec2 shiftdistance, glm::vec3 scale, int numberofsides, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawPyramidOblique()");
	gCone conemesh(r, h, shiftdistance, numberofsides, isFilled);
	conemesh.setPosition(x, y, z);
	conemesh.scale(scale.x, scale.y, scale.z);
	conemesh.draw();
	conemesh.clear();
}

void gDrawTube(float x, float y, float z, int outerradius, int innerradious, int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawTube()");
	gTube tubemesh(outerradius,innerradious ,outerradius,innerradious, h, glm::vec2(0.0f, 0.0f), segmentnum, isFilled);
	tubemesh.setPosition(x, y, z);
	tubemesh.scale(scale.x, scale.y, scale.z);
	tubemesh.draw();
	tubemesh.clear();
}

void gDrawTubeOblique(float x, float y, float z, int outerradius,
		int innerradious, int h, glm::vec2 shiftdistance, glm::vec3 scale,
		int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawTubeOblique()");
	gTube tubemesh(outerradius,innerradious ,outerradius,innerradious, h, shiftdistance, segmentnum, isFilled);
	tubemesh.setPosition(x, y, z);
	tubemesh.scale(scale.x, scale.y, scale.z);
	tubemesh.draw();
	tubemesh.clear();
}

void gDrawTubeTrapezodial(float x, float y, float z, int topouterradius,
		int topinnerradious, int buttomouterradious, int buttominnerradious,
		int h, glm::vec3 scale, int segmentnum, bool isFilled) {
	G_PROFILE_ZONE_SCOPED_N("gDrawTubeTrapezodial()");
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
	G_PROFILE_ZONE_SCOPED_N("gDrawTubeObliqueTrapezodial()");
	gTube tubemesh(topouterradius,topinnerradious , buttomouterradious,buttominnerradious, h, shiftdistance, segmentnum, isFilled);
	tubemesh.setPosition(x, y, z);
	tubemesh.scale(scale.x, scale.y, scale.z);
	tubemesh.draw();
	tubemesh.clear();
}

void gRenderer::init() {
	G_PROFILE_ZONE_SCOPED_N("gRenderer::init()");
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
	camera = nullptr;

	// This changes pack and unpack alignments
	// Fixes alignment issues with 3 channel images
	updatePackUnpackAlignment(1);

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

	gridshader = new gShader();
	gridshader->loadProgram(getShaderSrcGridVertex(), getShaderSrcGridFragment());
	originalgrid = new gGrid();
	grid = originalgrid;
	isdevelopergrid = false;
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

gShader* gRenderer::getGridShader() {
	return gridshader;
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

void gRenderer::setColor(const gColor& color) {
	rendercolor->set(color.r, color.g, color.b, color.a);
}

void gRenderer::setColor(gColor* color) {
	rendercolor->set(color->r, color->g, color->b, color->a);
}

gColor* gRenderer::getColor() {
	return rendercolor;
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
	G_PROFILE_ZONE_SCOPED_N("gRenderer::updateLights()");
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
		lightsubo->update(0, offsetof(gSceneLights, lights) + (sizeof(gSceneLightData) * data->lightnum) + sizeof(gSceneLightData));
		isglobalambientcolorchanged = false; // we updated this
		return;  // here we already updated lightnum, enabledlights and globalambientcolor, no need to go further and do it twice.
	}

	if (previouslightnum != data->lightnum) {
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

//grid
void gRenderer::drawGrid() {
	if(grid != nullptr) grid->draw();
}

void gRenderer::drawGridYZ() {
	if(grid != nullptr) grid->drawYZ();
}

void gRenderer::drawGridXY() {
	if(grid != nullptr) grid->drawXY();
}

void gRenderer::drawGridXZ() {
	if(grid != nullptr) grid->drawXZ();
}

void gRenderer::enableGrid() {
	if(grid != nullptr) grid->enable();
}

void gRenderer::disableGrid() {
	if(grid != nullptr) grid->disable();
}

bool gRenderer::isGridEnabled() {
	if(grid != nullptr)  return grid->isEnabled();
	return false;
}

void gRenderer::setGridEnableAxis(bool xy, bool yz, bool xz) {
	if(grid != nullptr) {
		grid->setEnableAxisX(xy);
		grid->setEnableAxisY(xy);

		grid->setEnableAxisY(yz);
		grid->setEnableAxisZ(yz);

		grid->setEnableAxisX(xz);
		grid->setEnableAxisZ(xz);
	}
}

void gRenderer::setGridMaxLength(float length) {
	if(grid != nullptr) grid->setFarClip(length);
}

float gRenderer::getGridMaxLength() {
	if(grid != nullptr) return grid->getFarClip();
	return 1000.0f;
}

void gRenderer::setGridLineInterval(float intervalvalue) {
	if(grid != nullptr) return;
}

float gRenderer::getGridLineInterval() {
	if(grid != nullptr) return grid->getLineSpacing();
	return 1.0f;
}

void gRenderer::setGridColorofAxisXZ(int r, int g, int b, int a) {
	if(grid != nullptr) {
		grid->setColorAxisX(r,g,b,a);
		grid->setColorAxisZ(r,g,b,a);
	}
}

void gRenderer::setGridColorofAxisYZ(int r, int g, int b, int a) {
	if(grid != nullptr) {
		grid->setColorAxisY(r,g,b,a);
		grid->setColorAxisZ(r,g,b,a);
	}
}

void gRenderer::setGridColorofAxisXY(int r, int g, int b, int a) {
	if(grid != nullptr) {
		grid->setColorAxisX(r,g,b,a);
		grid->setColorAxisY(r,g,b,a);
	}
}

void gRenderer::setGridColorofAxisXZ(gColor *color) {
	if(grid != nullptr) {
		grid->setColorAxisX(color);
		grid->setColorAxisZ(color);
	}
}

void gRenderer::setGridColorofAxisYZ(gColor *color) {
	if(grid != nullptr) {
		grid->setColorAxisY(color);
		grid->setColorAxisZ(color);
	}
}

void gRenderer::setGridColorofAxisXY(gColor *color) {
	if(grid != nullptr) {
		grid->setColorAxisX(color);
		grid->setColorAxisY(color);
	}
}

void gRenderer::setGridColorofAxisWireFrameXZ(int r, int g, int b, int a) {
	if(grid != nullptr) grid->setColorWireFrameXZ(r, g, b, a);
}

void gRenderer::setGridColorofAxisWireFrameXY(int r, int g, int b, int a) {
	if(grid != nullptr) grid->setColorWireFrameXY(r, g, b, a);
}

void gRenderer::setGridColorofAxisWireFrameYZ(int r, int g, int b, int a) {
	if(grid != nullptr) grid->setColorWireFrameYZ(r, g, b, a);
}

void gRenderer::setGridColorofAxisWireFrameXZ(gColor *color) {
	if(grid != nullptr) grid->setColorWireFrameXZ(color);
}

void gRenderer::setGridColorofAxisWireFrameYZ(gColor *color) {
	if(grid != nullptr) grid->setColorWireFrameYZ(color);
}

void gRenderer::setGridColorofAxisWireFrameXY(gColor *color) {
	if(grid != nullptr) grid->setColorWireFrameXY(color);
}

void gRenderer::setGridEnableXY(bool xy) {
	if(grid != nullptr) grid->setEnableXY(xy);
}

void gRenderer::setGridEnableYZ(bool yz) {
	if(grid != nullptr) grid->setEnableYZ(yz);
}

void gRenderer::setGridEnableXZ(bool xz) {
	if(grid != nullptr) grid->setEnableXZ(xz);
}

bool gRenderer::isGridXYEnabled() {
	if(grid != nullptr) return grid->isXYEnabled();
	return false;
}

bool gRenderer::isGridYZEnabled() {
	if(grid != nullptr) return grid->isYZEnabled();
	return false;
}

bool gRenderer::isGridXZEnabled() {
	if(grid != nullptr) return grid->isXZEnabled();
	return false;
}

gGrid* gRenderer::getGrid() const {
	return grid;
}

// if its the dev's grid let them manage its lifetime and disable the renderer's instance
void gRenderer::setGrid(gGrid* newgrid) {
	if(!isdevelopergrid){
		delete originalgrid;
		isdevelopergrid = true;
	}
	grid->disable();
	grid = newgrid;
	grid->enable();

}
