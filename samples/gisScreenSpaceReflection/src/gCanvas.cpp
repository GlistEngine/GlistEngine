/*
* gCanvas.cpp
*
*  Created on: May 6, 2020
*      Author: Noyan Culum
*/

#include "gCanvas.h"
#include "gInputManager.h"
#include "gMaterial.h"

/*
 * How to use screen-space reflections (SSR) in your own scene:
 *
 * step 1: load the SSR shader's vertex/fragment source from disk.
 * step 2: allocate the scene-capture FBO and the two ping-pong reflection FBOs at screen size.
 * step 3: attach the SSR shader to the material of every mesh that should be reflective.
 * step 4: each frame, begin the capture pass and bind the scene FBO.
 * step 5: store the camera's current matrices and clip planes for the reflection math.
 * step 6: draw the entire scene into the capture FBO.
 * step 7: end the capture pass and build mipmaps for blurred/rough reflections.
 * step 8: begin the reflection pass, passing a skybox (or null) as the miss fallback.
 * step 9: draw only the reflective meshes so the shader can raymarch their reflections.
 * step 10: end the reflection pass, saving this frame's matrix and swapping the ping-pong buffers.
 * step 11: draw the scene normally to the screen.
 * step 12: begin the composite pass, feeding it the captured color and the reflection buffer.
 * step 13: draw the reflective meshes again so the shader blends the reflection onto them.
 * step 14: end the composite pass.
 * step 15: on window resize, reallocate the FBOs at the new screen size.
 */

void gCanvas::setupssr() {
	ssrcapturenearclip = 0.01f;
	ssrcapturefarclip = 1000.0f;
	ssrcurrentreflectionindex = 0;
	ssrhasvalidhistory = false;
	ssrframeindex = 0;
	ssrpreviousviewprojection = glm::mat4(1.0f);

	renderer->disableSSAO();

	ssrshader.load(gObject::gGetShadersDir() + "screenspacereflection_vert.glsl", gObject::gGetShadersDir() + "screenspacereflection_frag.glsl");
	resizessr(renderer->getScreenWidth(), renderer->getScreenHeight());
}

void gCanvas::resizessr(int width, int height) {
	ssrscenefbo.allocate(width, height, false, true);

	renderer->bindTexture(ssrscenefbo.getDepthTextureId());
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_NEAREST, GL_NEAREST);
	renderer->bindTexture(ssrscenefbo.getTextureId());
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	renderer->generateMipMap();

	ssrreflectionfbo[0].allocate(width, height);
	ssrreflectionfbo[1].allocate(width, height);
	renderer->bindTexture(ssrreflectionfbo[0].getTextureId());
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	renderer->bindTexture(ssrreflectionfbo[1].getTextureId());
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);
	renderer->resetTexture();

	ssrhasvalidhistory = false;
}

void gCanvas::beginssrcapture() {
	renderer->activateTexture(8);
	renderer->resetTexture();
	renderer->activateTexture(9);
	renderer->resetTexture();
	renderer->activateTexture(10);
	renderer->resetTexture();
	renderer->activateTexture(11);
	renderer->resetTexture();

	gRenderObject::disableExtraShaders();

	ssrscenefbo.bind();
	renderer->clearScreen(true, true);
}

void gCanvas::capturessrcameramatrices(gCamera& camera) {
	ssrcaptureprojection = renderer->getProjectionMatrix();
	ssrcaptureview = renderer->getViewMatrix();
	ssrcapturenearclip = camera.getNearClip();
	ssrcapturefarclip = camera.getFarClip();
}

void gCanvas::endssrcapture() {
	ssrscenefbo.unbind();

	renderer->bindTexture(ssrscenefbo.getTextureId());
	renderer->generateMipMap();

	gRenderObject::enableExtraShaders();
}

void gCanvas::beginssrreflectionpass(gSkybox* skybox) {
	int previndex = 1 - ssrcurrentreflectionindex;

	ssrshader.use();
	ssrshader.setInt("sspass", 0);
	ssrshader.setInt("validhistory", ssrhasvalidhistory ? 1 : 0);
	ssrshader.setMat4("projection", ssrcaptureprojection);
	ssrshader.setMat4("invview", glm::inverse(ssrcaptureview));
	ssrshader.setMat4("previousviewprojection", ssrpreviousviewprojection);
	ssrshader.setVec2("screensize", glm::vec2(ssrscenefbo.getWidth(), ssrscenefbo.getHeight()));
	ssrshader.setFloat("nearclip", ssrcapturenearclip);
	ssrshader.setFloat("farclip", ssrcapturefarclip);
	ssrshader.setFloat("reflectivity", ssrreflectivity);
	ssrshader.setFloat("fresnelbias", ssrfresnelbias);
	ssrshader.setFloat("fresnelpower", ssrfresnelpower);
	ssrshader.setVec3("fallbackcolor", ssrfallbackcolor);
	ssrshader.setFloat("ditherphase", float(ssrframeindex % 64) * 0.6180339887f);
	ssrframeindex++;

	ssrshader.setInt("scenecolor", 8);
	renderer->activateTexture(8);
	renderer->bindTexture(ssrscenefbo.getTextureId());

	ssrshader.setInt("scenedepth", 9);
	renderer->activateTexture(9);
	renderer->bindTexture(ssrscenefbo.getDepthTextureId());

	ssrshader.setInt("hasskymap", skybox != nullptr ? 1 : 0);
	if(skybox != nullptr) {
		ssrshader.setInt("skymap", 10);
		renderer->bindSkyTexture(skybox->getTextureId(), GL_TEXTURE0 + 10);
	}

	ssrshader.setInt("reflectionbuffer", 11);
	renderer->activateTexture(11);
	renderer->bindTexture(ssrreflectionfbo[previndex].getTextureId());

	renderer->disableAlphaBlending();

	ssrreflectionfbo[ssrcurrentreflectionindex].bind();
	renderer->clearScreen(true, true);

	renderer->enableDepthTestEqual();
}

void gCanvas::endssrreflectionpass() {
	renderer->setDepthTestFunc(gRenderer::DEPTHTESTTYPE_LESS);

	ssrreflectionfbo[ssrcurrentreflectionindex].unbind();
	renderer->enableAlphaBlending();

	ssrpreviousviewprojection = ssrcaptureprojection * ssrcaptureview;
	ssrhasvalidhistory = true;
	ssrcurrentreflectionindex = 1 - ssrcurrentreflectionindex;
}

void gCanvas::beginssrcomposite() {
	ssrshader.use();
	ssrshader.setInt("sspass", 1);
	ssrshader.setVec2("screensize", glm::vec2(ssrscenefbo.getWidth(), ssrscenefbo.getHeight()));

	ssrshader.setInt("scenecolor", 8);
	renderer->activateTexture(8);
	renderer->bindTexture(ssrscenefbo.getTextureId());

	ssrshader.setInt("reflectionbuffer", 11);
	renderer->activateTexture(11);
	renderer->bindTexture(ssrreflectionfbo[1 - ssrcurrentreflectionindex].getTextureId());

	renderer->enableDepthTestEqual();
}

void gCanvas::endssrcomposite() {
	renderer->setDepthTestFunc(gRenderer::DEPTHTESTTYPE_LESS);
}

glm::vec3 cubefacedirection(int face, float s, float t) {
	switch(face) {
		case 0: return glm::normalize(glm::vec3(1.0f, -t, -s));
		case 1: return glm::normalize(glm::vec3(-1.0f, -t, s));
		case 2: return glm::normalize(glm::vec3(s, 1.0f, t));
		case 3: return glm::normalize(glm::vec3(s, -1.0f, -t));
		case 4: return glm::normalize(glm::vec3(s, -t, 1.0f));
		default: return glm::normalize(glm::vec3(-s, -t, -1.0f));
	}
}

glm::vec3 skycolorat(const glm::vec3& dir, const glm::vec3& sundir) {
	float elevation = dir.y;
	glm::vec2 dirxz(dir.x, dir.z);
	glm::vec2 sunxz(sundir.x, sundir.z);
	float dirxzlen = glm::length(dirxz);
	float sunxzlen = glm::length(sunxz);
	float az = (dirxzlen > 0.0001f && sunxzlen > 0.0001f) ? glm::dot(dirxz / dirxzlen, sunxz / sunxzlen) : 0.0f;

	glm::vec3 zenithcolor(0.06f, 0.08f, 0.20f);
	glm::vec3 sunsidehorizon(1.0f, 0.55f, 0.28f);
	glm::vec3 antisolarhorizon(0.30f, 0.26f, 0.42f);
	glm::vec3 horizoncolor = glm::mix(antisolarhorizon, sunsidehorizon, az * 0.5f + 0.5f);

	float elevationt = glm::clamp((elevation + 0.05f) / 0.65f, 0.0f, 1.0f);
	elevationt = elevationt * elevationt * (3.0f - 2.0f * elevationt);
	glm::vec3 color = glm::mix(horizoncolor, zenithcolor, elevationt);

	float groundt = glm::clamp(-elevation / 0.2f, 0.0f, 1.0f);
	glm::vec3 groundhaze(0.12f, 0.12f, 0.15f);
	color = glm::mix(color, groundhaze, groundt);

	float sunangledeg = glm::degrees(acosf(glm::clamp(glm::dot(dir, sundir), -1.0f, 1.0f)));
	float suncore = powf(glm::clamp(1.0f - sunangledeg / 1.1f, 0.0f, 1.0f), 2.0f);
	float sunhalo = expf(-sunangledeg / 5.5f) * 0.55f;
	glm::vec3 sunglow = glm::vec3(1.0f, 0.86f, 0.62f) * suncore + glm::vec3(1.0f, 0.55f, 0.24f) * sunhalo;
	color += sunglow;

	return glm::clamp(color, 0.0f, 1.0f);
}

void generatesunsetskybox(gSkybox& sky, const glm::vec3& sundir, int facesize) {
	std::array<int, 6> widths{};
	std::array<int, 6> heights{};
	std::array<void*, 6> facedata{};
	std::array<bool, 6> ishdr{};

	for(int face = 0; face < 6; face++) {
		unsigned char* pixels = new unsigned char[facesize * facesize * 3];
		for(int y = 0; y < facesize; y++) {
			float t = 2.0f * ((y + 0.5f) / facesize) - 1.0f;
			for(int x = 0; x < facesize; x++) {
				float s = 2.0f * ((x + 0.5f) / facesize) - 1.0f;
				glm::vec3 dir = cubefacedirection(face, s, t);
				glm::vec3 color = skycolorat(dir, sundir);
				int idx = (y * facesize + x) * 3;
				pixels[idx + 0] = (unsigned char) (color.r * 255.0f);
				pixels[idx + 1] = (unsigned char) (color.g * 255.0f);
				pixels[idx + 2] = (unsigned char) (color.b * 255.0f);
			}
		}
		widths[face] = facesize;
		heights[face] = facesize;
		facedata[face] = pixels;
		ishdr[face] = false;
	}

	sky.loadFromData(widths, heights, facedata, ishdr);

	for(int face = 0; face < 6; face++) delete[] (unsigned char*) facedata[face];
}

void generateflattexture(gTexture& texture, unsigned char r, unsigned char g, unsigned char b, int size) {
	unsigned char* pixels = new unsigned char[size * size * 3];
	for(int i = 0; i < size * size; i++) {
		pixels[i * 3 + 0] = r;
		pixels[i * 3 + 1] = g;
		pixels[i * 3 + 2] = b;
	}
	texture.setData(pixels, size, size, 3);
}

struct showcasepiece {
	const char* path;
	float x, y, z;
	float scale;
	float yawdeg;
	bool tintuntexturedmeshes;
	unsigned char tintr, tintg, tintb;
};

// y keeps each model's own lowest vertex resting on the floor, since the kit's models are not uniformly centered
const showcasepiece showcasepieces[] = {
	{"ModularSciFiKit/OBJ/Platforms/Door_Frame_Square.obj", 0.0f, 0.013425f, -7.0f, 1.0f, 0.0f, false, 0, 0, 0},
	{"ModularSciFiKit/OBJ/Columns/Column_Astra.obj", -3.6f, 0.006465f, -7.0f, 1.0f, 0.0f, false, 0, 0, 0},
	{"ModularSciFiKit/OBJ/Columns/Column_Astra.obj", 3.6f, 0.006465f, -7.0f, 1.0f, 0.0f, false, 0, 0, 0},
	{"ModularSciFiKit/OBJ/Columns/Column_Hollow.obj", -6.0f, 0.0f, -3.5f, 1.0f, 0.0f, false, 0, 0, 0},
	{"ModularSciFiKit/OBJ/Columns/Column_Round.obj", 6.0f, 0.0f, -3.5f, 1.0f, 0.0f, false, 0, 0, 0},
	{"ModularSciFiKit/OBJ/Props/Prop_Crate3.obj", -2.0f, 0.5f, 3.0f, 1.0f, 15.0f, false, 0, 0, 0},
	{"ModularSciFiKit/OBJ/Props/Prop_Crate4.obj", -1.0f, 0.560435f, 3.6f, 1.0f, -25.0f, false, 0, 0, 0},
	{"ModularSciFiKit/OBJ/Props/Prop_Computer.obj", 1.5f, 0.001192f, 3.2f, 1.0f, -10.0f, true, 30, 30, 34},
	{"ModularSciFiKit/OBJ/Props/Prop_Chest.obj", 2.6f, -0.003236f, 3.8f, 1.0f, 20.0f, false, 0, 0, 0},
};
const int showcasepiecenum = sizeof(showcasepieces) / sizeof(showcasepieces[0]);

// obj materials with no texture map default to flat white in this engine, so recolor them by hand
void tintuntexturedmeshes(gModel& model, unsigned char r, unsigned char g, unsigned char b) {
	for(int i = 0; i < model.getMeshNum(); i++) {
		gMaterial* material = model.getMeshPtr(i)->getMaterial();
		if(material->isDiffuseMapEnabled() || material->isAlbedoMapEnabled()) continue;
		material->setAmbientColor(r, g, b);
		material->setDiffuseColor(r, g, b);
	}
}

gCanvas::gCanvas(gApp* root) : gBaseCanvas(root), skyfill(gLight::LIGHTTYPE_AMBIENT) {
	this->root = root;
}

gCanvas::~gCanvas() {
}

void gCanvas::setupsky() {
	sundirection = glm::normalize(glm::vec3(0.2f, 0.32f, -0.92f));
	generatesunsetskybox(sky, sundirection, 512);
}

void gCanvas::setupfloor() {
	generateflattexture(flooralbedo, 22, 24, 30, 4);
	generateflattexture(floorroughness, 40, 40, 40, 4);
	generateflattexture(floormetalness, 200, 200, 200, 4);

	floortile.setScale(10.0f, 0.1f, 9.0f);
	floortile.setPosition(0.0f, -0.1f, -1.5f);
	floortile.getMaterial()->setAlbedoMap(&flooralbedo);
	floortile.getMaterial()->setRoughnessMap(&floorroughness);
	floortile.getMaterial()->setMetalnessMap(&floormetalness);
}

void gCanvas::setupsphere() {
	generateflattexture(sphereroughness, 30, 30, 30, 4);
	generateflattexture(spheremetalness, 235, 235, 235, 4);

	showcasesphere.setScale(1.3f);
	showcasesphere.setPosition(0.0f, 1.42f, -1.5f);
	showcasesphere.getMaterial()->setRoughnessMap(&sphereroughness);
	showcasesphere.getMaterial()->setMetalnessMap(&spheremetalness);
}

void gCanvas::setupshowcase() {
	showcasemodels.reserve(showcasepiecenum);
	for(int i = 0; i < showcasepiecenum; i++) {
		const showcasepiece& piece = showcasepieces[i];
		showcasemodels.emplace_back();
		gModel& model = showcasemodels.back();
		model.loadModel(piece.path);
		model.setScale(piece.scale);
		model.setPosition(piece.x, piece.y, piece.z);
		model.rotateDeg(piece.yawdeg, 0.0f, 1.0f, 0.0f);
		if(piece.tintuntexturedmeshes) tintuntexturedmeshes(model, piece.tintr, piece.tintg, piece.tintb);
	}
}

void gCanvas::drawscene() {
	sky.draw();
	floortile.draw();
	showcasesphere.draw();
	for(gModel& model : showcasemodels) model.draw();
}

void gCanvas::setup() {
	camera.setNearClip(0.1f);
	camera.setFarClip(300.0f);
	camera.setPosition(0.0f, 4.3f, 15.5f);

	camcontroller.setCamera(&camera);
	camcontroller.setMoveSpeed(5.0f);
	camcontroller.setEnabled(true);
	inputmanager->getPlayer(camcontroller.getPlayerIndex())->rebindAxis(gCameraController::AXIS_ELEVATION, G_KEY_Q, G_KEY_E);

	setupsky();

	sun.setDiffuseColor(255, 150, 82);
	sun.setSpecularColor(255, 214, 176);
	sun.setPosition(sundirection * 55.0f);

	skyfill.setAmbientColor(58, 54, 80);

	setupfloor();
	setupsphere();
	setupshowcase();

	ssrreflectivity = 0.85f;
	ssrfresnelbias = 0.5f;
	ssrfresnelpower = 3.0f;
	ssrfallbackcolor = glm::vec3(0.35f, 0.37f, 0.42f);
	setupssr();
	floortile.getMaterial()->addShader(&ssrshader);
	showcasesphere.getMaterial()->addShader(&ssrshader);
}

void gCanvas::update() {
	camcontroller.update();
}

void gCanvas::draw() {
	enableDepthTest();
	enableAlphaBlending();

	beginssrcapture();
	camera.begin();
	capturessrcameramatrices(camera);
	sun.enable();
	skyfill.enable();
	drawscene();
	skyfill.disable();
	sun.disable();
	camera.end();
	endssrcapture();

	camera.begin();
	beginssrreflectionpass(&sky);
	floortile.draw();
	showcasesphere.draw();
	endssrreflectionpass();
	camera.end();

	camera.begin();
	sun.enable();
	skyfill.enable();
	sky.draw();
	for(gModel& model : showcasemodels) model.draw();
	beginssrcomposite();
	floortile.draw();
	endssrcomposite();
	beginssrcomposite();
	showcasesphere.draw();
	endssrcomposite();
	skyfill.disable();
	sun.disable();
	camera.end();

	disableAlphaBlending();
	disableDepthTest();
}

void gCanvas::keyPressed(int key) {
}

void gCanvas::keyReleased(int key) {
}

void gCanvas::charPressed(unsigned int codepoint) {
}

void gCanvas::mouseMoved(int x, int y) {
}

void gCanvas::mouseDragged(int x, int y, int button) {
}

void gCanvas::mousePressed(int x, int y, int button) {
}

void gCanvas::mouseReleased(int x, int y, int button) {
}

void gCanvas::mouseScrolled(int x, int y) {
}

void gCanvas::mouseEntered() {
}

void gCanvas::mouseExited() {
}

void gCanvas::windowResized(int w, int h) {
	resizessr(getScreenWidth(), getScreenHeight());
}

void gCanvas::showNotify() {
}

void gCanvas::hideNotify() {
}
