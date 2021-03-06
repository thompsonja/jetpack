#include "Billboard.h"
#include "Image.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <GL/glut.h>
#include <math.h>

Billboard::Billboard(Point3D position) :
  position(position)
{
}

void Billboard::Draw(double dt, int xLength, int zLength, float scaleFactor, const std::shared_ptr<Image> &map, const Point3D &playerPos)
{
  int xPos = position.GetX();
  int zPos = position.GetZ();
  float lookAt[2]; //vector between the camera and the billboard, projected to the xz axis
  //gets the vector between the camera and the billboard, and calculates the angle
  lookAt[0] = (xPos - 1)*xLength - playerPos.GetX(); //x component of lookAt vector
  lookAt[1] = (zPos - 1)*zLength - playerPos.GetZ(); //z component of lookAt vector
  float angle = atan(lookAt[0]/lookAt[1])*180/M_PI;

  //this takes care of sign issues
  if (playerPos.GetZ() > (zPos - 1)*zLength)
    angle = 180 + angle;

  //move the billboard into position and scales it
  glPushMatrix();
  glTranslatef((xPos - 1)*xLength, map->grayValues[xPos - 1][zPos - 1], (zPos - 1)*zLength);
  glScalef(scaleFactor, scaleFactor, scaleFactor);
  //note that the rotation angle is such that the billboard isn't facing straight at you, but at
  //a 45 degree angle, and two such rectangles are used.  It kind of looks like: viewer---->X
  //where the X represents the two crossing billboards and the arrow is the line of sight.
  glRotatef(angle + 45, 0, 1, 0);

  //draw the first rectangle, I have the rectangles be essentially the size of the quad
  glBegin(GL_QUADS);
  glTexCoord2f(1.0f, 0.0f);
  glNormal3f(0, 0, -1);
  glVertex3f(-(float)xLength/2, 0, 0);

  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(-(float)xLength/2, xLength*(float)height/width, 0);

  glTexCoord2f(0.0f, 1.0f);
  glVertex3f((float)xLength/2, xLength*(float)height/width, 0);

  glTexCoord2f(0.0f, 0.0f);
  glVertex3f((float)xLength/2, 0, 0);
  glEnd();

  //rotate and draw the second rectangle
  glRotatef(-90, 0, 1, 0);
  glBegin(GL_QUADS);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(-(float)xLength/2, 0, 0);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(-(float)xLength/2, xLength*(float)height/width, 0);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f((float)xLength/2, xLength*(float)height/width, 0);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f((float)xLength/2, 0, 0);
  glEnd();
  glPopMatrix();
}
