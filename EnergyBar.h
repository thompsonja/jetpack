#ifndef ENERGYBAR_H
#define ENERGYBAR_H

//class for the jetpack energy meter
class EnergyBar
{
public:
	EnergyBar();					//constructor
	float getLength();
	float getMaxLength();
	void energyUp(float amount);
	void energyDown(float amount);
  void drawBar(int x, int y, int width, int height);
  bool IsEmpty();
  bool IsFull();
private:
  int maxLength; //length of bar
  int maxHeight; //height of bar
  float length;  //how full the bar is
  int xpos;      //x pos of bar
  int ypos;      //y pos of bar
  bool empty;
  bool full;
};

#endif