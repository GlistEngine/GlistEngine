/*
 * gGUIRadarChart.h
 *
 *  Created on: Jul 27, 2022
 *      Author: emirhantasdeviren
 */

#ifndef UI_GGUIRADARCHART_H_
#define UI_GGUIRADARCHART_H_

#include "gGUIControl.h"

class gGUIRadarChart: public gGUIControl {
public:
	gGUIRadarChart();
	virtual ~gGUIRadarChart();

	void draw() override;
    void update() override;

    void setVar(std::size_t i, std::size_t j, float var);
    void setColor(std::size_t i, const gColor &color);

    void setNumDataset(std::size_t new_size);
    void setNumVar(std::size_t new_size);

private:
    struct gDataset {
        std::vector<float> variables;
        gColor color;
    };

    void drawBase();
    void drawChart();

    void calcVertices();

	void calcTriangle(int length);
    void calcSquare(int length);
    void calcPentagon(int length);

	std::vector<gDataset> datasets;
    std::vector<gVertex> vertices;
    gVertex center;
	float max;
};

#endif /* UI_GGUIRADARCHART_H_ */
