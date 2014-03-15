#ifndef USER_H
#define USER_H

#include "EnergyBar.h"

class User
{
private:
	float x;
	float y;
	float z;
	float height;
	float rad;
public:
	User();
	EnergyBar healthBar;
	EnergyBar jetPack;
	void updatePos();
	void moveForward();
	void moveBackward();
	void strafeLeft();
	void strafeRight();
	float getX();
	float getY();
	float getZ();
	float getHeight();
	float getRad();
	void crouch();
	void uncrouch();
	void setX(float x);
	void setY(float y);
	void setZ(float z);
	bool onBox;
};

#endif