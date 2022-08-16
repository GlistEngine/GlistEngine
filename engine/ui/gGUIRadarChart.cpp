/*
 * gGUIRadarChart.cpp
 *
 *  Created on: Jul 27, 2022
 *      Author: emirhantasdeviren
 */

#include "gGUIRadarChart.h"

gGUIRadarChart::gGUIRadarChart() :
    datasets{{{0.0f, 0.0f, 0.0f}, gColor::RED}},
    vertices(3),
    labels(3),
    max{1.0f}
{}

gGUIRadarChart::~gGUIRadarChart() {}

void gGUIRadarChart::draw() {
    gColor *old_color = this->renderer->getColor();
    this->drawBase();
    this->drawChart();
    this->renderer->setColor(old_color);
}

void gGUIRadarChart::update() {
    this->calcVertices();
}

void gGUIRadarChart::setVar(std::size_t i, std::size_t j, float var) {
    this->datasets[i].variables[j] = var;
}

void gGUIRadarChart::setColor(std::size_t i, const gColor &color) {
    this->datasets[i].color = color;
}

void gGUIRadarChart::setLabel(std::size_t i, const std::string &label) {
    this->labels[i] = label;
}

void gGUIRadarChart::setNumDataset(std::size_t new_size) {
    this->datasets.resize(new_size);
}

void gGUIRadarChart::setNumVar(std::size_t new_size) {
    if (new_size >= 3) {
        for (gDataset &dataset : this->datasets) {
            dataset.variables.resize(new_size);
        }
        this->vertices.resize(new_size);
        this->labels.resize(new_size);
    }
}

void gGUIRadarChart::calcVertices() {
    int width = this->right - this->left;
    int height = this->bottom - this->top;
    int min_length = std::min(width, height);
    int min_pad = 10;

    constexpr float pi = 3.14159265358979323846264338327950288f;
    std::size_t n = this->vertices.size();

    float circumradius = static_cast<float>((min_length - 2 * min_pad) / 2);
    float exterior_angle = 2.0f * pi / static_cast<float>(n);

    this->center.position.x = static_cast<float>(this->left + width / 2);
    this->center.position.y = static_cast<float>(this->top + height / 2);

    for (std::size_t i = 0; i < n; i++) {
        this->vertices[i].position.x = this->center.position.x
            - circumradius * std::sin(static_cast<float>(i) * exterior_angle);
        this->vertices[i].position.y = this->center.position.y
            - circumradius * std::cos(static_cast<float>(i) * exterior_angle);
    }
}

void gGUIRadarChart::drawBase() {
    gColor *old_color = this->renderer->getColor();

    for (std::size_t i = 0; i < this->vertices.size(); i++) {
        this->renderer->setColor(this->middlegroundcolor);
        gDrawLine(
            this->center.position.x,
            this->center.position.y,
            this->vertices[i].position.x,
            this->vertices[i].position.y
        );

        if (i != this->vertices.size() - 1) {
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

        this->renderer->setColor(this->fontcolor);
        if (this->center.position.x - this->vertices[i].position.x < 0.0f) {
            this->font->drawText(
                this->labels[i],
                this->vertices[i].position.x,
                this->vertices[i].position.y
            );
        } else if (this->center.position.x - this->vertices[i].position.x > 0.0f) {
            float text_width = this->font->getStringWidth(this->labels[i]);

            this->font->drawText(
                this->labels[i],
                this->vertices[i].position.x - text_width,
                this->vertices[i].position.y
            );
        } else {
            float text_width = this->font->getStringWidth(this->labels[i]);

            this->font->drawText(
                this->labels[i],
                this->vertices[i].position.x - text_width / 2.0f,
                this->vertices[i].position.y
            );
        }
    }
    this->renderer->setColor(old_color);
}

void gGUIRadarChart::drawChart() {
    for (const gDataset &dataset : this->datasets) {
        gColor *old_color = this->renderer->getColor();
        this->renderer->setColor(dataset.color);

        std::vector<float> vertices(this->vertices.size() * 2);
        float circumradius = std::sqrt(
            std::pow(this->center.position.x - this->vertices[0].position.x, 2.0f)
                + std::pow(this->center.position.y - this->vertices[0].position.y, 2.0f)
        );

        for (std::size_t i = 0; i < vertices.size(); i += 2) {
            float distance = dataset.variables[i / 2] * (circumradius / this->max);

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
        this->renderer->setColor(old_color);
    }
}
