#ifndef SPHERERING_H
#define SPHERERING_H

#include <Windows.h>
#include <glut.h>

class SphereRing
{
public:
	SphereRing(int x, int y, int z, float ang, int num, float sphRad, float rRad);
  void drawList();
  void drawRing(float FPS);
  void setList(int list);
  int isPassed();
  static int numOfRings;
  static int ringsPassed;
private:
  GLuint ringList;
  int numOfSpheres;
  float sphereRad;
  float ringRad;
  float angle;
  float tempAngle;
  int ringX;
  int ringY;
  int ringZ;
  int whichList;
};

#endif