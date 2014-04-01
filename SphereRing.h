#ifndef SPHERERING_H
#define SPHERERING_H

#include <Windows.h>
#include <glut.h>
#include "Point.h"

class SphereRing
{
public:
	SphereRing(const Point3D &position, double rotationRate, int num, double sphRad, double rRad);
  void drawList();
  bool UpdatePassedStatus(const Point3D &playerPosition);
  void UpdateRotation(double dt);
  void drawRing(double dt);
  void setList(int list);
  bool isPassed();
  void SetPassed(bool passed);
  int GetNumSpheres(){return numSpheres;}
  double GetSphereRadius(){return sphereRadius;}
  double GetRingRadius(){return ringRadius;}
  const Point3D& GetPosition(){return ringPosition;}
  double GetRotation(){return rotation;}
private:
  GLuint ringList;
  int numSpheres;
  double sphereRadius;
  double ringRadius;
  double rotationRate;
  double rotation;
  Point3D ringPosition;
  int whichList;
};

#endif