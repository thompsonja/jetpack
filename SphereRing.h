#ifndef SPHERERING_H
#define SPHERERING_H

#include <Windows.h>
#include <glut.h>

class Point3D;

class SphereRing
{
public:
	SphereRing(int x, int y, int z, double rotationRate, int num, double sphRad, double rRad);
  void drawList();
  bool UpdatePassedStatus(const Point3D &playerPosition);
  void drawRing(double dt);
  void setList(int list);
  bool isPassed();
  void SetPassed(bool passed);
  int GetNumSpheres(){return numSpheres;}
  double GetSphereRadius(){return sphereRadius;}
  double GetRingRadius(){return ringRadius;}
private:
  GLuint ringList;
  int numSpheres;
  double sphereRadius;
  double ringRadius;
  double rotationRate;
  double tempAngle;
  int ringX;
  int ringY;
  int ringZ;
  int whichList;
};

#endif