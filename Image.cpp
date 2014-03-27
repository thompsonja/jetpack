#include "Image.h"

Image::Image(std::string filename)
{
  FILE *inf = fopen(filename.c_str(), "r+");
  //some error handling
  if((inf = fopen(filename.c_str(), "r+")) == NULL)
  {
    printf("Unable to open pgm file: %s!\n", filename);
    exit(1);
  }
  if(fscanf(inf, " P2 "))
  {
    printf("not a valid pgm: %s!\n",filename);
    exit(1);
  }

  char temp[256];
  fgets(temp, 128 * sizeof(char), inf);
  while(temp[0] == '#')						//skips commented lines
    fgets(temp, 128 * sizeof(char), inf);

  sscanf(temp, "%d %d", &width, &height);		//scans for width, height
  fscanf(inf, "%d", &maxval);					//and maxval

  grayValues = new double*[height];
  for(int i = 0; i < height; i++)
  {
    grayValues[i] = new double[width];
  }

  //fills the 2D array with grayscale vlues from .pgm file
  for(int i=height - 1; i >= 0; i--)
  {
    for(int j=0; j<width; j++)
    {
      int color;
      fscanf(inf, "%d ", &color);
      grayValues[i][j] = (double) color;
    }
  }
  fclose(inf);
}

Image::~Image()
{
  for(int i = 0; i < height; i++)
  {
    delete grayValues[i];
  }
  delete grayValues;
}