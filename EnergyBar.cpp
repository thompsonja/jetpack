#include "EnergyBar.h"
#include <Windows.h>
#include <glut.h>
#include <math.h>

EnergyBar::EnergyBar() :
  maxLength(1.0),
  length(1.0),
  empty(false),
  full(true)
{
}

float EnergyBar::getLength()
{
	return length;
}

float EnergyBar::getMaxLength()
{
	return maxLength;
}

void EnergyBar::energyUp(float amount)
{
  length += amount;
	if(length > maxLength)
  {
    length = maxLength;
    full = true;
  }
  if(amount > 0)
  {
    empty = false;
  }
}

void EnergyBar::energyDown(float amount)
{
  if(amount > 0)
  {
    full = false;
  }
  length -= amount;
	if(length < 0)
  {
		length = 0;
    empty = true;
  }
}

void EnergyBar::drawBar(int x, int y, int width, int height)
{
	glColor3f(1, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(x, y, 0.0f);
	glVertex3f(x + width, y, 0.0f);
	glVertex3f(x + width, y + height, 0.0f);
	glVertex3f(x, y + height, 0.0f);
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
		glVertex3f(x + i, y, 0.0f);
		glVertex3f(x + i, y + height - 1, 0.0f);
	}
	glEnd();
}

bool EnergyBar::IsEmpty()
{
  return empty;
}

bool EnergyBar::IsFull()
{
  return full;
}