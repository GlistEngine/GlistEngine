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
gRect::gRect() : left(0), top(0), right(0), bottom(0) {}

gRect::gRect(int left, int top, int right, int bottom) : left(left), top(top), right(right), bottom(bottom) {}

gRect::gRect(const gRect& r) : left(r.left), top(r.top), right(r.right), bottom(r.bottom) {}

gRect::~gRect() {}

/**
 * Sets edges of the rectangle.
 * @param left left edge to be set
 * @param top top edge to be set
 * @param right right edge to be set
 * @param bottom bottom edge to be set
 */
void gRect::set(int l, int t, int r, int b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

/**
 * Sets edges of the rectangle with the given rectangle
 * @param r Rectangle to be set
 */
void gRect::set(const gRect& r)
{
	left = r.left;
	top = r.top;
	right = r.right;
	bottom = r.bottom;
}

/**
 * Checks whether of intersection given 2 rectangle.
 * @param r1left left edge of the first rectangle
 * @param r1top top edge of the first rectangle
 * @param r1right right edge of the first rectangle
 * @param r1bottom bottom edge of the first rectangle
 * @param r2left left edge of the second rectangle
 * @param r2top top edge of the second rectangle
 * @param r2right right edge of the second rectangle
 * @param r2bottom bottom edge of the second rectangle
 * @return true if there is intersection false if not.
 */
bool gRect::intersects(int l1, int t1, int r1, int b1, int l2, int t2, int r2, int b2) {return l1 < r2 && r1 > l2 && t1 < b2 && b1 > t2;}

/**
 * Checks whether of intersection given 2 rectangle.
 * @param r1 first rectangle to be check.
 * @param r2 second rectangle to be check.
 * @return true if there is intersection false if not.
 */
bool gRect::intersects(const gRect& r1, const gRect& r2) { return intersects(r1.left, r1.top, r1.right, r1.bottom, r2.left, r2.top, r2.right, r2.bottom); }

/**
 * Checks whether of intersection with given rectangle.
 * @param left left edge of the given rectangle
 * @param top top edge of the given rectangle
 * @param right right edge of the given rectangle
 * @param bottom bottom edge of the given rectangle
 * @return true if there is intersection false if not.
 */
bool gRect::intersects(int l, int t, int r, int b) { return intersects(left, top, right, bottom, l, t, r, b); }

/**
 * Checks whether of intersection with given rectangle.
 * @param r rectangle to be check.
 * @return true if there is intersection false if not.
 */
bool gRect::intersects(const gRect& r) { return intersects(r.left, r.top, r.right, r.bottom); }

/**
 * Checks if second rectangles edges are inside or equal of the first rectangle.
 * @param r1left left edge of the first rectangle
 * @param r1top top edge of the first rectangle
 * @param r1right right edge of the first rectangle
 * @param r1bottom bottom edge of the first rectangle
 * @param r2left left edge of the second rectangle
 * @param r2top top edge of the second rectangle
 * @param r2right right edge of the second rectangle
 * @param r2bottom bottom edge of the second rectangle
 * @return true if second rectangle is inside of first rectangle false if not.
 */
bool gRect::contains(int l1, int t1, int r1, int b1, int l2, int t2, int r2, int b2) { return l1 <= l2 && r1 >= r2 && t1 <= t2 && b1 >= b2; }

/**
 * Checks if second rectangles edges are inside or equal of the first rectangle.
 * @param r1 first rectangle to be check.
 * @param r2 second rectangle to be check.
 * @return true if second rectangle is inside of first rectangle false if not.
 */
bool gRect::contains(const gRect& r1, const gRect& r2) { return contains(r1.left, r1.top, r1.right, r1.bottom, r2.left, r2.top, r2.right, r2.bottom); }

/**
 * Checks if given rectangles edges are inside or equal of the current rectangle.
 * @param left left edge of the given rectangle
 * @param top top edge of the given rectangle
 * @param right right edge of the given rectangle
 * @param bottom bottom edge of the given rectangle
 * @return true if given rectangle is inside of current rectangle false if not.
 */
bool gRect::contains(int l, int t, int r, int b) { return contains(left, top, right, bottom, l, t, r, b); }

/**
 * Checks if given rectangles edges are inside or equal of the current rectangle.
 * @param r rectangle to be check.
 * @return true if given rectangle is inside of current rectangle false if not.
 */
bool gRect::contains(const gRect& r) { return contains(r.left, r.top, r.right, r.bottom); }

/**
 * Checks if given point are inside of the current rectangle.
 * @param x X Position of the given point
 * @param y Y Position of the given point
 * @return true if given point is inside of current rectangle false if not.
 */
bool gRect::contains(int x, int y) { return contains(x, y, x, y); }
