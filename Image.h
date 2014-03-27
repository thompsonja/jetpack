#ifndef IMAGE_H
#define IMAGE_H

#include <string>

//image struct used to store the heightmap
class Image
{
public:
  Image(std::string filename);
  ~Image();
  double **grayValues;	//read in from the .pgm and later converted to actual heights
  int width;
  int height;
  int maxval;
};

#endif