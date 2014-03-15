#ifndef SPHERERING_H
#define SPHERERING_H

#include <Windows.h>
#include <glut.h>

class SphereRing
{
private:
	GLuint ringList;
	int numOfSpheres;
	float sphereRad;
	float ringRad;
	float angle;
	float tempAngle;
	int ringX;
	int ringY;
	int ringZ;
	int whichList;
public:
	SphereRing *previous, *next;
	SphereRing(int x, int y, int z, float ang, int num, float sphRad, float rRad);
	void insert();
	void drawList();
	void drawRing(float FPS);
	void setList(int list);
	int isPassed();
	static int numOfRings;
	static int ringsPassed;
};

#endif