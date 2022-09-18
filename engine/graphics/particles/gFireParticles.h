/*
 * gFireParticles.h
 *
 *  Created on: Aug 22, 2022
 *      Author: noyan
 */

#ifndef GRAPHICS_PARTICLES_GFIREPARTICLES_H_
#define GRAPHICS_PARTICLES_GFIREPARTICLES_H_

#include "gBaseParticles.h"
#include "gImage.h"


class gFireParticles: public gBaseParticles {
public:
	gFireParticles();
	virtual ~gFireParticles();

	void init(int particleNum);
	void resetParticle(int particleNo);

private:
	std::string getFlameImageData();
	gImage tex;
};

#endif /* GRAPHICS_PARTICLES_GFIREPARTICLES_H_ */
