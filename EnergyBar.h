#ifndef ENERGYBAR_H
#define ENERGYBAR_H

//class for the jetpack energy meter
class EnergyBar
{
private:
	int maxLength;					//length of bar
	int maxHeight;					//height of bar
	float length;					//how full the bar is
	int xpos;							//x pos of bar
	int ypos;							//y pos of bar
public:
	EnergyBar();					//constructor
	float getLength();
	float getMaxLength();
	float getMaxHeight();
	void setPos(int xpos, int ypos);
	void energyUp(float FPS, float amount);		//increases energy bar
	void energyDown(float FPS, float amount);	//decreases energy bar
	void drawBar();
};

#endif