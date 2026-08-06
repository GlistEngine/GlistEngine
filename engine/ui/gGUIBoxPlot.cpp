/*
 * gGUIBoxPlot.cpp
 *
 *  Created on: Ap 27, 2026
 *      Author: ahmet celal
 */

#include "gGUIBoxPlot.h"

#include <algorithm>
#include <cmath>
#include <limits>

//#include "gBaseApp.h"
//#include "gBaseCanvas.h"

gGUIBoxPlot::gGUIBoxPlot() {
    showOutliers = true;
    title = "Box Plot";
    boxWidth = 160;
    markerSize = 6;
    hasStats = false;
}

gGUIBoxPlot::~gGUIBoxPlot() {}

void gGUIBoxPlot::set(gBaseApp* root,
                      gBaseGUIObject* topParentGUIObject,
                      gBaseGUIObject* parentGUIObject,
                      int parentSlotLineNo,
                      int parentSlotColumnNo,
                      int x, int y, int w, int h) {
    gGUIControl::set(root, topParentGUIObject, parentGUIObject,
                     parentSlotLineNo, parentSlotColumnNo, x, y, w, h);

    // chart rect inside control
    chartX = left + padLeft;
    chartY = top + height - padBottom;          // bottom baseline in screen coords
    chartW = width - padLeft - padRight;
    chartH = height - padTop - padBottom;

    rebuildYScale();
}

void gGUIBoxPlot::update() {
}

void gGUIBoxPlot::setData(const std::vector<float>& values) {
    raw.clear();
    raw.reserve(values.size());
    for(float v : values) {
        if(std::isfinite(v)) raw.push_back(v);
    }
    hasStats = computeStats();
    rebuildYScale();
}

void gGUIBoxPlot::clearData() {
    raw.clear();
    sorted.clear();
    outliers.clear();
    hasStats = false;
    rebuildYScale();
}

void gGUIBoxPlot::setShowOutliers(bool show) {
    showOutliers = show;
}

bool gGUIBoxPlot::getShowOutliers() const {
    return showOutliers;
}

void gGUIBoxPlot::setTitle(const std::string& t) {
    title = t;
}

const std::string& gGUIBoxPlot::getTitle() const {
    return title;
}

void gGUIBoxPlot::setBoxWidth(int w) {
    boxWidth = std::max(20, w);
}

void gGUIBoxPlot::setMarkerSize(int px) {
    markerSize = std::max(2, px);
}

float gGUIBoxPlot::medianSorted(const std::vector<float>& s, int l, int rExclusive) {
    int n = rExclusive - l;
    if(n <= 0) return 0.0f;
    int mid = l + n / 2;
    if(n % 2 == 1) return s[mid];
    return 0.5f * (s[mid - 1] + s[mid]);
}

bool gGUIBoxPlot::computeStats() {
    outliers.clear();
    sorted = raw;

    if(sorted.size() < 5) {
        return false;
    }

    std::sort(sorted.begin(), sorted.end());
    int n = (int)sorted.size();

    // Median
    med = medianSorted(sorted, 0, n);

    // Q1/Q3 by median-of-halves
    // If n is odd, exclude the median element from halves
    if(n % 2 == 1) {
        q1 = medianSorted(sorted, 0, n/2);
        q3 = medianSorted(sorted, n/2 + 1, n);
    } else {
        q1 = medianSorted(sorted, 0, n/2);
        q3 = medianSorted(sorted, n/2, n);
    }

    iqr = q3 - q1;
    float lowFence = q1 - 1.5f * iqr;
    float highFence = q3 + 1.5f * iqr;

    // whiskers are most extreme values inside fences
    whiskLow = sorted.front();
    whiskHigh = sorted.back();

    for(int i = 0; i < n; i++) {
        if(sorted[i] >= lowFence) {
            whiskLow = sorted[i];
            break;
        }
    }
    for(int i = n - 1; i >= 0; i--) {
        if(sorted[i] <= highFence) {
            whiskHigh = sorted[i];
            break;
        }
    }

    // outliers
    for(float v : sorted) {
        if(v < whiskLow || v > whiskHigh) outliers.push_back(v);
    }

    return true;
}

void gGUIBoxPlot::rebuildYScale() {
    // default
    minY = 0.0f;
    maxY = 1.0f;

    if(!hasStats) return;

    // scale by this column only (include outliers so they fit)
    float minv = sorted.front();
    float maxv = sorted.back();

    float range = maxv - minv;
    float pad = std::max(range * 0.08f, 1e-6f);

    minY = minv - pad;
    maxY = maxv + pad;

    if(maxY - minY < 1e-9f) {
        maxY = minY + 1.0f;
    }
}

int gGUIBoxPlot::yToPix(float y) const {
    // y=maxY => top, y=minY => bottom
    float t = (y - minY) / (maxY - minY);
    float py = (float)chartY - t * (float)chartH;
    return (int)std::round(py);
}

void gGUIBoxPlot::drawOutlierMarker(int x, int y) const {
    int s = markerSize;
    // small filled square marker
    gDrawRectangle(x - s/2, y - s/2, s, s, true);
}

void gGUIBoxPlot::draw() {
//    renderer->setColor(gColor(1,1,1,1));
//	gDrawRectangle(left, top, width, height, true);

    gColor* old = renderer->getColor();

    // If no data
    if(!hasStats) {
        renderer->setColor(0.35f, 0.35f, 0.35f);
        if(font) {
            getFont()->drawText("Not enough data for box plot.", left + 20, top + 80);
        }
        renderer->setColor(old);
        return;
    }

    // Recompute chart rect in case resized but set() not called again
    chartX = left + padLeft;
    chartY = top + height - padBottom;
    chartW = width - padLeft - padRight;
    chartH = height - padTop - padBottom;

    // Axis (simple Y axis)
    renderer->setColor(0.75f, 0.75f, 0.75f);
    gDrawLine(chartX, chartY - chartH, chartX, chartY);

    // Title
    renderer->setColor(0.25f, 0.25f, 0.25f);
    if(font) {
        getFont()->drawText(title, left + 20, top + 20);
    }

    // Center X
    int cx = chartX + chartW / 2;

    int yQ1  = yToPix(q1);
    int yQ3  = yToPix(q3);
    int yMed = yToPix(med);
    int yWL  = yToPix(whiskLow);
    int yWH  = yToPix(whiskHigh);

    // Box (Q1..Q3)
    int boxTop = yQ3;
    int boxH   = yQ1 - yQ3;

    // fill
    renderer->setColor(0.90f, 0.90f, 0.90f);
    gDrawRectangle(cx - boxWidth/2, boxTop, boxWidth, boxH, true);

    // border
    renderer->setColor(0.35f, 0.35f, 0.35f);
    gDrawRectangle(cx - boxWidth/2, boxTop, boxWidth, boxH, false);

    // Median line
    renderer->setColor(0.20f, 0.20f, 0.20f);
    gDrawLine(cx - boxWidth/2, yMed, cx + boxWidth/2, yMed);

    // Whisker line + caps
    renderer->setColor(0.35f, 0.35f, 0.35f);
    gDrawLine(cx, yWH, cx, yWL);

    int capW = (int)(boxWidth * 0.65f);
    gDrawLine(cx - capW/2, yWH, cx + capW/2, yWH);
    gDrawLine(cx - capW/2, yWL, cx + capW/2, yWL);

    // Outliers
    if(showOutliers) {
        renderer->setColor(0.65f, 0.20f, 0.20f);
        for(float o : outliers) {
            int yo = yToPix(o);
            drawOutlierMarker(cx, yo);
        }
    }

    // Footer quick stats (optional)
    renderer->setColor(0.35f, 0.35f, 0.35f);
    if(font) {
        getFont()->drawText("Q1=" + gToStr(q1) + "  Med=" + gToStr(med) + "  Q3=" + gToStr(q3),
                       left + 20, top + height - 45);
        getFont()->drawText("Whiskers: [" + gToStr(whiskLow) + ", " + gToStr(whiskHigh) + "]" +
                       (showOutliers ? ("  Outliers: " + gToStr((int)outliers.size())) : ""),
                       left + 20, top + height - 25);
    }

    renderer->setColor(old);
}
