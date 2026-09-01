
#ifndef ENGINE_GRAPHICS_GDEFERREDLIGHT_H_
#define ENGINE_GRAPHICS_GDEFERREDLIGHT_H_

#include"gRenderObject.h"
#include"gShader.h"

// Forward declarations
class gLight;
class gCamera;
class gShadowMap;

class gDeferredLight : public gRenderObject{
public:
	gDeferredLight();
	virtual ~gDeferredLight();
	// Initializes deferred rendering
	void setup(int screenWidth, int screenHeight);

	// Starts geometry pass
	void enable();

    // Lighting Pass
	void renderLightingPass(gLight* light, gCamera* camera, gShadowMap* shadowmap);

	// End Deferred
	void disable();

	gShader* getGeomShader() {return geometryShader;}
	gShader* getLightShader() {return lightingShader;}

private:
	// Sets up G-Buffer textures
	void setupGBuffer();

	// Draws lighting quad
	void drawFullScreenQuad();

	unsigned int gBufferFBO;
	unsigned int positionTexture;
	unsigned int normalTexture;
	unsigned int albedoTexture;
	unsigned int rboDepth;
	unsigned int quadVAO;
	unsigned int quadVBO;

	int width;
	int height;

	gShader* geometryShader;
	gShader* lightingShader;
};

#endif /* ENGINE_GRAPHICS_GDEFERREDLIGHT_H_ */
