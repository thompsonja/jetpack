#include "SphereRing.h"
#include "User.h"
#include <math.h>

extern SphereRing *currentRing, *lastRing;
extern User player;
extern GLuint lightList[7];
extern bool passedSound;
extern int XLEN;
extern int ZLEN;

int SphereRing::numOfRings = 0;
int SphereRing::ringsPassed = 0;

SphereRing::SphereRing(int x, int y, int z, float ang, int num, float sphRad, float rRad)
{
	numOfSpheres = num;
	angle = ang;
	sphereRad = sphRad;
	ringRad = rRad;
	ringX = x;
	ringY = y;
	ringZ = z;
	tempAngle = 0;
	whichList = 4;
	SphereRing::numOfRings++;
}

void SphereRing::drawList()
{
	GLUquadricObj *quadratic;
	
	quadratic=gluNewQuadric();	
	gluQuadricOrientation(quadratic, GLU_OUTSIDE);
	gluQuadricNormals(quadratic, GLU_SMOOTH);
	
	float theta = 360.0/numOfSpheres;
	
	ringList = glGenLists(1);
	glNewList(ringList, GL_COMPILE);

	for(int i = 0; i < numOfSpheres; i++)
	{
		glPushMatrix();
		glRotatef(i*theta, 0, 0, 1);
		glTranslatef(ringRad, 0, 0);
		gluSphere(quadratic, sphereRad, 16, 16);
		glPopMatrix();
	}
	glEndList();
}

void SphereRing::drawRing(float FPS)
{
	float theta = 360.0/numOfSpheres;
	GLUquadricObj *quadratic;

	quadratic=gluNewQuadric();	
	gluQuadricOrientation(quadratic, GLU_OUTSIDE);
	gluQuadricNormals(quadratic, GLU_SMOOTH);

	if((fabs(player.getX() - ringX*XLEN) < ringRad) && (fabs(player.getY() - ringY) < ringRad)
					&& (fabs(player.getZ() - ringZ*ZLEN) < sphereRad))
	{
		if(whichList == 4)
		{
			whichList = 5;
			passedSound = true;
		}
	}

	glDisable(GL_CULL_FACE);

	glPushMatrix();

	glTranslatef(ringX*XLEN, ringY, ringZ*ZLEN);
	glRotatef(tempAngle, 0, 0, 1);

	if(glIsEnabled(GL_LIGHTING))
		glCallList(lightList[whichList]);
	
	glCallList(ringList);
	glPopMatrix();

	tempAngle += angle/FPS;

	if (tempAngle > 360)
		tempAngle -= 360;

	glEnable(GL_CULL_FACE);
}

void SphereRing::setList(int list)
{
	whichList = list;
}

int SphereRing::isPassed()
{
	return whichList - 4;
}
