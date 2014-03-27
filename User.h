#ifndef USER_H
#define USER_H

#include "EnergyBar.h"
#include "Point.h"

class User
{
public:
	User();
	EnergyBar healthBar;
	EnergyBar jetPack;
	void  UpdatePos();
	void  MoveForward();
	void  MoveBackward();
	void  StrafeLeft();
	void  StrafeRight();
	double GetX();
	double GetY();
	double GetZ();
	float GetHeight();
	float GetRad();
	void Crouch();
	void Uncrouch();
	void SetX(double x);
	void SetY(double y);
	void SetZ(double z);
	bool OnBox;
  const Point3D& GetPosition();
private:
  Point3D position;
	float height;
	float rad;
};

#endif