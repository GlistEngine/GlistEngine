/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GRAPHICS_GRECT_H_
#define GRAPHICS_GRECT_H_
/**
 * gRect holds six integer for a rectangle. The rectangle is represented by the coordinates of its four edges(left, top, right, bottom).
 * These fields can be accessed via their respective functions(e.g. to acces left edge of the rectangle use left()).
 * Use getWidth() and getHeight() to retrieve the rectangles width and height.
 * To set edges of the rectangle use one of the set function.
 */
class gRect {
	private:
	int l, r, t, b;
	int width, height;
	public:
	/**
	 * Creates empty rectangle
	 */
	gRect();
	/**
	 * Creates rectangle with given edges.
	 * @param left 	 Left edge of the rectangle to be created.
	 * @param top 	 Top edge of the rectangle to be created.
	 * @param right  Right edge of the rectangle to be created.
	 * @param bottom Bottom edge of the rectangle to be created.
	 */
	gRect(int left, int top, int right, int bottom);
	/**
	 * Creates copy of the given rectangle.
	 * @param r Rectangle to be copied.
	 */
	gRect(const gRect& r);
	virtual ~gRect();

	/**
	 * Sets edges of the rectangle.
	 * @param left 	 Left edge to be set
	 * @param top 	 Top edge to be set
	 * @param right  Right edge to be set
	 * @param bottom Bottom edge to be set
	 */
	void set(int left, int top, int right, int bottom);
	/**
	 * Sets edges of the rectangle with the given rectangle
	 * @param r Rectangle to be set
	 */
	void set(const gRect& r);

	/**
	 * Checks whether of intersection given 2 rectangle.
	 * @param left1   Left edge of the first rectangle
	 * @param top1 	  Top edge of the first rectangle
	 * @param right1  Right edge of the first rectangle
	 * @param bottom1 Bottom edge of the first rectangle
	 * @param left2   Left edge of the second rectangle
	 * @param top2 	  Top edge of the second rectangle
	 * @param right2  Right edge of the second rectangle
	 * @param bottom2 Bottom edge of the second rectangle
	 * @return true if there is intersection false if not.
	 */
	static bool intersects(int left1, int top1, int right1, int bottom1, int left2, int top2, int right2, int bottom2);
	/**
	 * Checks whether of intersection given 2 rectangle.
	 * @param r1 First rectangle to be check.
	 * @param r2 Second rectangle to be check.
	 * @return true if there is intersection false if not.
	 */
	static bool intersects(const gRect& r1, const gRect& r2);
	/**
	 * Checks whether of intersection with given rectangle.
	 * @param left 	 Left edge of the given rectangle
	 * @param top 	 Top edge of the given rectangle
	 * @param right  Right edge of the given rectangle
	 * @param bottom Bottom edge of the given rectangle
	 * @return true if there is intersection false if not.
	 */
	bool intersects(int left, int top, int right, int bottom);
	/**
	 * Checks whether of intersection with given rectangle.
	 * @param r Rectangle to be check.
	 * @return true if there is intersection false if not.
	 */
	bool intersects(const gRect& r);

	/**
	 * Checks if second rectangles edges are inside or equal of the first rectangle.
	 * @param left1   Left edge of the first rectangle
	 * @param top1 	  Top edge of the first rectangle
	 * @param right1  Right edge of the first rectangle
	 * @param bottom1 Bottom edge of the first rectangle
	 * @param left2   Left edge of the second rectangle
	 * @param top2 	  Top edge of the second rectangle
	 * @param right2  Right edge of the second rectangle
	 * @param bottom2 Bottom edge of the second rectangle
	 * @return true if second rectangle is inside of first rectangle false if not.
	 */
	static bool contains(int left1, int top1, int right1, int bottom1, int left2, int top2, int right2, int bottom2);
	/**
	 * Checks if second rectangles edges are inside or equal of the first rectangle.
	 * @param r1 First rectangle to be check.
	 * @param r2 Second rectangle to be check.
	 * @return true if second rectangle is inside of first rectangle false if not.
	 */
	static bool contains(const gRect& r1, const gRect& r2);
	/**
	 * Checks if given rectangles edges are inside or equal of the current rectangle.
	 * @param left 	 Left edge of the given rectangle
	 * @param top 	 Top edge of the given rectangle
	 * @param right  Right edge of the given rectangle
	 * @param bottom Bottom edge of the given rectangle
	 * @return true if given rectangle is inside of current rectangle false if not.
	 */
	bool contains(int left, int top, int right, int bottom);
	/**
	 * Checks if given rectangles edges are inside or equal of the current rectangle.
	 * @param r Rectangle to be check.
	 * @return true if given rectangle is inside of current rectangle false if not.
	 */
	bool contains(const gRect& r);
	/**
	 * Checks if given point are inside of the current rectangle.
	 * @param x X Position of the given point
	 * @param y Y Position of the given point
	 * @return true if given point is inside of current rectangle false if not.
	 */
	bool contains(int x, int y);

	/**
	 * Returns left edge of the rectangle.
	 * @return Left edge of the rectangle.
	 */
	int left() const;
	/**
	 * Returns top edge of the rectangle.
	 * @return Top edge of the rectangle.
	 */
	int top() const;
	/**
	 * Returns right edge of the rectangle.
	 * @return Right edge of the rectangle.
	 */
	int right() const;
	/**
	 * Returns bottom edge of the rectangle.
	 * @return Bottom edge of the rectangle.
	 */
	int bottom() const;
	/**
	 * Returns width of the rectangle.
	 * @return Width of the rectangle.
	 */
	int getWidth()  const;
	/**
	 * Returns height of the rectangle.
	 * @return Height of the rectangle.
	 */
	int getHeight() const;
};

#endif /* GRAPHICS_GRECT_H_ */
