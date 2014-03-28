#include "EnergyBar.h"
#include <Windows.h>
#include <glut.h>
#include <math.h>

EnergyBar::EnergyBar()
{
	maxLength = 1.0;
	length = 1.0;
}

float EnergyBar::getLength()
{
	return length;
}

float EnergyBar::getMaxLength()
{
	return maxLength;
}

void EnergyBar::setPos(int xpos, int ypos)
{
	this->xpos = xpos;
	this->ypos = ypos;
}

void EnergyBar::energyUp(float amount)
{
  length += amount;
	if(length > maxLength)
    length = maxLength;
}

void EnergyBar::energyDown(float amount)
{
  length -= amount;
	if(length < 0)
		length = 0;
}

void EnergyBar::drawBar(int width, int height)
{
	glColor3f(1, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(xpos, ypos, 0.0f);
	glVertex3f(xpos + width, ypos, 0.0f);
	glVertex3f(xpos + width, ypos + height, 0.0f);
	glVertex3f(xpos, ypos + height, 0.0f);
	glEnd();

	if(length > 0.66)
		glColor4f(0, .5, 0, 1.0);
	else if(length <= 0.66 && length > 0.33)
		glColor4f(1, 1, 0, 1.0);
	else if(length < 0.33)
		glColor4f(1, 0, 0, 1.0);
	glBegin(GL_LINES);
	for(int i = 1; i < length*width; i++)
	{
		glVertex3f(xpos + i, ypos, 0.0f);
		glVertex3f(xpos + i, ypos + height - 1, 0.0f);
	}
	glEnd();
}