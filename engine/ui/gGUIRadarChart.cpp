/*
 * gGUIRadarChart.cpp
 *
 *  Created on: Jul 27, 2022
 *      Author: emirhantasdeviren
 */

#include "gGUIRadarChart.h"

gGUIRadarChart::gGUIRadarChart() :
    datasets{{std::vector<float>(3), gColor::RED}},
    vertices(3),
    grids{std::vector<gVertex>(3), std::vector<gVertex>(3), std::vector<gVertex>(3)},
    is_grid_enabled{true},
    labels(3),
    min{0.0f},
    max{1.0f}
{}

gGUIRadarChart::~gGUIRadarChart() {}

void gGUIRadarChart::draw() {
    gColor *old_color = renderer->getColor();
    this->drawBase();
    this->drawChart();
    renderer->setColor(old_color);
}

void gGUIRadarChart::update() {
    this->calcVertices();
}

void gGUIRadarChart::setValue(std::size_t i, std::size_t j, float value) {
    this->datasets[i].values[j] = value;
}

void gGUIRadarChart::setColor(std::size_t i, const gColor &color) {
    this->datasets[i].color = color;
}

void gGUIRadarChart::setLabel(std::size_t i, const std::string &label) {
    this->labels[i] = label;
}

void gGUIRadarChart::setGrid(bool enable) {
    this->is_grid_enabled = enable;
}

void gGUIRadarChart::setGridSize(std::size_t new_size) {
    this->grids.resize(new_size);
    for (std::vector<gVertex> &grid : this->grids) {
        grid.resize(this->vertices.size());
    }
}

void gGUIRadarChart::setNumDataset(std::size_t new_size) {
    this->datasets.resize(new_size);
    for (gDataset &dataset : this->datasets) {
        dataset.values.resize(this->vertices.size());
    }
}

void gGUIRadarChart::setNumAxes(std::size_t new_size) {
    if (new_size >= 3) {
        for (gDataset &dataset : this->datasets) {
            dataset.values.resize(new_size);
        }
        for (std::vector<gVertex> &grid : this->grids) {
            grid.resize(new_size);
        }
        this->vertices.resize(new_size);
        this->labels.resize(new_size);
    }
}

void gGUIRadarChart::setMin(float min) {
    this->min = min;
}

void gGUIRadarChart::setMax(float max) {
    this->max = max;
}

void gGUIRadarChart::calcVertices() {
    int width = this->right - this->left;
    int height = this->bottom - this->top;
    int min_length = std::min(width, height);
    int min_pad = 10;

    std::size_t n = this->vertices.size();

    float circumradius = static_cast<float>((min_length - 2 * min_pad) / 2);
    float exterior_angle = 2.0f * PI / static_cast<float>(n);
    float ratio = 1.0f / static_cast<float>(this->grids.size() + 1);

    this->center.position.x = static_cast<float>(this->left + width / 2);
    this->center.position.y = static_cast<float>(this->top + height / 2);

    for (std::size_t i = 0; i < n; i++) {
        this->vertices[i].position.x = this->center.position.x
            - circumradius * std::sin(static_cast<float>(i) * exterior_angle);
        this->vertices[i].position.y = this->center.position.y
            - circumradius * std::cos(static_cast<float>(i) * exterior_angle);

        for (std::size_t j = 0; j < this->grids.size(); j++) {
            this->grids[j][i].position.x = this->center.position.x
                - ratio * static_cast<float>(j + 1)
                * circumradius * std::sin(static_cast<float>(i) * exterior_angle);
           
            this->grids[j][i].position.y = this->center.position.y
                - ratio * static_cast<float>(j + 1)
                * circumradius * std::cos(static_cast<float>(i) * exterior_angle);
        }
    }
}

void gGUIRadarChart::drawBase() {
    gColor *old_color = renderer->getColor();
    std::size_t n = this->vertices.size();

    for (std::size_t i = 0; i < n; i++) {
        renderer->setColor(this->buttoncolor);
        gDrawLine(
            this->center.position.x,
            this->center.position.y,
            this->vertices[i].position.x,
            this->vertices[i].position.y
        );

        if (i != n - 1) {
            gDrawLine(
                this->vertices[i].position.x,
                this->vertices[i].position.y,
                this->vertices[i + 1].position.x,
                this->vertices[i + 1].position.y
            );
        } else {
            gDrawLine(
                this->vertices[i].position.x,
                this->vertices[i].position.y,
                this->vertices[0].position.x,
                this->vertices[0].position.y
            );
        }

        if (this->is_grid_enabled) {
            for (const std::vector<gVertex> &grid : this->grids) {
                if (i != grid.size() - 1) {
                    gDrawLine(
                        grid[i].position.x,
                        grid[i].position.y,
                        grid[i + 1].position.x,
                        grid[i + 1].position.y
                    );
                } else {
                    gDrawLine(
                        grid[i].position.x,
                        grid[i].position.y,
                        grid[0].position.x,
                        grid[0].position.y
                    );
                }
            }
        }

        renderer->setColor(this->fontcolor);
        if ((i > 0 && i < n / 2) || (n % 2 != 0 && i == n / 2)) {
            float text_width = this->font->getStringWidth(this->labels[i]);

            this->font->drawText(
                this->labels[i],
                this->vertices[i].position.x - text_width,
                this->vertices[i].position.y
            );
        } else if (i == 0 || (n % 2 == 0 && i == n / 2)) {
            float text_width = this->font->getStringWidth(this->labels[i]);

            this->font->drawText(
                this->labels[i],
                this->vertices[i].position.x - text_width / 2.0f,
                this->vertices[i].position.y
            );
        } else {
            this->font->drawText(
                this->labels[i],
                this->vertices[i].position.x,
                this->vertices[i].position.y
            );
        }
    }
    renderer->setColor(old_color);
}

void gGUIRadarChart::drawChart() {
    for (const gDataset &dataset : this->datasets) {
        gColor *old_color = renderer->getColor();
        renderer->setColor(dataset.color);

        std::vector<float> vertices(this->vertices.size() * 2);
        float circumradius = std::sqrt(
            std::pow(this->center.position.x - this->vertices[0].position.x, 2.0f)
                + std::pow(this->center.position.y - this->vertices[0].position.y, 2.0f)
        );

        for (std::size_t i = 0; i < vertices.size(); i += 2) {
            float normalized =
                (dataset.values[i / 2] - this->min) / (this->max - this->min);
            float distance = normalized * circumradius;

            float rad = std::atan2(
                this->center.position.y - this->vertices[i / 2].position.y,
                this->center.position.x - this->vertices[i / 2].position.x
            );

            float x = this->center.position.x - distance * std::cos(rad);
            float y = this->center.position.y - distance * std::sin(rad);
            vertices[i] = x;
            vertices[i + 1] = y;

            if (i >= 2) {
                gDrawLine(vertices[i - 2], vertices[i - 1], vertices[i], vertices[i + 1]);
                if (i == vertices.size() - 2) {
                    gDrawLine(vertices[i], vertices[i + 1], vertices[0], vertices[1]);
                }
            }
        }
        renderer->setColor(old_color);
    }
}
