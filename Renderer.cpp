#include "Renderer.h"
#include "EnergyBar.h"
#include "SphereRing.h"
#include "Point.h"

Renderer::Renderer(int width, int height, int XLEN, int ZLEN) :
  width(width),
  height(height),
  XLEN(XLEN),
  ZLEN(ZLEN)
{
}

void Renderer::InitFog(float fogEnd)
{
  GLuint	fogMode  [ ] = { GL_EXP, GL_EXP2, GL_LINEAR };	// Storage For Three Types Of Fog
  GLfloat	fogColor [4] = {0.8519f, 0.8588f, 0.8882f, 1}; 

  glFogi(GL_FOG_MODE, fogMode[2]);
  glFogfv(GL_FOG_COLOR, fogColor);
  glFogf(GL_FOG_DENSITY, 0.01f);
  glHint(GL_FOG_HINT, GL_NICEST);
  glFogf(GL_FOG_START, 0);
  glFogf(GL_FOG_END  , fogEnd);
}

void Renderer::InitLight()
{
  //Position is set in camera function

  //light settings
  GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 0.5f}; 
  GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat globalAmbient[] = {0.6f, 0.6f, 0.6f, 1.0f};

  glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);	
  glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);	

  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);	
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

  glEnable(GL_LIGHT0);

  //disabling this seemed to make the light look better to me, personally
  //	glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
  //	glEnable(GL_COLOR_MATERIAL);

  //I moved this to the camera function to have the light position stay the same
  //regardless of camera position, etc.
  //	glLightfv(GL_LIGHT0, GL_POSITION,LightPosition);
}

void Renderer::Render2D(double dt)
{
  GLboolean light_enabled = glIsEnabled(GL_LIGHTING);

  //sets up the projection for drawing text on the screen
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();  
  glOrtho(0, width, height, 0, 0, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_LIGHTING);							//disable light for drawing text
  glLineWidth(1.0);
  //reset the matrices
  glBegin(GL_LINES);
  glVertex3f(width/2 + 9, (float)height/2, 0.0f);
  glVertex3f(width/2 - 8, (float)height/2, 0.0f);
  glVertex3f((float)width/2, height/2 + 8, 0.0f);
  glVertex3f((float)width/2, height/2 - 9, 0.0f);
  glEnd();

  DrawEnergyBar(jetpackBar, 5, 5, width / 5, height / 40);
  DrawEnergyBar(healthBar, 5, 5 + height / 40, width / 5, height / 40);

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  if(light_enabled)
    glEnable(GL_LIGHTING);
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

 void Renderer::AddSphereRing(std::shared_ptr<SphereRing> ring)
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
   for(auto i = ringLists.begin(); i != ringLists.end(); i++)
   {
     std::shared_ptr<SphereRing> ring = i->first;
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