/*
 * gFireworksParticles.h
 *
 *  Created on: 26 Tem 2023
 *      Author: Batuhan Yigit
 */

#ifndef GRAPHICS_PARTICLES_GFIREWORKSPARTICLES_H_
#define GRAPHICS_PARTICLES_GFIREWORKSPARTICLES_H_

#include "gBaseParticles.h"
#include "gImage.h"

class gFireworksParticles: public gBaseParticles {
public:
	gFireworksParticles();
	virtual ~gFireworksParticles();

	void init(int particleNum);
	void resetParticle(int particleNo);

	void setPosition(float x, float y, float z);

	void drawTex();

private:
	std::string getFireworkImageData();
	gImage tex;
	glm::vec3 pos;
	float rainbowcolors[7][3] = {{0.9, 0.6, 0.9},
								{0.97, 0.21, 0.21},
								{0.99, 0.76, 0.99},
								{0.98, 0.99, 0.21},
								{0.38, 0.99, 0.94},
								{0.32, 0.55, 0.99},
								{0.64, 0.44, 0.99}};
	int burstsize;
	std::vector<int> resetedparticles;
};

#endif /* GRAPHICS_PARTICLES_GFIREWORKSPARTICLES_H_ */
