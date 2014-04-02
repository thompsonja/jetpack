#ifndef BOX_H
#define BOX_H

#include "Point.h"

class Box
{
private:
	bool exists;
	float xlen;
	float ylen;
	float zlen;
  Point3D position;
	int side;
public:
	Box(float xlen, float ylen, float zlen, const Point3D &position);
	void setExistence(bool a);
	bool Exists();
	void draw(float FPS);
};

#endif