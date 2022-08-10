/*
 * gGUIRadarChart.cpp
 *
 *  Created on: Jul 27, 2022
 *      Author: emirhantasdeviren
 */

#include "gGUIRadarChart.h"

gGUIRadarChart::gGUIRadarChart() :
    datasets{{{0.0f, 0.0f, 0.0f}, gColor::RED}}, vertices(3), max{1.0f} {}

gGUIRadarChart::~gGUIRadarChart() {}

void gGUIRadarChart::draw() {
	gColor *old_color = this->renderer->getColor();
	this->renderer->setColor(0.0f, 0.0f, 0.0f);
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

void gGUIRadarChart::setNumDataset(std::size_t new_size) {
	this->datasets.resize(new_size);
}

void gGUIRadarChart::setNumVar(std::size_t new_size) {
    if (new_size >= 3) {
        for (gDataset &dataset : this->datasets) {
            dataset.variables.resize(new_size);
        }
        this->vertices.resize(new_size);
    }
}

void gGUIRadarChart::calcVertices() {
	int length = std::min((this->right - this->left), (this->bottom - this->top));

    switch (this->vertices.size()) {
        case 3: {
            this->calcTriangle(length);
        } break;
        case 4: {
            this->calcSquare(length);
        } break;
        case 5: {
            this->calcPentagon(length);
        } break;
    }
}

void gGUIRadarChart::drawBase() {
    gColor *old_color = this->renderer->getColor();
    this->renderer->setColor(this->middlegroundcolor);

    for (std::size_t i = 0; i < this->vertices.size(); i++) {
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

void gGUIRadarChart::calcTriangle(int a) {
	int h = static_cast<int>((std::sqrt(3) / 2.0f) * static_cast<float>(a));
    int pad = ((this->bottom - this->top) - h) / 2;

	int middle_x = (this->left + this->right) / 2;
	int middle_y = (this->top + pad + (h / 3 * 2));

	this->center.position.x = static_cast<float>(middle_x);
	this->center.position.y = static_cast<float>(middle_y);

	this->vertices[0].position.x = middle_x;
	this->vertices[0].position.y = middle_y - (h / 3 * 2);

	this->vertices[1].position.x = middle_x - (a / 2);
	this->vertices[1].position.y = middle_y + (h / 3);

	this->vertices[2].position.x = middle_x + (a / 2);
    this->vertices[2].position.y = middle_y + (h / 3);


}

void gGUIRadarChart::calcSquare(int a) {
    int min_pad = 3;
    int h = a - 2 * min_pad;

    int pad_x = ((this->right - this->left) - h) / 2;
    int pad_y = ((this->bottom - this->top) - h) / 2;

	int middle_x = this->left + std::max(min_pad, pad_x) + h / 2;
	int middle_y = this->top + std::max(min_pad, pad_y) + h / 2;

    this->center.position.x = middle_x;
    this->center.position.y = middle_y;

    this->vertices[0].position.x = middle_x;
    this->vertices[0].position.y = middle_y - h / 2;

    this->vertices[1].position.x = middle_x - h / 2;
    this->vertices[1].position.y = middle_y;

    this->vertices[2].position.x = middle_x;
    this->vertices[2].position.y = middle_y + h / 2;

    this->vertices[3].position.x = middle_x + h / 2;
    this->vertices[3].position.y = middle_y;
}

void gGUIRadarChart::calcPentagon(int a) {
    int min_pad = 3;

    int circumdiameter = a - 2 * min_pad;
    int circumradius = circumdiameter / 2;

    int t = static_cast<int>(
        static_cast<float>(circumradius) / std::sqrt((5.0f + std::sqrt(5.0f)) / 10.0f)
    );

    int pad_x = (this->right - this->left - circumdiameter) / 2;
    int pad_y = (this->bottom - this->top - circumdiameter) / 2;

	int middle_x = this->left + std::max(min_pad, pad_x) + circumradius;
	int middle_y = this->top + std::max(min_pad, pad_y) + circumradius;

    float px = static_cast<float>(middle_x);
    float py = static_cast<float>(middle_y - circumradius);

    float qx = px - static_cast<float>(t) * std::sin(gDegToRad(54.0f));
    float qy = py + static_cast<float>(t) * std::cos(gDegToRad(54.0f));

    float rx = qx + static_cast<float>(t) * std::sin(gDegToRad(18.0f));
    float ry = qy + static_cast<float>(t) * std::cos(gDegToRad(18.0f));

    float sx = rx + static_cast<float>(t);
    float sy = ry;

    float ux = px + static_cast<float>(t) * std::sin(gDegToRad(54.0f));
    float uy = py + static_cast<float>(t) * std::cos(gDegToRad(54.0f));

    this->center.position.x = middle_x;
    this->center.position.y = middle_y;

    this->vertices[0].position.x = px;
    this->vertices[0].position.y = py;

    this->vertices[1].position.x = qx;
    this->vertices[1].position.y = qy;

    this->vertices[2].position.x = rx;
    this->vertices[2].position.y = ry;
    
    this->vertices[3].position.x = sx;
    this->vertices[3].position.y = sy;

    this->vertices[4].position.x = ux;
    this->vertices[4].position.y = uy;
}
