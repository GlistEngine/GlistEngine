/*
 * gBasePostProcess.h
 *
 *  Created on: 29 Kas 2021
 *      Author: kayra
 */

#ifndef BASE_GBASEPOSTPROCESS_H_
#define BASE_GBASEPOSTPROCESS_H_

#include "gRenderObject.h"
#include "gFbo.h"
#include "gShader.h"

class gBasePostProcess : public gRenderObject {
public:
	gBasePostProcess();
	virtual ~gBasePostProcess();

	virtual void use() = 0;
	gShader* getShader();

protected:
	virtual const std::string getVertSrc() = 0;
	virtual const std::string getFragSrc() = 0;
	gShader* shader;
};

#endif /* BASE_GBASEPOSTPROCESS_H_ */
