#ifndef RENDERER_H
#define RENDERER_H

class EnergyBar;

class Renderer
{
public:
  Renderer(int width, int height);
  void Render(double dt);
  EnergyBar *healthBar;
  EnergyBar *jetpackBar;
private:
  void DrawEnergyBar(EnergyBar *bar, int x, int y, int width, int height);
  int width;
  int height;
};

#endif