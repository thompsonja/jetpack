#include "EnergyBar.h"
#include <Windows.h>
#include <glut.h>
#include "User.h"
#include <math.h>

extern int width;
extern int height;
extern float yVel;
extern float jumpHeight;
extern bool jumped;
extern User player;

EnergyBar::EnergyBar()
{
	maxLength = width/5;
	length = maxLength;
	maxHeight = height/40;
}

float EnergyBar::getLength()
{
	return length;
}

float EnergyBar::getMaxLength()
{
	return maxLength;
}

float EnergyBar::getMaxHeight()
{
	return maxHeight;
}

void EnergyBar::setPos(int xpos, int ypos)
{
	this->xpos = xpos;
	this->ypos = ypos;
}

void EnergyBar::energyUp(float FPS, float amount)
{
	if(amount == 0)
		length += 300/FPS * .25;
	else
		length += amount;

	if(length > maxLength)
		length = maxLength;

}

void EnergyBar::energyDown(float FPS, float amount)
{
	if(amount == 0)
	{
		if(length > 0)
		{
			length -= 300/FPS * .25;
			yVel += 300/FPS * .48;
			if(!jumped)
			{
				jumped = true;
				jumpHeight = player.GetY();
				yVel = 10;
			}
		}
	}
	else
		length -= fabs(amount*maxLength);

	if(length < 0)
		length = 0;
}

void EnergyBar::drawBar()
{
	glColor3f(1, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(xpos, ypos, 0.0f);
	glVertex3f(xpos + maxLength, ypos, 0.0f);
	glVertex3f(xpos + maxLength, ypos + maxHeight, 0.0f);
	glVertex3f(xpos, ypos + maxHeight, 0.0f);
	glEnd();

	if(length/maxLength > 2.0/3)
		glColor4f(0, .5, 0, 1.0);
	else if(length/maxLength <= 2.0/3 && length/maxLength > 1.0/3)
		glColor4f(1, 1, 0, 1.0);
	else if(length/maxLength < 1.0/3)
		glColor4f(1, 0, 0, 1.0);
	glBegin(GL_LINES);
	for(int i = 1; i < length; i++)
	{
		glVertex3f(xpos + i, ypos, 0.0f);
		glVertex3f(xpos + i, ypos + maxHeight - 1, 0.0f);
	}
	glEnd();
}