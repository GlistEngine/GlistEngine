/*
 * gRect.h
 *
 *  Created on: 9 Ara 2020
 *      Author: Emrullah
 */

#ifndef GRAPHICS_GRECT_H_
#define GRAPHICS_GRECT_H_

class gRect {
public:
	int left, right, top, bottom;
	gRect();
	gRect(int left, int top, int right, int bottom);
	gRect(const gRect& r);
	virtual ~gRect();

	void set(int left, int top, int right, int bottom);
	void set(const gRect& r);

	static bool intersects(int left1, int top1, int right1, int bottom1, int left2, int top2, int right2, int bottom2);
	static bool intersects(const gRect& r1, const gRect& r2);
	bool intersects(int left, int top, int right, int bottom);
	bool intersects(const gRect& r);

	static bool contains(int left1, int top1, int right1, int bottom1, int left2, int top2, int right2, int bottom2);
	static bool contains(const gRect& r1, const gRect& r2);
	bool contains(int left, int top, int right, int bottom);
	bool contains(const gRect& r);
	bool contains(int x, int y);

	inline int getWidth()  const { return right - left; }
	inline int getHeight() const { return bottom - top; }
};

#endif /* GRAPHICS_GRECT_H_ */
