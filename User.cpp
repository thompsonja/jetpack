#include "User.h"
#include <Windows.h>
#include <glut.h>
#include <math.h>

extern float terrainAngle;

User::User() : 
  healthBar(),
  jetPack(),
  position(0, 0, 0),
  height(6),
  rad(1.5),
  isOnBox(false),
  isJumping(false),
  speed(1)
{
}

void User::UpdatePos(double dx, double dy)
{
	glLoadIdentity();
	
	//rotates and moves the camera
	glRotatef(dy, 1, 0, 0);
	glRotatef(dx, 0, 1, 0);
	glRotatef(180 - terrainAngle*180/M_PI, 0, 1, 0);
	
	glTranslatef(-GetX(), -GetY(), -GetZ());
}

void User::MoveForward(double dt, double dx, double dy)
{
  position.SetX(position.GetX() + 30*dt*speed*sin(dx + terrainAngle));
	position.SetY(position.GetY() + 30*dt*speed*sin(dy));
	position.SetZ(position.GetZ() + 30*dt*speed*cos(dx + terrainAngle));
}

void User::MoveBackward(double dt, double dx, double dy)
{
	position.SetX(position.GetX() - 30*dt*speed*sin(dx + terrainAngle));
	position.SetY(position.GetY() - 30*dt*speed*sin(dy));
	position.SetZ(position.GetZ() - 30*dt*speed*cos(dx + terrainAngle));
}

void User::StrafeLeft(double dt, double dx, double dy)
{
	position.SetX(position.GetX() + 30*dt*speed*cos(dx + terrainAngle));
	position.SetZ(position.GetZ() - 30*dt*speed*sin(dx + terrainAngle));
}

void User::StrafeRight(double dt, double dx, double dy)
{
	position.SetX(position.GetX() - 30*dt*speed*cos(dx + terrainAngle));
	position.SetZ(position.GetZ() + 30*dt*speed*sin(dx + terrainAngle));
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
  speed = .4;
}

void User::Uncrouch()
{
	height = 6;
  speed = 1;
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

bool User::IsJumping()
{
  return isJumping;
}

void User::SetJumping(bool val)
{
  isJumping = val;
}

bool User::IsOnBox()
{
  return isOnBox;
}

void User::SetOnBox(bool val)
{
  isOnBox = val;
}