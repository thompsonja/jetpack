#ifndef BOX_H
#define BOX_H

#include "Point.h"

class Box
{
public:
  Box(float xlen, float ylen, float zlen, const Point3D &position);
  void setExistence(bool a);
  bool Exists();
  void UpdatePosition(double dt);
  const Point3D& GetPosition(){return position;}
  float GetXlen(){return xlen;}
  float GetYlen(){return ylen;}
  float GetZlen(){return zlen;}
  int GetSide(){return side;}
private:
	bool exists;
	float xlen;
	float ylen;
	float zlen;
  Point3D position;
	int side;
};

#endif