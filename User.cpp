#include "User.h"
#include <Windows.h>
#include <glut.h>
#include <math.h>

extern float dx;
extern float dy;
extern float mouseXsens;
extern float mouseYsens;
extern float FPS;
extern float speed;
extern float terrainAngle;

User::User() : 
  healthBar(),
  jetPack(),
  position(0, 0, 0),
  height(6),
  rad(1.5),
  OnBox(false)
{
}

void User::UpdatePos()
{
	glLoadIdentity();
	
	//rotates and moves the camera
	glRotatef(dy*mouseYsens, 1, 0, 0);
	glRotatef(dx*mouseXsens, 0, 1, 0);
	glRotatef(180 - terrainAngle*180/M_PI, 0, 1, 0);
	
	glTranslatef(-GetX(), -GetY(), -GetZ());
}

void User::MoveForward()
{
  position.SetX(position.GetX() + 300.0/FPS*.1*speed*sin((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle));
	position.SetY(position.GetY() + 300.0/FPS*.1*speed*sin((dy*fabs(mouseYsens))*M_PI/180));
	position.SetZ(position.GetZ() + 300.0/FPS*.1*speed*cos((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle));
}

void User::MoveBackward()
{
	position.SetX(position.GetX() - 300.0/FPS*.1*speed*sin((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle));
	position.SetY(position.GetY() - 300.0/FPS*.1*speed*sin((dy*fabs(mouseYsens))*M_PI/180));
	position.SetZ(position.GetZ() - 300.0/FPS*.1*speed*cos((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle));
}

void User::StrafeLeft()
{
	position.SetX(position.GetX() + 300.0/FPS*.1*speed*cos((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle));
	position.SetZ(position.GetZ() - 300.0/FPS*.1*speed*sin((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle));
}

void User::StrafeRight()
{
	position.SetX(position.GetX() - 300.0/FPS*.1*speed*cos((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle));
	position.SetZ(position.GetZ() + 300.0/FPS*.1*speed*sin((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle));
}

double User::GetX()
{
	return position.GetX();
}

double User::GetY()
{
	return position.GetY();
}

double User::GetZ()
{
	return position.GetZ();
}

float User::GetHeight()
{
	return height;
}

float User::GetRad()
{
	return rad;
}

void User::Crouch()
{
	height = 4;
}

void User::Uncrouch()
{
	height = 6;
}

void User::SetX(double x)
{
	position.SetX(x);
}

void User::SetY(double y)
{
	position.SetY(y);
}

void User::SetZ(double z)
{
	position.SetZ(z);
}

const Point3D& User::GetPosition()
{
  return position;
}