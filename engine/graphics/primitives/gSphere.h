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

#ifndef GRAPHICS_PRIMITIVES_GSPHERE_H_
#define GRAPHICS_PRIMITIVES_GSPHERE_H_

#include "gMesh.h"

class gSphere: public gMesh {
public:
	gSphere(int xSegmentNum = 64, int ySegmentNum = 64, bool isFilled = true);
	~gSphere() override;
};

#endif /* GRAPHICS_PRIMITIVES_GSPHERE_H_ */
