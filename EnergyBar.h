#ifndef ENERGYBAR_H
#define ENERGYBAR_H

class EnergyBar
{
public:
	EnergyBar(double maxEnergy = 1.0);
  double GetEnergy(){return energy;}
  double GetMaxEnergy(){return maxEnergy;}
	void IncreaseEnergy(double amount);
	void DecreaseEnergy(double amount);
  bool IsEmpty();
  bool IsFull();
private:
  double maxEnergy; // Maximum amount of energy that can be held
  double energy;  // current amount of energy in bar
};

#endif