#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "Point.h"
#include <memory>

class Image;

//billboard type for billboards defined in the environment file
class Billboard
{
public:
  Billboard(Point3D position);
  void Draw(double dt, int xLength, int zLength, float scaleFactor, const std::shared_ptr<Image> &map, const Point3D &playerPos);
  int width, height;
private:
  Point3D position;
};

#endif