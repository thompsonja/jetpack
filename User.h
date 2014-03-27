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
	void UpdatePos(double dx, double dy);
	void MoveForward(double dt, double dx, double dy);
	void MoveBackward(double dt, double dx, double dy);
	void StrafeLeft(double dt, double dx, double dy);
	void StrafeRight(double dt, double dx, double dy);
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
  const Point3D& GetPosition();
  bool IsJumping();
  void SetJumping(bool val);
  bool IsOnBox();
  void SetOnBox(bool val);
private:
  Point3D position;
	float height;
	float rad;
  bool isJumping;
  bool isOnBox;
  float speed;
};

#endif