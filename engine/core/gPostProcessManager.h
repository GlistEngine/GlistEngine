/*
 * gPostProcessManager.h
 *
 *  Created on: 29 Kas 2021
 *      Author: kayra
 */

#ifndef CORE_GPOSTPROCESSMANAGER_H_
#define CORE_GPOSTPROCESSMANAGER_H_

#include "gFbo.h"
#include "gRenderObject.h"
#include "gBasePostProcess.h"

/**
 * To use this class, simply add the instance of this class to your
 * Canvas's header file by: "gPostProcessManager ppm;". Then in your setup,
 * first call setDimensions to specify the area of which you want your
 * post-processing to happen. (Remember that currently it is only possible to
 * post-process on the center of the screen. So it's recommended that you
 * specify the width and height as your game's original width and height.)
 *
 * Then, to add post-process effects to your manager, call addEffect(). As a
 * parameter, you can use one of the default post-process effects that comes
 * with the engine. Such as; gBlur, gGrayscale, gInversion etc. To actually
 * add the effect, you can call the function with: "ppm.addEffect(new gBlur())"
 * You can change the "new gBlur()" part with any post-process that you like.
 * To use the effect, of course you have to include it first.
 *
 * After setting the dimensions and adding your effects inside the setup (or
 * any relevant work-once function), you can then use enable() function before
 * the draw call of an object that's going to get affected by the post-process
 * effect. After the draw calls of the ones that needs to get affected by post-
 * process effects, simply call disable().
 */
class gPostProcessManager : public gRenderObject {
public:
	gPostProcessManager();
	virtual ~gPostProcessManager();

	/**
	 * Set the dimensions of which the post-process framebuffer will take
	 * effect.
	 *
	 * @param width The width of the post-process framebuffer.
	 * @param height The height of the post-process framebuffer.
	 */
	void setDimensions(int width, int height);

	/**
	 * Add a post-process effect to the manager by dynamically instancing it.
	 *
	 * @param effect The effect instance that has been created with "new"
	 * keyword.
	 */
	void addEffect(gBasePostProcess *effect);

	/**
	 * Enables the post process manager to direct draw calls into the
	 * associated framebuffers. Call this before the draw calls that you want
	 * to apply post-process.
	 */
	void enable();

	/**
	 * Finalizes the framebuffer textures and draws them on the default
	 * framebuffer. Call this after enable(), and after the draw calls that
	 * you want to apply post process.
	 */
	void disable();
private:
	static const int fbocount;
	std::vector<gBasePostProcess*> effects;

	gFbo* fbos;
	int fbotoread, fbotowrite, lastwrittenfbo;

	int width, height;
};

#endif /* CORE_GPOSTPROCESSMANAGER_H_ */
