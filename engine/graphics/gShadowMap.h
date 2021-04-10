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


class gShadowMap : public gRenderObject {
public:
	gShadowMap();
	virtual ~gShadowMap();

	void allocate(int width, int height);
	bool isAllocated();
	int getWidth();
	int getHeight();

	void activate();
	void deactivate();
	bool isActivated();

	void enable();
	void disable();
	bool isEnabled();

	void setLightPosition(glm::vec3 lightPosition);
	void setLightProjection(glm::mat4 lightProjection);
	void setLightView(glm::mat4 lightView);
	glm::vec3 getLightPosition();
	glm::mat4 getLightProjection();
	glm::mat4 getLightView();
	glm::mat4 getLightMatrix();

	void setViewPosition(glm::vec3 viewPosition);
	glm::vec3 getViewPosition();

	gFbo& getDepthFbo();


private:
	bool isallocated, isactivated, isenabled;
	glm::mat4 lightprojection, lightview, lightmatrix;
	glm::vec3 lightposition;
	glm::vec3 viewposition;
	gFbo depthfbo;
	int width, height;
	int shadowmaptextureslot;
};

#endif /* GRAPHICS_GSHADOWMAP_H_ */
