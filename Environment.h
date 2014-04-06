#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <Windows.h>
#include <glut.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

class SphereRing;
class Billboard;
class Image;
class Model;

class Environment
{
public:
  Environment();
  void Parse(std::string filename);

  typedef void (Environment::*LineParser)(const std::vector<std::string> &subStrings);

  void SplitLine(std::string str, std::vector<std::string> &out, char ch = ' ');
  void ParseHeightMapDefinition(const std::vector<std::string> &subStrings);
  void ParseLengths(const std::vector<std::string> &subStrings);
  void ParseTexture(const std::vector<std::string> &subStrings);
  void ParseModel(const std::vector<std::string> &subStrings);
  void ParseBillboard(const std::vector<std::string> &subStrings);
  void ParseSkydome(const std::vector<std::string> &subStrings);
  void ParseWater(const std::vector<std::string> &subStrings);
  void ParseSun(const std::vector<std::string> &subStrings);
  void ParseRing(const std::vector<std::string> &subStrings);

  bool LoadTextures(const std::string &filename, GLuint *texture, int &width, int &height);
  
  //variables storing which things exist
  bool textureExists; //whether a texture will cover the terrain
  bool waterExists;   //whether or not there is water
  bool skyExists;
  bool sun1Exists;
  bool sun2Exists;

  int XLEN; //xlength of quad mesh
  int ZLEN; //zlength of quad mesh (I define y as up)
  float minHeight, maxHeight;  //min and max y-values defined by environment file
  float waterHeight, waterOsc; //water height and how much it oscillates
  int skydomeAngle; //how much the skydome rotates

  //skydome variables
  int angle;       //updates rotation angle based on skydomeAngle
  float skyRadius; //radius of the sky
  int skySpeed; //affects sky speed.  Even though the project description

  float scaleFactor; //how much to scale objects/billboards

  std::vector<std::shared_ptr<SphereRing>> rings;
  std::vector<std::shared_ptr<Billboard>> billboards;
  std::vector<std::shared_ptr<Model>> models;

  std::map<std::string, LineParser> parsers;

  std::shared_ptr<Image> map;

  bool readHeight;
  bool readTexture;
  bool readLengths;
  bool readWater;
  bool readSky;
  bool readSun;

  //Textures
  GLuint terrainTexture;
  GLuint waterTexture;
  GLuint skyTexture;
  GLuint sunTexture1;
  GLuint sunTexture2;
  std::map<std::shared_ptr<Billboard>, GLuint> billboardTextures;
  GLuint *currentSun;
};

#endif