#include "EnergyBar.h"
#include <algorithm>

EnergyBar::EnergyBar(double maxEnergy) :
  maxEnergy(maxEnergy),
  energy(maxEnergy)
{
}

void EnergyBar::IncreaseEnergy(double amount)
{
  energy = std::min(energy + amount, maxEnergy);
}

void EnergyBar::DecreaseEnergy(double amount)
{
  energy = std::max(energy - amount, (double)0);
}

bool EnergyBar::IsEmpty()
{
  return energy <= 0;
}

bool EnergyBar::IsFull()
{
  return energy >= maxEnergy;
}