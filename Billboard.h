#ifndef BILLBOARD_H
#define BILLBOARD_H

#include "Point.h"

class Image;

#define MAXBILLBOARDS 20 //max billboards per texture

//billboard type for billboards defined in the environment file
class Billboard
{
public:
  int numOfBoards; //how many billboards from env. file
  int xpos[MAXBILLBOARDS], zpos[MAXBILLBOARDS];
  int width, height;
  void Draw(double dt, int xLength, int zLength, float scaleFactor, const Image * const map, const Point3D &playerPos);
};

#endif