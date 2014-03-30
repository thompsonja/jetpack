#include "Renderer.h"
#include "EnergyBar.h"
#include <Windows.h>
#include <glut.h>

Renderer::Renderer(int width, int height) :
  width(width),
  height(height)
{
}

void Renderer::Render(double dt)
{
  DrawEnergyBar(jetpackBar, 5, 5, width / 5, height / 40);
  DrawEnergyBar(healthBar, 5, 5 + height / 40, width / 5, height / 40);
}

void Renderer::DrawEnergyBar(EnergyBar *bar, int x, int y, int width, int height)
{
  if(bar == NULL)
    return;

	glColor3f(1, 0, 0);
	glBegin(GL_LINE_LOOP);
	glVertex3f(x, y, 0.0f);
	glVertex3f(x + width, y, 0.0f);
	glVertex3f(x + width, y + height, 0.0f);
	glVertex3f(x, y + height, 0.0f);
	glEnd();

  double energyRatio = bar->GetEnergy() / bar->GetMaxEnergy();

	if(energyRatio > 0.66)
		glColor4f(0, .5, 0, 1.0);
	else if(energyRatio <= 0.66 && bar->GetEnergy() > 0.33)
		glColor4f(1, 1, 0, 1.0);
	else if(energyRatio < 0.33)
		glColor4f(1, 0, 0, 1.0);

	glBegin(GL_LINES);
	for(int i = 1; i < bar->GetEnergy()*width; i++)
	{
		glVertex3f(x + i, y, 0.0f);
		glVertex3f(x + i, y + height - 1, 0.0f);
	}
	glEnd();
}
