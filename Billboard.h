#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "Point.h"

class Image;

//billboard type for billboards defined in the environment file
class Billboard
{
public:
  Billboard(Point3D position);
  void Draw(double dt, int xLength, int zLength, float scaleFactor, const Image * const map, const Point3D &playerPos);
  int width, height;
private:
  Point3D position;
};

#endif