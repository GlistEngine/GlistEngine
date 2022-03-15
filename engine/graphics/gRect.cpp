/*
 * gRect.cpp
 *
 *  Created on: 9 Ara 2020
 *      Author: Emrullah
 */

#include "gRect.h"

/**
 * Default constructor sets values to zero.
 */
gRect::gRect() : l(0), t(0), r(0), b(0), width(0), height(0) {}

gRect::gRect(int left, int top, int right, int bottom) : l(left), t(top), r(right), b(bottom), width(right - left), height(bottom - top) {}

gRect::gRect(const gRect& r) : l(r.left()), t(r.top()), r(r.right()), b(r.bottom()), width(r.right() - r.left()), height(r.bottom() - r.top()) {}

gRect::~gRect() {}

void gRect::set(int left, int top, int right, int bottom) {
	l = left;
	t = top;
	r = right;
	b = bottom;
	width = right - left;
	height = bottom - top;
}

void gRect::set(const gRect& r) {
	l = r.left();
	t = r.top();
	this->r = r.right();
	b = r.bottom();
	width = r.getWidth();
	height = r.getHeight();
}

int gRect::centerX() const {
	return l + (width / 2);
}

int gRect::centerY() const {
	return t + (height / 2);
}

float gRect::exactCenterX() const {
	return l + (width / 2);
}

float gRect::exactCenterY() const {
	return t + (height / 2);
}

bool gRect::intersects(int left1, int top1, int right1, int bottom1, int left2, int top2, int right2, int bottom2) { return left1 <= right2 && right1 >= left2 && top1 <= bottom2 && bottom1 >= top2; }

bool gRect::intersects(const gRect& r1, const gRect& r2) { return intersects(r1.left(), r1.top(), r1.right(), r1.bottom(), r2.left(), r2.top(), r2.right(), r2.bottom()); }

bool gRect::intersects(int left, int top, int right, int bottom) const { return intersects(l, t, r, b, left, top, right, bottom); }

bool gRect::intersects(const gRect& r) const { return intersects(l, t, this->r, b, r.left(), r.top(), r.right(), r.bottom()); }

bool gRect::contains(int left1, int top1, int right1, int bottom1, int left2, int top2, int right2, int bottom2) { return left1 <= left2 && right1 >= right2 && top1 <= top2 && bottom1 >= bottom2; }

bool gRect::contains(const gRect& r1, const gRect& r2) { return contains(r1.left(), r1.top(), r1.right(), r1.bottom(), r2.left(), r2.top(), r2.right(), r2.bottom()); }

bool gRect::contains(int left, int top, int right, int bottom) const { return contains(l, t, r, b, left, top, right, bottom); }

bool gRect::contains(const gRect& r) const { return contains(l, t, this->r, b, r.left(), r.top(), r.right(), r.bottom()); }

bool gRect::contains(int x, int y) const { return contains(l, t, r, b, x, y, x, y); }

int gRect::left() const { return l; }

int gRect::top() const { return t; }

int gRect::right() const { return r; }

int gRect::bottom() const { return b; }

int gRect::getWidth() const { return width; }

int gRect::getHeight() const { return height; }
