#include "Box.h"
#include <Windows.h>
#include <glut.h>
#include "User.h"
#include <math.h>

extern bool jumped;
extern User player;
extern float yVel;
extern float jumpHeight;

Box::Box(float xlen, float ylen, float zlen, float x, float y, float z)
{
	this->xlen = xlen;
	this->ylen = ylen;
	this->zlen = zlen;
	this->x = x;
	this->y = y;
	this->z = z;
	side = 0;
}

void Box::draw(float FPS)
{
	glBegin(GL_QUADS);
	glNormal3f(-1, 0, 0);
	glVertex3f(x + xlen/2, y + ylen/2, z + zlen/2);
	glVertex3f(x + xlen/2, y - ylen/2, z + zlen/2);
	glVertex3f(x + xlen/2, y - ylen/2, z - zlen/2);
	glVertex3f(x + xlen/2, y + ylen/2, z - zlen/2);
	glNormal3f(1, 0, 0);
	glVertex3f(x - xlen/2, y - ylen/2, z + zlen/2);
	glVertex3f(x - xlen/2, y + ylen/2, z + zlen/2);
	glVertex3f(x - xlen/2, y + ylen/2, z - zlen/2);
	glVertex3f(x - xlen/2, y - ylen/2, z - zlen/2);
	glNormal3f(0, 0, -1);
	glVertex3f(x - xlen/2, y + ylen/2, z - zlen/2);
	glVertex3f(x + xlen/2, y + ylen/2, z - zlen/2);
	glVertex3f(x + xlen/2, y - ylen/2, z - zlen/2);
	glVertex3f(x - xlen/2, y - ylen/2, z - zlen/2);
	glNormal3f(0, 0, 1);
	glVertex3f(x + xlen/2, y + ylen/2, z + zlen/2);
	glVertex3f(x - xlen/2, y + ylen/2, z + zlen/2);
	glVertex3f(x - xlen/2, y - ylen/2, z + zlen/2);
	glVertex3f(x + xlen/2, y - ylen/2, z + zlen/2);
	glNormal3f(0, -1, 0);
	glVertex3f(x - xlen/2, y - ylen/2, z - zlen/2);
	glVertex3f(x + xlen/2, y - ylen/2, z - zlen/2);
	glVertex3f(x + xlen/2, y - ylen/2, z + zlen/2);
	glVertex3f(x - xlen/2, y - ylen/2, z + zlen/2);
	glNormal3f(0, 1, 0);
	glVertex3f(x + xlen/2, y + ylen/2, z - zlen/2);
	glVertex3f(x - xlen/2, y + ylen/2, z - zlen/2);
	glVertex3f(x - xlen/2, y + ylen/2, z + zlen/2);
	glVertex3f(x + xlen/2, y + ylen/2, z + zlen/2);
	glEnd();

	if(side == 0)
	{
		x += 30/FPS;
		if(x + xlen/2 >= 381)
			side = 1;
	}
	if(side == 1)
	{
		z += 30/FPS;
		if(z + zlen/2 >= 381)
			side = 2;
	}
	if(side == 2)
	{
		x -= 30/FPS;
		if(x - xlen/2 <= 0)
			side = 3;
	}
	if(side == 3)
	{
		z -= 30/FPS;
		if(z - zlen/2 <= 0)
			side = 0;
	}

	if(fabs(player.GetY() - (y + ylen/2)) < player.GetHeight())
	{
		if(fabs(player.GetZ() - z) < zlen/2 && fabs(player.GetX() - x) < xlen/2)
		{
			if(yVel <= 0)
			{
				jumped = false;
				player.OnBox = true;
				player.SetY(y + ylen/2 + player.GetHeight());
				yVel = 0;
				if(side == 0)
					player.SetX(player.GetX() + 30/FPS);
				if(side == 1)
					player.SetZ(player.GetZ() + 30/FPS);
				if(side == 2)
					player.SetX(player.GetX() - 30/FPS);
				if(side == 3)
					player.SetZ(player.GetZ() - 30/FPS);

			}
		}
		else if(player.OnBox)
		{
			jumped = true;
			player.SetY(y + ylen/2 + player.GetHeight());
			player.OnBox = false;
			jumpHeight = y + ylen/2 + player.GetHeight();
		}
		else
		{
			if(fabs(player.GetZ() - z) < zlen/2)
			{
				if((player.GetX() - x > - xlen/2 - player.GetRad()) && (player.GetX() - x < 0))
				{
					player.SetX(x - xlen/2 - player.GetRad());
				}
				else if(player.GetX() - x < xlen/2 + player.GetRad())
				{
					if(player.GetX() - x > 0)
						player.SetX(x + xlen/2 + player.GetRad());
				}
			}
			if(fabs(player.GetX() - x) < xlen/2)
			{
				if((player.GetZ() - z > - zlen/2 - player.GetRad()) && (player.GetZ() - z < 0))
				{
					player.SetZ(z - zlen/2 - player.GetRad());
				}
				else if(player.GetZ() - z < zlen/2 + player.GetRad())
				{
					if(player.GetZ() - z > 0)
						player.SetZ(z + zlen/2 + player.GetRad());
				}
			}
		}

	}
	
}

void Box::setExistence(bool a)
{
	exists = a;
}

bool Box::Exists()
{
	return exists;
}