#ifndef BOX_H
#define BOX_H

class Box
{
private:
	bool exists;
	float xlen;
	float ylen;
	float zlen;
	float x;
	float y;
	float z;
	int side;
public:
	Box(float xlen, float ylen, float zlen, float x, float y, float z);
	void setExistence(bool a);
	bool Exists();
	void draw(float FPS);
};

#endif