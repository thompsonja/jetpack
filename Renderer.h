#ifndef RENDERER_H
#define RENDERER_H

#include <Windows.h>
#include <glut.h>
#include <map>
#include <memory>

class EnergyBar;
class SphereRing;
class Point3D;

class Renderer
{
public:
  Renderer(int width, int height, int XLEN, int ZLEN);
  void InitFog(float fogEnd);
  void InitLight();
  void Render2D(double dt);
  void Render3D(double dt, const Point3D &playerPosition);
  void AddSphereRing(std::shared_ptr<SphereRing> ring);
  EnergyBar *healthBar;
  EnergyBar *jetpackBar;
  GLuint ringLightingNotPassedList;
  GLuint ringLightingPassedList;
private:
  void DrawEnergyBar(EnergyBar *bar, int x, int y, int width, int height);
  void DrawSphereRings(double dt, const Point3D &playerPosition);
  int width;
  int height;
  int XLEN;
  int ZLEN;
  std::map<std::shared_ptr<SphereRing>, GLuint> ringLists;
};

#endif