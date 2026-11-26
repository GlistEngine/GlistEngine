/*
 * gGUIGraph.cpp
 *
 *  Created on: Sep 23, 2022
 *      Author: noyan
 */

#include "gGUIGraph.h"
#include <algorithm>
#include <cmath>
#include <memory>

gGUIGraph::gGUIGraph() {
	axisx1 = 0;
	axisx2 = 0;
	axisy1 = 0;
	axisy2 = 0;
	axisxstart = 0;
	axisystart = 0;
	axisxw = 0;
	axisyh = 0;
	maxx = -2147483647.0f;
	maxy = -2147483647.0f;
	minx = 0;
	miny = 0;
	largestvaluex = 0;
	largestvaluey = 0;
	smallestvaluex = 0;
	smallestvaluey = 0;

	rangeenabled = false;
	rangestart = 0;
	rangeend = 0;

	labelwidthx = 0.0f;
	labelwidthy = 0.0f;
	gridlinesxenabled = true;
	gridlinesyenabled = true;
	isxaxislinenable = true;
	yaxislenable = true;
	floatlabelsxenabled = false;
	floatlabelsyenabled = false;
	isbackgroundenabled = true;
	labelcountx = 5;
	labelcounty = 7;

	axisxtitle = "x-axis";
	axisytitle = "y-axis";

	for (int i = 0; i < labelcounty; i++) {
		labelsx.push_back(i);
		labelsy.push_back(i);
	}

	for (int i = 0; i < 8; i++) {
		int labelstep = std::pow(10, i);
		labelsteps.push_back(labelstep);
		labelsteps.push_back(labelstep * 2);
		labelsteps.push_back(labelstep * 5);
	}
}

gGUIGraph::~gGUIGraph() {
}

void gGUIGraph::set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h) {
	gGUIControl::set(root, topParentGUIObject, parentGUIObject, parentSlotLineNo, parentSlotColumnNo, x, y, w, h);
	axisxw = width * 9 / 10;
	axisyh = height * 3 / 4;
	axisx1 = left + 80;
	axisy1 = top + 0.14f * height;
	axisx2 = axisx1 + axisxw;
	axisy2 = axisy1 + axisyh;
	axisxstart = axisy2;
	if (miny < 0) axisxstart -= axisyh * ((-miny) / (maxy - miny));
	axisystart = axisx1;
	if (minx < 0) axisystart += axisxw * ((-minx) / (maxx - minx));
	if (labelcountx > 1) labelwidthx = axisxw / (labelcountx - 1);
	else labelwidthx = 0;
	if (labelcounty > 1) labelwidthy = axisyh / (labelcounty - 1);
	else labelwidthy = 0;
}

void gGUIGraph::setMaxX(float maxX) {
	maxx = maxX;
	largestvaluex = maxX;
	updateLabelsX();
}

int gGUIGraph::getMaxX() {
	return maxx;
}

void gGUIGraph::setMinX(float minX) {
	minx = minX;
	smallestvaluex = minX;
	updateLabelsX();
}

int gGUIGraph::getMinX() {
	return minx;
}

void gGUIGraph::setMaxY(float maxY) {
	maxy = maxY;
	largestvaluey = maxY;
	updateLabelsY();
}

int gGUIGraph::getMaxY() {
	return maxy;
}

void gGUIGraph::setMinY(float minY) {
	miny = minY;
	smallestvaluey = minY;
	updateLabelsY();
}

int gGUIGraph::getMinY() {
	return miny;
}

void gGUIGraph::enableGridlinesX(bool isEnabled) {
	gridlinesxenabled = isEnabled;
}

void gGUIGraph::enableGridlinesY(bool isEnabled) {
	gridlinesyenabled = isEnabled;
}

void gGUIGraph::setAxislinesEnable(bool xaxis, bool yaxis) {
	isxaxislinenable = xaxis;
	yaxislenable = yaxis;
}

void gGUIGraph::setTitleX(std::string titleX) {
	axisxtitle = titleX;
}

std::string gGUIGraph::getTitleX() {
	return axisxtitle;
}

void gGUIGraph::setTitleY(std::string titleY) {
	axisytitle = titleY;
}

std::string gGUIGraph::getTitleY() {
	return axisytitle;
}

void gGUIGraph::setLabelCountX(int labelCount) {
	labelcountx = labelCount;
	if (labelcountx > 1) {
		labelwidthx = axisxw / (labelcountx - 1);
	} else {
		labelwidthx = 0;
	}
}

int gGUIGraph::getLabelCountX() {
	return labelcountx;
}

void gGUIGraph::setLabelCountY(int labelCount) {
	labelcounty = labelCount;
	if (labelcounty > 1) {
		labelwidthy = axisyh / (labelcounty - 1);
	} else {
		labelwidthy = 0;
	}
}

int gGUIGraph::getLabelCountY() {
	return labelcounty;
}

void gGUIGraph::enableRange(bool isRangeEnabled) {
	rangeenabled = isRangeEnabled;
}

void gGUIGraph::setRange(float rangeStart, float rangeEnd) {
	rangestart = rangeStart;
	rangeend = rangeEnd;
}

int gGUIGraph::getRangeStart() {
	return rangestart;
}

int gGUIGraph::getRangeEnd() {
	return rangeend;
}

void gGUIGraph::enableBackground(bool isEnabled) {
	isbackgroundenabled = isEnabled;
}

bool gGUIGraph::isBackgroundEnabled() {
	return isbackgroundenabled;
}

void gGUIGraph::draw() {
	gColor oldcolor = *renderer->getColor();

	if (title != "") {
		renderer->setColor(fontcolor);
		getFont()->drawText(title, (axisx1 + axisx2) / 2, axisy1 - 10);
	}

	drawBackground();
	drawLabels();
	drawGraph();

	renderer->setColor(oldcolor);
}

void gGUIGraph::drawBackground() {
	// First draw the background
	if (isbackgroundenabled) {
		renderer->setColor(textbackgroundcolor);
		gDrawRectangle(left, top, width, height, true);
	}

	// Draw the axis
	renderer->setColor(backgroundcolor);
	if (isxaxislinenable) gDrawLine(axisx1, axisxstart, axisx2, axisxstart);
	if (yaxislenable) gDrawLine(axisystart, axisy2, axisystart, axisy1);
}

void gGUIGraph::drawLabels() {
	renderer->setColor(foregroundcolor);

	// Draw the labels for x-axis
	if (labelcountx > 0) {
		float xpoint = 0;
		float labely = axisxstart + 0.04f * height + 4.0f;// small fixed gap below the tick mark, independent of the control's size
		for (int i = 0; i < labelcountx; i++) {
			xpoint = i * labelwidthx;
			if (isxaxislinenable) {
				renderer->setColor(foregroundcolor);
				if (!gridlinesxenabled) gDrawLine(axisx1 + xpoint, axisy2 + 0.04f * height, axisx1 + xpoint, axisy2);
				else gDrawLine(axisx1 + xpoint, axisy2 + 0.04f * height, axisx1 + xpoint, axisy2 - axisyh);
			}

			renderer->setColor(fontcolor);
			std::string labeltext;
			if (floatlabelsxenabled) {
				if (i < floatlabelsx.size()) labeltext = gToStr(floatlabelsx[i]);
			} else {
				if (i < labelsx.size()) labeltext = std::to_string(labelsx[i]);
			}
			if (!labeltext.empty()) getFont()->drawText(labeltext, axisx1 + xpoint - getFont()->getStringWidth(labeltext) / 2.0f, labely);
		}
	} else if (isxaxislinenable) {
		renderer->setColor(foregroundcolor);
		float yend = gridlinesxenabled ? (axisy2 - axisyh) : axisy2;
		gDrawLine(axisx1, axisy2 + 0.04f * height, axisx1, yend);
		gDrawLine(axisx2, axisy2 + 0.04f * height, axisx2, yend);
	}

	// Draw the labels for y-axis
	if (labelcounty > 0) {
		int ypoint = 0;
		float lineheight = getFont()->getLineHeight();
		for (int i = 0; i < labelcounty; i++) {
			ypoint = i * labelwidthy;
			if (yaxislenable) {
				renderer->setColor(foregroundcolor);
				if (!gridlinesyenabled) gDrawLine(axisx1 - 0.02f * width, axisy2 - ypoint, axisx1, axisy2 - ypoint);
				else gDrawLine(axisx1 - 0.02f * width, axisy2 - ypoint, axisx1 + axisxw, axisy2 - ypoint);
			}

			renderer->setColor(fontcolor);
			std::string labeltext;
			if (floatlabelsyenabled) {
				if (i < floatlabelsy.size()) labeltext = gToStr(floatlabelsy[i]);
			} else {
				if (i < labelsy.size()) labeltext = std::to_string(labelsy[i]);
			}
			if (!labeltext.empty()) {
				float labelw = getFont()->getStringWidth(labeltext);
				getFont()->drawText(labeltext, axisystart - labelw - 8.0f, axisy2 - ypoint - lineheight / 2.0f);
			}
		}
	} else if (yaxislenable) {
		renderer->setColor(foregroundcolor);
		float xend = gridlinesyenabled ? (axisx1 + axisxw) : axisx1;
		gDrawLine(axisx1 - 0.02f * width, axisy2, xend, axisy2);
		gDrawLine(axisx1 - 0.02f * width, axisy2 - axisyh, xend, axisy2 - axisyh);
	}
}

void gGUIGraph::drawGraph() {
}

void gGUIGraph::updateLabelsX() {
	if (floatlabelsxenabled) {
		float step = (maxx - minx) / (labelcountx - 1);
		floatlabelsx.resize(labelcountx);
		for (int i = 0; i < labelcountx; i++) {
			floatlabelsx[i] = minx + i * step;
		}
	} else {
		int step = abs(largestvaluex) / 10;
		for (int i = 0; i < labelsteps.size(); i++) {
			if (step < labelsteps[i]) {
				step = labelsteps[i];
				break;
			}
		}

		if (minx < 0) minx = (int(smallestvaluex / step) - 1) * step;
		maxx = (int)std::ceil(largestvaluex / (float)step) * step;// round up only as far as needed to cover the data, no extra step
		labelcountx = (maxx - minx) / step + 1;
		//		gLogi("labelx") << labelcountx << " " << maxx << " " << minx << " " << step;
		labelsx.clear();
		for (int i = 0; i < labelcountx + 1; i++) {
			labelsx.push_back(minx + i * step);
		}
	}
}

void gGUIGraph::updateLabelsY() {
	if (floatlabelsyenabled) {
		float step = (maxy - miny) / (labelcounty - 1);
		floatlabelsy.resize(labelcounty);
		for (int i = 0; i < labelcounty; i++) {
			floatlabelsy[i] = miny + i * step;
		}
	} else {
		int step = std::abs((int) largestvaluey) / 10;
		for (int i = 0; i < labelsteps.size(); i++) {
			if (step < labelsteps[i]) {
				step = labelsteps[i];
				break;
			}
		}
		if (step <= 0) step = 1;

		if (smallestvaluey >= 0) {
			miny = 0;
		} else {
			miny = (int(smallestvaluey / step) - 1) * step;
		}

		maxy = (int)std::ceil(largestvaluey / (float)step) * step;// round up only as far as needed to cover the data, no extra step

		labelcounty = (maxy - miny) / step + 1;
		labelsy.clear();
		for (int i = 0; i < labelcounty; i++) {
			labelsy.push_back(miny + i * step);
		}
	}
}

void gGUIGraph::setLabelsYForRange(float rangeMinY, float rangeMaxY) {
	float span = rangeMaxY - rangeMinY;
	if (span < 1e-6f) span = 1.0f;
	int targetcount = (labelcounty > 1) ? labelcounty : 7;

	// snap the step to a "nice" 1/2/5 x 10^n value so labels land on whole numbers whenever the
	// data range allows it (e.g. EUR-SEK's ~7-12), falling back to a nice decimal step only when
	// the range is too narrow for whole numbers to be meaningful (e.g. EUR-GBP's ~0.66-0.92)
	float rawstep = span / (targetcount - 1);
	float magnitude = std::pow(10.0f, std::floor(std::log10(rawstep)));
	float normalized = rawstep / magnitude;
	float nicestep = ((normalized <= 1.0f) ? 1.0f : (normalized <= 2.0f) ? 2.0f : (normalized <= 5.0f) ? 5.0f : 10.0f) * magnitude;

	float nicemin = std::floor(rangeMinY / nicestep) * nicestep;
	float nicemax = std::ceil(rangeMaxY / nicestep) * nicestep;

	miny = nicemin;
	maxy = nicemax;
	labelcounty = (int)std::round((nicemax - nicemin) / nicestep) + 1;

	floatlabelsy.resize(labelcounty);
	for (int i = 0; i < labelcounty; i++) {
		floatlabelsy[i] = nicemin + i * nicestep;
	}
	labelwidthy = (labelcounty > 1) ? axisyh / (labelcounty - 1) : 0.0f;
}

int gGUIGraph::countDigits(int number) {
	return std::to_string(number).size();
}

void gGUIGraph::clear() {
}

bool gGUIGraph::buildPolylineRibbon(PolylineMesh& mesh, const std::vector<std::vector<std::array<float, 2>>>& runs, float halfWidth, bool roundedCorners) {
	if (runs.empty() || halfWidth <= 0.0f) {
		mesh.clear();
		return false;
	}

	const float pi = 3.14159265358979323846f;
	auto verts = std::make_shared<std::vector<gVertex>>();
	auto inds = std::make_shared<std::vector<gIndex>>();

	auto pushVert = [&](float x, float y) -> gIndex {
		gVertex v;
		v.position = glm::vec3(x, y, 0.0f);
		gIndex idx = static_cast<gIndex>(verts->size());
		verts->push_back(v);
		return idx;
	};

	auto emitArcFan = [&](float cx, float cy, float startX, float startY, float endX, float endY, float angle1, float delta, int steps) {
		gIndex center = pushVert(cx, cy);
		gIndex prev = pushVert(cx + startX * halfWidth, cy + startY * halfWidth);
		for (int s = 1; s <= steps; s++) {
			gIndex next = 0;
			if (s == steps) {
				next = pushVert(cx + endX * halfWidth, cy + endY * halfWidth);
			} else {
				float t = static_cast<float>(s) / static_cast<float>(steps);
				float angle = angle1 + delta * t;
				next = pushVert(cx + std::cos(angle) * halfWidth, cy + std::sin(angle) * halfWidth);
			}
			inds->push_back(center);
			inds->push_back(prev);
			inds->push_back(next);
			prev = next;
		}
	};

	for (const auto& rawrun : runs) {
		std::vector<std::array<float, 2>> clean;
		clean.reserve(rawrun.size());
		for (const auto& pt : rawrun) {
			if (!std::isfinite(pt[0]) || !std::isfinite(pt[1])) {
				continue;
			}
			if (clean.empty()) {
				clean.push_back(pt);
			} else {
				float dx = pt[0] - clean.back()[0];
				float dy = pt[1] - clean.back()[1];
				if (dx * dx + dy * dy > 1e-4f) {
					clean.push_back(pt);
				}
			}
		}

		if (clean.size() < 2) {
			continue;
		}

		const size_t n = clean.size();
		const size_t segcount = n - 1;
		std::vector<std::array<float, 2>> dirs(segcount);
		std::vector<std::array<float, 2>> norms(segcount);

		for (size_t i = 0; i < segcount; i++) {
			float dx = clean[i + 1][0] - clean[i][0];
			float dy = clean[i + 1][1] - clean[i][1];
			float len = std::sqrt(dx * dx + dy * dy);
			if (len < 1e-6f) len = 1.0f;
			float ux = dx / len;
			float uy = dy / len;
			dirs[i] = {ux, uy};
			norms[i] = {-uy, ux};
		}

		// Emit true-width segment quads
		for (size_t i = 0; i < segcount; i++) {
			float nx = norms[i][0] * halfWidth;
			float ny = norms[i][1] * halfWidth;

			float x0 = clean[i][0];
			float y0 = clean[i][1];
			float x1 = clean[i + 1][0];
			float y1 = clean[i + 1][1];

			gIndex aL = pushVert(x0 + nx, y0 + ny);
			gIndex aR = pushVert(x0 - nx, y0 - ny);
			gIndex bL = pushVert(x1 + nx, y1 + ny);
			gIndex bR = pushVert(x1 - nx, y1 - ny);

			inds->push_back(aL);
			inds->push_back(bL);
			inds->push_back(bR);
			inds->push_back(aL);
			inds->push_back(bR);
			inds->push_back(aR);
		}

		// Emit interior corner joins (rounded arc fans when enabled, sharp miter/bevel by default)
		for (size_t k = 1; k + 1 < n; k++) {
			float cross = dirs[k - 1][0] * dirs[k][1] - dirs[k - 1][1] * dirs[k][0];
			float dot = dirs[k - 1][0] * dirs[k][0] + dirs[k - 1][1] * dirs[k][1];
			if (std::fabs(cross) < 1e-4f && dot > 0.0f) {
				continue;
			}

			float cx = clean[k][0];
			float cy = clean[k][1];

			if (roundedCorners) {
				float o1x = 0.0f;
				float o1y = 0.0f;
				float o2x = 0.0f;
				float o2y = 0.0f;
				float angle1 = 0.0f;
				float delta = 0.0f;

				if (std::fabs(cross) < 1e-4f) {
					// 180-degree reversal: semicircular cap bulging along incoming direction
					o1x = norms[k - 1][0];
					o1y = norms[k - 1][1];
					o2x = -o1x;
					o2y = -o1y;
					angle1 = std::atan2(o1y, o1x);
					float rot90x = -o1y;
					float rot90y = o1x;
					delta = (rot90x * dirs[k - 1][0] + rot90y * dirs[k - 1][1] >= 0.0f) ? pi : -pi;
				} else {
					// In Y-down coordinates with norm=(-dy, dx), cross > 0 turns clockwise => outer corner is -norm
					float side = (cross > 0.0f) ? -1.0f : 1.0f;
					o1x = norms[k - 1][0] * side;
					o1y = norms[k - 1][1] * side;
					o2x = norms[k][0] * side;
					o2y = norms[k][1] * side;
					angle1 = std::atan2(o1y, o1x);
					float angle2 = std::atan2(o2y, o2x);
					delta = angle2 - angle1;
					while (delta > pi) delta -= 2.0f * pi;
					while (delta < -pi) delta += 2.0f * pi;
				}

				int steps = std::max(2, static_cast<int>(std::ceil(std::fabs(delta) / (pi / 6.0f))));
				emitArcFan(cx, cy, o1x, o1y, o2x, o2y, angle1, delta, steps);
			} else {
				if (std::fabs(cross) < 1e-4f) {
					continue;
				}

				float side = (cross > 0.0f) ? -1.0f : 1.0f;
				float o1x = norms[k - 1][0] * side;
				float o1y = norms[k - 1][1] * side;
				float o2x = norms[k][0] * side;
				float o2y = norms[k][1] * side;

				float bx = o1x + o2x;
				float by = o1y + o2y;
				float blen = std::sqrt(bx * bx + by * by);
				float dothalf = (blen > 1e-5f) ? ((bx * o1x + by * o1y) / blen) : 0.0f;

				gIndex center = pushVert(cx, cy);
				gIndex pPrev = pushVert(cx + o1x * halfWidth, cy + o1y * halfWidth);
				gIndex pNext = pushVert(cx + o2x * halfWidth, cy + o2y * halfWidth);

				if (dothalf > 0.5f) {
					float ux = bx / blen;
					float uy = by / blen;
					float miterdist = halfWidth / dothalf;
					gIndex pMiter = pushVert(cx + ux * miterdist, cy + uy * miterdist);

					inds->push_back(center);
					inds->push_back(pPrev);
					inds->push_back(pMiter);
					inds->push_back(center);
					inds->push_back(pMiter);
					inds->push_back(pNext);
				} else {
					inds->push_back(center);
					inds->push_back(pPrev);
					inds->push_back(pNext);
				}
			}
		}

		// Emit rounded end-caps when rounded corners are enabled
		if (roundedCorners) {
			float s1x = norms[0][0];
			float s1y = norms[0][1];
			float soutx = -dirs[0][0];
			float souty = -dirs[0][1];
			float sangle = std::atan2(s1y, s1x);
			float srot90x = -s1y;
			float srot90y = s1x;
			float sdelta = (srot90x * soutx + srot90y * souty >= 0.0f) ? pi : -pi;
			emitArcFan(clean.front()[0], clean.front()[1], s1x, s1y, -s1x, -s1y, sangle, sdelta, 4);

			float e1x = norms.back()[0];
			float e1y = norms.back()[1];
			float eoutx = dirs.back()[0];
			float eouty = dirs.back()[1];
			float eangle = std::atan2(e1y, e1x);
			float erot90x = -e1y;
			float erot90y = e1x;
			float edelta = (erot90x * eoutx + erot90y * eouty >= 0.0f) ? pi : -pi;
			emitArcFan(clean.back()[0], clean.back()[1], e1x, e1y, -e1x, -e1y, eangle, edelta, 4);
		}
	}

	if (inds->empty()) {
		mesh.clear();
		return false;
	}

	mesh.setRibbonVertices(verts, inds);
	return true;
}

void gGUIGraph::buildPolylineStroke(PolylineStroke& stroke, const std::vector<std::vector<std::array<float, 2>>>& runs, float thickness, bool roundedCorners) {
	if (runs.empty() || thickness <= 0.0f) {
		stroke.clear();
		return;
	}

	if (roundedCorners) {
		float targetwidth = std::max(0.75f, thickness);
		float corehalf = std::max(0.36f, targetwidth * 0.5f - std::min(0.20f, targetwidth * 0.06f));
		float fringehalf = corehalf + 0.40f;
		stroke.hascore = buildPolylineRibbon(stroke.coremesh, runs, corehalf, true);
		stroke.hasfringe = buildPolylineRibbon(stroke.fringemesh, runs, fringehalf, true);
	} else {
		float halfwidth = std::max(0.5f, thickness * 0.5f);
		stroke.hascore = buildPolylineRibbon(stroke.coremesh, runs, halfwidth, false);
		stroke.hasfringe = false;
		stroke.fringemesh.clear();
	}
}

void gGUIGraph::drawPolylineStroke(PolylineStroke& stroke, const gColor& color) {
	if (renderer == nullptr) return;
	if (stroke.hasfringe) {
		gColor fringecolor = color;
		fringecolor.a = color.a * 0.24f;
		renderer->setColor(fringecolor);
		stroke.fringemesh.draw();
	}
	if (stroke.hascore) {
		renderer->setColor(color);
		stroke.coremesh.draw();
	}
}
