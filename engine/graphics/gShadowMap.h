/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GRAPHICS_GSHADOWMAP_H_
#define GRAPHICS_GSHADOWMAP_H_

#include "gRenderObject.h"
#include "gFbo.h"
#include "gLight.h"
#include "gCamera.h"


class gShadowMap : public gRenderObject {
public:
	gShadowMap();
	virtual ~gShadowMap();

	void allocate(gLight* light, gCamera* camera, int width = 4096, int height = 4096);
	bool isAllocated() const;
	int getWidth() const;
	int getHeight() const;

	void update();

	void setLight(gLight* light);
	void setCamera(gCamera* camera);
	gLight* getLight() const;
	gCamera* getCamera() const;

	void activate();
	void deactivate();
	bool isActivated() const;

	void enable();
	void disable();
	bool isEnabled() const;

	void setLightProjection(glm::mat4 lightProjection);
	void setLightProjection(float leftx, float rightx, float fronty, float backy, float nearz, float farz);
	void setLightView(glm::mat4 lightView);
	glm::mat4 getLightProjection() const;
	glm::mat4 getLightView() const;
	glm::mat4 getLightMatrix() const;

	gFbo& getDepthFbo();


private:
	bool isallocated, isactivated, isenabled;
	gLight* light;
	gCamera* camera;
	glm::mat4 lightprojection, lightview, lightmatrix;
	glm::vec3 lightposition;
	gFbo depthfbo;
	int width, height;
	int shadowmaptextureslot;
	bool updateshadows;
};

#endif /* GRAPHICS_GSHADOWMAP_H_ */
