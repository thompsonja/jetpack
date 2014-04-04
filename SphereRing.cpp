#include "SphereRing.h"
#include "User.h"

#include <math.h>

SphereRing::SphereRing(const Point3D &position, double rotationRate, int num, double sphRad, double rRad) :
  numSpheres(num),
  sphereRadius(sphRad),
  ringRadius(rRad),
  rotationRate(rotationRate),
  rotation(0),
  ringPosition(position),
  passed(false)
{
}

bool SphereRing::UpdatePassedStatus(const Point3D &playerPosition, int XLEN, int ZLEN)
{
  bool currentlyPassed = ((fabs(playerPosition.GetX() - ringPosition.GetX()*XLEN) < ringRadius) && 
                          (fabs(playerPosition.GetY() - ringPosition.GetY())      < ringRadius) && 
                          (fabs(playerPosition.GetZ() - ringPosition.GetZ()*ZLEN) < sphereRadius));
  bool newlyPassed = currentlyPassed && !isPassed();
  if(newlyPassed)
  {
    SetPassed(true);
  }
  return newlyPassed;
}

void SphereRing::UpdateRotation(double dt)
{
  rotation += rotationRate * dt;

  if (rotation > 360)
    rotation -= 360;
}

bool SphereRing::isPassed()
{
	return passed;
}

void SphereRing::SetPassed(bool passed)
{
  this->passed = passed;
}
