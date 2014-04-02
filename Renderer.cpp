#include "Renderer.h"
#include "EnergyBar.h"
#include "SphereRing.h"
#include "Point.h"

extern int XLEN;
extern int ZLEN;

Renderer::Renderer(int width, int height) :
  width(width),
  height(height)
{
}

void Renderer::Render2D(double dt)
{
  DrawEnergyBar(jetpackBar, 5, 5, width / 5, height / 40);
  DrawEnergyBar(healthBar, 5, 5 + height / 40, width / 5, height / 40);
}

void Renderer::Render3D(double dt, const Point3D &playerPosition)
{
  DrawSphereRings(dt, playerPosition);
}

void Renderer::DrawEnergyBar(EnergyBar *bar, int x, int y, int width, int height)
{
  if(bar == NULL)
    return;

	glColor3f(1, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(x, y, 0.0f);
	glVertex3f(x + width, y, 0.0f);
	glVertex3f(x + width, y + height, 0.0f);
	glVertex3f(x, y + height, 0.0f);
	glEnd();

  double energyRatio = bar->GetEnergy() / bar->GetMaxEnergy();

	if(energyRatio > 0.66)
		glColor4f(0, .5, 0, 1.0);
	else if(energyRatio <= 0.66 && bar->GetEnergy() > 0.33)
		glColor4f(1, 1, 0, 1.0);
	else if(energyRatio < 0.33)
		glColor4f(1, 0, 0, 1.0);

	glBegin(GL_LINES);
	for(int i = 1; i < bar->GetEnergy()*width; i++)
	{
		glVertex3f(x + i, y, 0.0f);
		glVertex3f(x + i, y + height - 1, 0.0f);
	}
	glEnd();
}

 void Renderer::AddSphereRing(SphereRing *ring)
 {
   if(ringLists.find(ring) != ringLists.end())
   {
     return;
     // TODO ERROR
   }

   GLuint list = glGenLists(1);
   ringLists[ring] = list;

   GLUquadricObj *quadratic;

   quadratic=gluNewQuadric();	
   gluQuadricOrientation(quadratic, GLU_OUTSIDE);
   gluQuadricNormals(quadratic, GLU_SMOOTH);

   float theta = 360.0/ring->GetNumSpheres();

   glNewList(list, GL_COMPILE);

   for(int i = 0; i < ring->GetNumSpheres(); i++)
   {
     glPushMatrix();
     glRotatef(i*theta, 0, 0, 1);
     glTranslatef(ring->GetRingRadius(), 0, 0);
     gluSphere(quadratic, ring->GetSphereRadius(), 16, 16);
     glPopMatrix();
   }
   glEndList();
 }

 void Renderer::DrawSphereRings(double dt, const Point3D &playerPosition)
 {
   for(std::map<SphereRing*, GLuint>::iterator i = ringLists.begin(); i != ringLists.end(); i++)
   {
     SphereRing *ring = i->first;
     if(ring == NULL)
     {
       // TODO error
       continue;
     }

     GLUquadricObj *quadratic;

     quadratic=gluNewQuadric();	
     gluQuadricOrientation(quadratic, GLU_OUTSIDE);
     gluQuadricNormals(quadratic, GLU_SMOOTH);

     glDisable(GL_CULL_FACE);

     glPushMatrix();

     glTranslatef(ring->GetPosition().GetX()*XLEN, ring->GetPosition().GetY(), ring->GetPosition().GetZ()*ZLEN);
     glRotatef(ring->GetRotation(), 0, 0, 1);

     if(glIsEnabled(GL_LIGHTING))
       glCallList(ring->isPassed() ? ringLightingPassedList : ringLightingNotPassedList);

     glCallList(i->second);
     glPopMatrix();

     glEnable(GL_CULL_FACE);
   }
 }