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

/*
 * gCube.h
 *
 *  Created on: Sep 1, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_PRIMITIVES_GCUBE_H_
#define ENGINE_GRAPHICS_PRIMITIVES_GCUBE_H_

#include "gMesh.h"


class gBox : public gMesh {
public:
	gBox();
	~gBox() override;
};

#endif /* ENGINE_GRAPHICS_PRIMITIVES_GCUBE_H_ */
