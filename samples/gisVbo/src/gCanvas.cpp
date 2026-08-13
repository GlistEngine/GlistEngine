/*
* gCanvas.cpp
*
*  Created on: May 6, 2020
*      Author: Noyan Culum
*/


#include "gCanvas.h"
#include "gRenderObject.h"
#include "gShader.h"

gCanvas::gCanvas(gBaseApp *root) : gBaseCanvas(root) {
}

gCanvas::~gCanvas() {
}

void gCanvas::setup() {
    verts1[0].position = glm::vec3(-0.75f, -0.5f, 0.0f);
    verts1[1].position = glm::vec3( 0.25f, -0.5f, 0.0f);
    verts1[2].position = glm::vec3(-0.25f,  0.5f, 0.0f);

    verts1[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
    verts1[1].color = glm::vec3(1.0f, 1.0f, 1.0f);
    verts1[2].color = glm::vec3(1.0f, 1.0f, 1.0f);

    verts1[0].normal = glm::vec3(0.0f, 0.0f, 1.0f);
    verts1[1].normal = glm::vec3(0.0f, 0.0f, 1.0f);
    verts1[2].normal = glm::vec3(0.0f, 0.0f, 1.0f);

    vbo1.setVertexData(&verts1[0], 3, 3);

    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    verts2[0].position = glm::vec3( 0.25f, -0.5f, 0.0f);
    verts2[1].position = glm::vec3( 0.75f,  0.5f, 0.0f);
    verts2[2].position = glm::vec3(-0.25f,  0.5f, 0.0f);

    verts2[0].color = glm::vec3(1.0f, 1.0f, 1.0f);
    verts2[1].color = glm::vec3(1.0f, 1.0f, 1.0f);
    verts2[2].color = glm::vec3(1.0f, 1.0f, 1.0f);

    verts2[0].normal = glm::vec3(0.0f, 0.0f, 1.0f);
    verts2[1].normal = glm::vec3(0.0f, 0.0f, 1.0f);
    verts2[2].normal = glm::vec3(0.0f, 0.0f, 1.0f);

    vbo2.setVertexData(&verts2[0], 3, 3);

    light = gLight(gLight::LIGHTTYPE_AMBIENT);
    light.setAmbientColor(255, 255, 255, 255);

    auto* renderer = gRenderObject::getRenderer();

    renderer->setProjectionMatrix(glm::mat4(1.0f));
    renderer->setViewMatrix(glm::mat4(1.0f));
    renderer->setCameraPosition(glm::vec3(0.0f));
}

void gCanvas::update() {
}

void gCanvas::draw() {

    auto* renderer = gRenderObject::getRenderer();

    light.enable();

    gRenderer::gMeshSurface red;
    red.ambient = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    red.diffuse = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    red.specular = glm::vec4(1.0f);
    red.shininess = 0.5f;

    renderer->drawVbo(
        vbo1,
        glm::mat4(1.0f),
        red
    );


    gRenderer::gMeshSurface green;
    green.ambient = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    green.diffuse = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    green.specular = glm::vec4(1.0f);
    green.shininess = 0.5f;

    renderer->drawVbo(
        vbo2,
        glm::mat4(1.0f),
        green
    );

    light.disable();
}

void gCanvas::keyPressed(int key) {
//	gLogi("gCanvas") << "keyPressed:" << key;
}

void gCanvas::keyReleased(int key) {
//	gLogi("gCanvas") << "keyReleased:" << key;
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

void gCanvas::mouseEntered() {

}

void gCanvas::mouseExited() {

}

void gCanvas::showNotify() {

}

void gCanvas::hideNotify() {

}

