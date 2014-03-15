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

	if(fabs(player.getY() - (y + ylen/2)) < player.getHeight())
	{
		if(fabs(player.getZ() - z) < zlen/2 && fabs(player.getX() - x) < xlen/2)
		{
			if(yVel <= 0)
			{
				jumped = false;
				player.onBox = true;
				player.setY(y + ylen/2 + player.getHeight());
				yVel = 0;
				if(side == 0)
					player.setX(player.getX() + 30/FPS);
				if(side == 1)
					player.setZ(player.getZ() + 30/FPS);
				if(side == 2)
					player.setX(player.getX() - 30/FPS);
				if(side == 3)
					player.setZ(player.getZ() - 30/FPS);

			}
		}
		else if(player.onBox)
		{
			jumped = true;
			player.setY(y + ylen/2 + player.getHeight());
			player.onBox = false;
			jumpHeight = y + ylen/2 + player.getHeight();
		}
		else
		{
			if(fabs(player.getZ() - z) < zlen/2)
			{
				if((player.getX() - x > - xlen/2 - player.getRad()) && (player.getX() - x < 0))
				{
					player.setX(x - xlen/2 - player.getRad());
				}
				else if(player.getX() - x < xlen/2 + player.getRad())
				{
					if(player.getX() - x > 0)
						player.setX(x + xlen/2 + player.getRad());
				}
			}
			if(fabs(player.getX() - x) < xlen/2)
			{
				if((player.getZ() - z > - zlen/2 - player.getRad()) && (player.getZ() - z < 0))
				{
					player.setZ(z - zlen/2 - player.getRad());
				}
				else if(player.getZ() - z < zlen/2 + player.getRad())
				{
					if(player.getZ() - z > 0)
						player.setZ(z + zlen/2 + player.getRad());
				}
			}
		}

	}
	
}

void Box::setExistence(bool a)
{
	exists = a;
}

int Box::isExistent()
{
	return exists;
}