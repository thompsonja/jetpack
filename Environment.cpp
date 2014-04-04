#include <algorithm>
#include "Environment.h"
#include "SphereRing.h"
#include "Point.h"
#include "Billboard.h"
#include "Image.h"
#include "Model.h"
#include "FreeImage.h"

Environment::Environment() :
  textureExists(false),
  waterExists(false),
  skyExists(false),
  sun1Exists(false),
  sun2Exists(false),
  XLEN(1),
  ZLEN(1),
  skySpeed(50),
  scaleFactor(1),
  map(NULL),
  readHeight(false),
  readTexture(false),
  readLengths(false),
  readWater(false),
  readSky(false),
  readSun(false),
  currentSun(NULL)
{
  parsers["HEIGHT"] = &Environment::ParseHeightMapDefinition;
  parsers["LENGTHS"] = &Environment::ParseLengths;
  parsers["TEXTURE"] = &Environment::ParseTexture;
  parsers["OBJ"] = &Environment::ParseModel;
  parsers["BILLBOARD"] = &Environment::ParseBillboard;
  parsers["SKYDOME"] = &Environment::ParseSkydome;
  parsers["WATER"] = &Environment::ParseWater;
  parsers["SUN"] = &Environment::ParseSun;
  parsers["RING"] = &Environment::ParseRing;
}

Environment::~Environment()
{
  for(unsigned int i = 0; i < models.size(); i++)
  {
    delete models[i];
  }
  for(unsigned int i = 0; i < rings.size(); i++)
  {
    delete rings[i];
  }
  for(unsigned int i = 0; i < billboards.size(); i++)
  {
    delete billboards[i];
  }
  models.clear();
  rings.clear();
  billboards.clear();
}

void Environment::SplitLine(std::string str, std::vector<std::string> &out, char ch)
{
  unsigned int pos = str.find(ch);
  unsigned int initialPos = 0;
  out.clear();

  // Decompose statement
  while(pos != std::string::npos)
  {
    out.push_back(str.substr(initialPos, pos - initialPos));
    initialPos = pos + 1;

    while(initialPos != std::string::npos && str[initialPos] == ch)
    {
      initialPos++;
    }
    pos = str.find(ch, initialPos);
  }

  // Add the last one
  out.push_back(str.substr(initialPos, std::min(pos, (unsigned int)str.size()) - initialPos + 1));
}

void Environment::Parse(std::string filename)
{
  printf("Loading environment file: %s\n", filename.c_str());
  FILE *f = fopen(filename.c_str(), "r");

  //grabs a new line from the env. file
  char temp[256];
  while(fgets(temp, 128*sizeof(char), f) != NULL)
  {
    std::vector<std::string> subStrings;
    std::string str(temp);
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    SplitLine(str, subStrings);
    if(subStrings.size() <= 1)
    {
      continue;
    }

    std::string keyword = subStrings[0];

    if(parsers.find(keyword) != parsers.end())
    {
      subStrings.erase(subStrings.begin());
      (this->*parsers[keyword])(subStrings);
    }
  }

  printf("Loading complete, executing program\n");
}

void Environment::ParseHeightMapDefinition(const std::vector<std::string> &subStrings)
{
  printf("	Loading height map  . . . . . ");
  if(readHeight)
  {
    printf("FAILED\n\t\tAlready read height map!\n");
    return;
  }

  if(subStrings.size() != 3)
  {
    printf("Wrong number of parameters!\n");
    return;
  }

  const std::string &filename = subStrings[0];
  map = new Image(filename);
  minHeight = atof(subStrings[1].c_str());
  maxHeight = atof(subStrings[2].c_str());
  
  readHeight = true;
  printf("DONE\n");    
}

void Environment::ParseLengths(const std::vector<std::string> &subStrings)
{
  printf("	Loading lengths . . . . . . . ");
  if(readLengths)
  {
    printf("FAILED\n\t\tAlready read lengths!\n");
    return;
  }

  if(subStrings.size() != 2)
  {
    printf("Wrong number of parameters!\n");
    return;
  }

  XLEN = atof(subStrings[0].c_str());
  ZLEN = atof(subStrings[1].c_str());

  readLengths = true;

  printf("DONE\n");
}

void Environment::ParseTexture(const std::vector<std::string> &subStrings)
{
  printf("	Loading land texture  . . . . ");
  if(readTexture)
  {
    printf("FAILED\n\t\tAlready read land texture!\n");
    return;
  }

  if(subStrings.size() != 1)
  {
    printf("Wrong number of parameters!\n");
    return;
  }

  const std::string &textureFilename = subStrings[0];

  //Loads the texture and sets the textureExists parameter
  int width, height;
  textureExists = LoadTextures(textureFilename, &terrainTexture, width, height);
  if(textureExists)
    printf("DONE\n");
  else
    printf("FAILED\n\t\tCould not open texture: %s\n", textureFilename.c_str());

  readTexture = true;
}

void Environment::ParseModel(const std::vector<std::string> &subStrings)
{
  if(subStrings.size() != 4)
  {
    printf("Wrong number of parameters!\n");
    return;
  }

  const std::string &modelType = subStrings[0];
  const std::string &modelName = subStrings[1];

  int objz = atoi(subStrings[2].c_str());
  int objx = atoi(subStrings[3].c_str());

  //inserts the object into the linked list
  if(modelType.compare("Static") != 0 && modelType.compare("Animated") != 0)
  {
    printf("Invalid model type: %s\n", modelType.c_str());
    return;
  }

  printf("	Loading %s object . . . . ", modelType.c_str());
  if(objx > map->height || objz > map->width)
  {
    printf("FAILED\n\t\tObject outside of terrain\n");
    return;
  }

  Model::AnimationType animationType = modelType.compare("Static") == 0 ? Model::MODEL_STATIC : Model::MODEL_ANIMATED;

  models.push_back(new Model(animationType, modelName.c_str()));
  if(models.back()->IsValid())
  {
    models.back()->SetStartPosition(Point2D(objx, objz));
  }
}

void Environment::ParseBillboard(const std::vector<std::string> &subStrings)
{
  printf("	Loading Billboard . . . ");

  if(subStrings.size() != 4)
  {
    printf("Wrong number of parameters!\n");
    return;
  }

  const std::string &billboardName = subStrings[0];
  const std::string &billboardFilename = subStrings[1];
  int billboardZ = atoi(subStrings[2].c_str());
  int billboardX = atoi(subStrings[2].c_str());

  Billboard *newBillboard = new Billboard(Point3D(billboardX, map->grayValues[billboardX-1][billboardZ-1], billboardZ));
  
  int width, height;
  if(LoadTextures(billboardFilename, &billboardTextures[newBillboard], width, height))		//if it loads the texture correctly
  {
    newBillboard->width = width;
    newBillboard->height = height;
    billboards.push_back(newBillboard);
  }
  else
  {
    printf("FAILED\n\t\tCould not open billboard: %s\n", billboardFilename.c_str());
  }

  printf("DONE\n");
}

void Environment::ParseSkydome(const std::vector<std::string> &subStrings)
{
  printf("	Loading skydome texture . . . ");
  if(readSky)
  {
    printf("FAILED\n\t\tAlready read skydome texture!\n");
    return;
  }

  if(subStrings.size() != 2)
  {
    printf("Wrong number of parameters!\n");
    return;
  }

  const std::string &skydomeTextureFilename = subStrings[0];
  skydomeAngle = atoi(subStrings[1].c_str());
  angle = skydomeAngle; //set angle to skydomeAngle, since angle is what's actually going to be increased

  int width, height;
  skyExists = LoadTextures(skydomeTextureFilename, &skyTexture, width, height);
  if(skyExists)
  {
    printf("DONE\n");
    readSky = true;
  }
  else
  {
    printf("FAILED\n\t\tCould not open texture: %s\n", skydomeTextureFilename.c_str());
  }
}

void Environment::ParseWater(const std::vector<std::string> &subStrings)
{
  printf("	Loading water texture . . . . ");
  if(readWater)
  {
    printf("FAILED\n\t\tAlready read water values!\n");
    return;
  }

  if(subStrings.size() != 3)
  {
    printf("Wrong number of parameters!\n");
    return;
  }

  const std::string &skydomeTextureFilename = subStrings[0];
  waterHeight = atof(subStrings[1].c_str());
  waterOsc = atof(subStrings[2].c_str());

  int width, height;
  if(waterExists = LoadTextures(skydomeTextureFilename, &waterTexture, width, height))
  {
    printf("DONE\n");
    readWater = true;
  }
  else
  {
    printf("FAILED\n\t\tCould not open texture: %s\n", skydomeTextureFilename.c_str());
  }
}

void Environment::ParseSun(const std::vector<std::string> &subStrings)
{
  if(readSun)
  {
    printf("	Loading sun textures . . . . FAILED\n\t\tAlready read sun textures!\n");
    return;
  }

  if(subStrings.size() != 2)
  {
    printf("Wrong number of parameters!\n");
    return;
  }

  const std::string &sunTexture1Filename = subStrings[0];
  const std::string &sunTexture2Filename = subStrings[1];
    
  printf("	Loading sun texture 1 . . . . ");

  int width, height;
  sun1Exists = LoadTextures(sunTexture1Filename, &sunTexture1, width, height); //load texture
  if(sun1Exists)
  {
    printf("DONE\n");
    currentSun = &sunTexture1;
  }
  else
    printf("FAILED\n\t\tCould not open sun: %s", sunTexture1Filename.c_str());

  printf("	Loading sun texture 2 . . . . ");
  sun2Exists = LoadTextures(sunTexture2Filename, &sunTexture2, width, height); //load texture
  if(sun2Exists)
  {
    printf("DONE\n");
    currentSun = &sunTexture2;
    readSun = true;
  }
  else
  {
    printf("FAILED\n\t\tCould not open sun: %s", sunTexture2Filename.c_str());
  }
}

void Environment::ParseRing(const std::vector<std::string> &subStrings)
{
  printf("	Loading ring  . . . . . . . . ");

  if(subStrings.size() != 7)
  {
    printf("Wrong number of parameters!\n");
    return;
  }

  int x = atoi(subStrings[0].c_str());
  int y = atoi(subStrings[1].c_str());
  int z = atoi(subStrings[2].c_str());
  double ang = atof(subStrings[3].c_str());
  int num = atoi(subStrings[4].c_str());
  double sphRad = atof(subStrings[5].c_str());
  double rRad = atof(subStrings[6].c_str());

  rings.push_back(new SphereRing(Point3D(x, y, z), ang, num, sphRad, rRad));

  printf("DONE\n");
}

//Load bmps and convert to textures
bool Environment::LoadTextures(const std::string &filename, GLuint *texture, int &width, int &height)
{
  FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename.c_str());
  if(format == FIF_UNKNOWN)
  {
    printf("Unknown image format for file %s\n", filename.c_str());
    return false;
  }

  FIBITMAP *tex = FreeImage_Load(format, filename.c_str());
  if(tex == NULL)
  {
    printf("FreeImage failed to load file %s\n", filename.c_str());
    return false;
  }

  FIBITMAP *tex32 = FreeImage_ConvertTo32Bits(tex);
  if(tex32 == NULL)
  {
    printf("FreeImage failed to convert file %s to 32 bit format\n", filename.c_str());
    return false;
  }

  width = FreeImage_GetWidth(tex32);
  height = FreeImage_GetHeight(tex32);

  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_2D, *texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(tex32));
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
  FreeImage_Unload(tex32);
  FreeImage_Unload(tex);
  
  return true;										//Returns whether or not it succeeds
}