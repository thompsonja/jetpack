#include "Box.h"

Box::Box(float xlen, float ylen, float zlen, const Point3D &position) :
  xlen(xlen),
  ylen(ylen),
  zlen(zlen),
  position(position),
  side(0)
{
}

void Box::UpdatePosition(double dt)
{
  auto x = position.GetX();
  auto y = position.GetY();
  auto z = position.GetZ();

  if(side == 0)
  {
    x += 30*dt;
    if(x + xlen/2 >= 381)
      side = 1;
  }
  if(side == 1)
  {
    z += 30*dt;
    if(z + zlen/2 >= 381)
      side = 2;
  }
  if(side == 2)
  {
    x -= 30*dt;
    if(x - xlen/2 <= 0)
      side = 3;
  }
  if(side == 3)
  {
    z -= 30*dt;
    if(z - zlen/2 <= 0)
      side = 0;
  }

  position.Set(x, y, z);
}

void Box::setExistence(bool a)
{
	exists = a;
}

bool Box::Exists()
{
	return exists;
}