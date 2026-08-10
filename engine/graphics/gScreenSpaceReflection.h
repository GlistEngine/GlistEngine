/*
* gScreenSpaceReflection.h
*
*  Created on: August 5, 2026
*      Author: Hasan Cem Eren
*/

#ifndef GSCREENSPACEREFLECTION_H_
#define GSCREENSPACEREFLECTION_H_

#include "gFbo.h"
#include "gShader.h"
#include "gCamera.h"
#include "gSkybox.h"
#include "gMesh.h"

class gScreenSpaceReflection {
public:
	gScreenSpaceReflection();
	~gScreenSpaceReflection();

	void setup();
	void resize(int width, int height);

	void attachTo(gMesh& mesh);

	void beginCapture();
	void captureCameraMatrices(gCamera& camera);
	void endCapture();

	void beginReflectionPass(gSkybox* skybox = nullptr);
	void endReflectionPass();

	void beginComposite();
	void endComposite();

	void setReflectivity(float reflectivity);
	float getReflectivity() const;

	void setFresnelBias(float fresnelbias);
	float getFresnelBias() const;

	void setFresnelPower(float fresnelpower);
	float getFresnelPower() const;

	void setFallbackColor(const glm::vec3& color);
	glm::vec3 getFallbackColor() const;

private:
	gFbo scenefbo;
	gFbo reflectionfbo[2];
	int currentreflectionindex;
	bool hasvalidhistory;
	int frameindex;

	gShader ssrshader;

	glm::mat4 captureprojection;
	glm::mat4 captureview;
	glm::mat4 previousviewprojection;
	float capturenearclip;
	float capturefarclip;

	float reflectivity;
	float fresnelbias;
	float fresnelpower;
	glm::vec3 fallbackcolor;
};

#endif
