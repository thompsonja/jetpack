#ifndef LIGHT_H
#define LIGHT_H

//light type used for setting material properties
class Light
{
public:
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float emission[4];
  float shininess;

  void SetGlLight();
};

#endif