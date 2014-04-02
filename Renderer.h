#ifndef RENDERER_H
#define RENDERER_H

#include <Windows.h>
#include <glut.h>
#include <map>

class EnergyBar;
class SphereRing;
class Point3D;

class Renderer
{
public:
  Renderer(int width, int height);
  void Render2D(double dt);
  void Render3D(double dt, const Point3D &playerPosition);
  void AddSphereRing(SphereRing *ring);
  EnergyBar *healthBar;
  EnergyBar *jetpackBar;
  GLuint ringLightingNotPassedList;
  GLuint ringLightingPassedList;
private:
  void DrawEnergyBar(EnergyBar *bar, int x, int y, int width, int height);
  void DrawSphereRings(double dt, const Point3D &playerPosition);
  int width;
  int height;
  std::map<SphereRing*, GLuint> ringLists;
};

#endif