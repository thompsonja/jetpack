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

User::User():healthBar(), jetPack()
{
	x = 0;
	y = 0;
	z = 0;
	height = 6;
	rad = 1.5;
	onBox = false;
}

void User::updatePos()
{
	glLoadIdentity();
	
	//rotates and moves the camera
	glRotatef(dy*mouseYsens, 1, 0, 0);
	glRotatef(dx*mouseXsens, 0, 1, 0);
	glRotatef(180 - terrainAngle*180/M_PI, 0, 1, 0);
	
	glTranslatef(-x, -y, -z);
}

void User::moveForward()
{
	x += 300.0/FPS*.1*speed*sin((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle);
	y += 300.0/FPS*.1*speed*sin((dy*fabs(mouseYsens))*M_PI/180);
	z += 300.0/FPS*.1*speed*cos((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle);
}

void User::moveBackward()
{
	x -= 300.0/FPS*.1*speed*sin((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle);
	y -= 300.0/FPS*.1*speed*sin((dy*fabs(mouseYsens))*M_PI/180);
	z -= 300.0/FPS*.1*speed*cos((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle);
}

void User::strafeLeft()
{
	x += 300.0/FPS*.1*speed*cos((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle);
	z -= 300.0/FPS*.1*speed*sin((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle);
}

void User::strafeRight()
{
	x -= 300.0/FPS*.1*speed*cos((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle);
	z += 300.0/FPS*.1*speed*sin((dx*fabs(mouseXsens))*M_PI/180 + terrainAngle);
}

float User::getX()
{
	return x;
}

float User::getY()
{
	return y;
}

float User::getZ()
{
	return z;
}

float User::getHeight()
{
	return height;
}

float User::getRad()
{
	return rad;
}

void User::crouch()
{
	height = 4;
}

void User::uncrouch()
{
	height = 6;
}

void User::setX(float x)
{
	this->x = x;
}

void User::setY(float y)
{
	this->y = y;
}

void User::setZ(float z)
{
	this->z = z;
}