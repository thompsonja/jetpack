#include "Light.h"
#include <Windows.h>
#include <GL/glut.h>

void Light::SetGlLight()
{
  glMaterialfv( GL_FRONT/*_AND_BACK*/, GL_AMBIENT, ambient );
  glMaterialfv( GL_FRONT/*_AND_BACK*/, GL_DIFFUSE, diffuse );
  glMaterialfv( GL_FRONT/*_AND_BACK*/, GL_SPECULAR, specular );
  glMaterialfv( GL_FRONT/*_AND_BACK*/, GL_SPECULAR, emission );
  glMaterialf ( GL_FRONT/*_AND_BACK*/, GL_SHININESS, shininess );
}