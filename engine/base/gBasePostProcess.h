/*
 * gBasePostProcess.h
 *
 *  Created on: 29 Kas 2021
 *      Author: kayra
 */

#ifndef BASE_GBASEPOSTPROCESS_H_
#define BASE_GBASEPOSTPROCESS_H_

#include "gObject.h"
#include "gFbo.h"

class gBasePostProcess : public gRenderObject {
public:
	gBasePostProcess();
	virtual ~gBasePostProcess();

	virtual void use() = 0;

protected:
	gShader* shader;
};

#endif /* BASE_GBASEPOSTPROCESS_H_ */
