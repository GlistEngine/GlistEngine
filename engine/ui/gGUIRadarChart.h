/*
 * gGUIRadarChart.h
 *
 *  Created on: Jul 27, 2022
 *      Author: emirhantasdeviren
 */

#ifndef UI_GGUIRADARCHART_H_
#define UI_GGUIRADARCHART_H_

#include "gGUIControl.h"

/*
 * A radar chart is a graphical method for displaying multivariate data.
 */
class gGUIRadarChart: public gGUIControl {
public:
    gGUIRadarChart();
    virtual ~gGUIRadarChart();

    void draw() override;
    void update() override;

    /*
     * Sets specified quantitative variable of the specified dataset.
     *
     * @param i     Dataset index.
     * @param j     Axis index.
     * @param value Quantitative data.
     */
    void setValue(std::size_t i, std::size_t j, float value);

    /*
     * Sets color of given dataset.
     *
     * @param i      Dataset index.
     * @param color  An RGB color.
     */
    void setColor(std::size_t i, const gColor &color);

    /*
     * Sets label of specified axis.
     *
     * @param i     Axis index.
     * @param label A text label.
     */
    void setLabel(std::size_t i, const std::string &label);

    /*
     * Sets grid visibility.
     *
     * @param enable Grid visibility state.
     */
    void setGrid(bool enable);

    /*
     * Sets grid size, number of polygons inside chart.
     *
     * @param new_size Grid size.
     */
    void setGridSize(std::size_t new_size);

    /*
     * Sets number of datasets.
     *
     * @param new_size Number of dataset.
     */
    void setNumDataset(std::size_t new_size);

    /*
     * Sets number of quantitative variables, number of axes.
     *
     * @param new_size Number of variables.
     */
    void setNumAxes(std::size_t new_size);

    /*
     * Sets minimum value of datasets. Defaults to 0.
     *
     * @param min A minimum value.
     */
    void setMin(float min);

    /*
     * Sets maximum value of datasets. Defaults to 1.
     *
     * @param max A maximum value.
     */
    void setMax(float max);

private:
    struct gDataset {
        std::vector<float> values;
        gColor color;
    };

    void drawBase();
    void drawChart();

    void calcVertices();

    std::vector<gDataset> datasets;
    std::vector<gVertex> vertices;
    std::vector<std::vector<gVertex>> grids;
    bool is_grid_enabled;
    std::vector<std::string> labels;
    gVertex center;
    float min;
    float max;
};

#endif /* UI_GGUIRADARCHART_H_ */
