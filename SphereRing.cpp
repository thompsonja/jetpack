#include "SphereRing.h"
#include "User.h"

#include <math.h>

extern GLuint lightList[7];
extern int XLEN;
extern int ZLEN;

SphereRing::SphereRing(const Point3D &position, double rotationRate, int num, double sphRad, double rRad) :
  numSpheres(num),
  sphereRadius(sphRad),
  ringRadius(rRad),
  rotationRate(rotationRate),
  rotation(0),
  ringPosition(position),
  whichList(4)
{
}

void SphereRing::drawList()
{
	GLUquadricObj *quadratic;
	
	quadratic=gluNewQuadric();	
	gluQuadricOrientation(quadratic, GLU_OUTSIDE);
	gluQuadricNormals(quadratic, GLU_SMOOTH);
	
	float theta = 360.0/numSpheres;
	
	ringList = glGenLists(1);
	glNewList(ringList, GL_COMPILE);

	for(int i = 0; i < numSpheres; i++)
	{
		glPushMatrix();
		glRotatef(i*theta, 0, 0, 1);
		glTranslatef(ringRadius, 0, 0);
		gluSphere(quadratic, sphereRadius, 16, 16);
		glPopMatrix();
	}
	glEndList();
}

bool SphereRing::UpdatePassedStatus(const Point3D &playerPosition)
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

void SphereRing::drawRing(double dt)
{
	float theta = 360.0/numSpheres;
	GLUquadricObj *quadratic;

	quadratic=gluNewQuadric();	
	gluQuadricOrientation(quadratic, GLU_OUTSIDE);
	gluQuadricNormals(quadratic, GLU_SMOOTH);

	glDisable(GL_CULL_FACE);

	glPushMatrix();

	glTranslatef(ringPosition.GetX()*XLEN, ringPosition.GetY(), ringPosition.GetZ()*ZLEN);
	glRotatef(rotation, 0, 0, 1);

	if(glIsEnabled(GL_LIGHTING))
		glCallList(lightList[whichList]);
	
	glCallList(ringList);
	glPopMatrix();

  UpdateRotation(dt);

	glEnable(GL_CULL_FACE);
}

bool SphereRing::isPassed()
{
	return whichList != 4;
}

void SphereRing::SetPassed(bool passed)
{
  whichList = passed ? 5 : 4;
}
