/*
 * gTerrain.cpp
 *
 *  Created on: 24 Þub 2026
 *      Author: Engin Kutlu
 */

#include "gTerrain.h"
#include "gImage.h"

gTerrain::gTerrain() {
	// TODO Auto-generated constructor stub

}

gTerrain::~gTerrain() {
	// TODO Auto-generated destructor stub
}


void gTerrain::generateTerrain(const std::string& heightMapPath, int widthSegments, int depthSegments, float cellSize, float maxHeight)
{

	gImage img;

	if (!img.loadImage(heightMapPath)) {
		gLogi("gTerrain") << "Heightmap load failed\n";
	    return;
	}

	int imgwidth  = img.getWidth();
	int imgheight = img.getHeight();
	unsigned char* data = img.getData();

    const int vertcountx = widthSegments + 1;
    const int vertcountz = depthSegments + 1;

    std::vector<gVertex> vertices;
    vertices.reserve(vertcountx * vertcountz);

    std::vector<gIndex> indices;
    indices.reserve(widthSegments * depthSegments * 6);

    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    glm::vec3 tangent(1.0f, 0.0f, 0.0f);
    glm::vec3 bitangent(0.0f, 0.0f, 1.0f);

    // Vertex generation
    for (int z = 0; z < vertcountz; ++z)
    {
        for (int x = 0; x < vertcountx; ++x)
        {
            gVertex v;
            int imgx = static_cast<int>((float)x / widthSegments * (imgwidth - 1));
            int imgz = static_cast<int>((float)z / depthSegments * (imgheight - 1));
            unsigned char pixel = data[imgz * imgwidth + imgx];

            float heightvalue = (pixel / 255.0f) * maxHeight;

            v.position = glm::vec3(
                x * cellSize,
                heightvalue,
                z * cellSize
            );
            v.texcoords = glm::vec2(
                static_cast<float>(x) / widthSegments,
                static_cast<float>(z) / depthSegments
            );

            v.normal    = normal;
            v.tangent   = tangent;
            v.bitangent = bitangent;

            vertices.push_back(v);
        }
    }

    // Index generation
    for (int z = 0; z < depthSegments; ++z)
    {
        for (int x = 0; x < widthSegments; ++x)
        {
            gIndex i0 =  z * vertcountx + x;
            gIndex i1 =  i0 + 1;
            gIndex i2 = (z + 1) * vertcountx + x;
            gIndex i3 =  i2 + 1;

            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i1);

            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    setVertices(
        std::make_shared<std::vector<gVertex>>(std::move(vertices)),
        std::make_shared<std::vector<gIndex>>(std::move(indices))
    );
}
