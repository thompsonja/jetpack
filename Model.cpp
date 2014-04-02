#include "Model.h"
#include <stdio.h>

Model::Model(AnimationType type, const char *baseFilename) :
  type(type),
  baseFilename(baseFilename),
  numTotalFrames(1),
  currentFrame(0),
  minHeight(0)
{
  if(type == MODEL_STATIC)
  {
    vertices = new Point3D*[1];
    textureCoords = new Point2D*[1];
    normals = new Point3D*[1];
    modelLines = new int[1];

    valid = LoadFrame(0, baseFilename);
    if(!valid)
      return;
  }
  else
  {
    char rootName[32];

    //gets the base name (ie anim.mod becomes anim)
    int i = 0;
    for (i = 0; baseFilename[i] != '.'; i++)
      rootName[i] = baseFilename[i];
    rootName[i] = '\0';

    char buffer[256];
    int fileCounter = 1;
    //calculates how many files are in a row
    for(sprintf(buffer, "%s%.2d.mod\0", rootName, fileCounter); 
      (fopen(buffer, "r") != NULL); )
    {
      fileCounter++;
      sprintf(buffer, "%s%.2d.mod\0", rootName, fileCounter);
    }
    //fileCounter will end up being one more than the number of files, so reduce it by one
    numTotalFrames = fileCounter-1;

    vertices = new Point3D*[numTotalFrames];
    textureCoords = new Point2D*[numTotalFrames];
    normals = new Point3D*[numTotalFrames];
    modelLines = new int[numTotalFrames];

    //this is the same as for the static case, but this does it for each file
    for(int i = 0; i < numTotalFrames; i++)
    {
      sprintf(buffer, "%s%.2d%s\0", rootName, i+1, ".mod");
      valid = LoadFrame(i, buffer);
      if(!valid)
        return;
    }
  }

  minHeight = vertices[0][0].GetY();
  for(int i = 0; i < modelLines[0]; i++)
  {
    if (vertices[0][i].GetY() < minHeight)
      minHeight = vertices[0][i].GetY();
  }

  currentPosition.SetY(minHeight);
  printf("DONE\n");
}

Model::~Model()
{
  for(int i = 0; i < numTotalFrames; i++)
  {
    delete vertices[i];
    delete textureCoords[i];
    delete normals[i];
  }
  delete vertices;
  delete textureCoords;
  delete normals;
  delete modelLines;
}

bool Model::LoadFrame(int index, const char *filename)
{
  FILE *f = fopen(filename, "r");
  if(f == NULL)
  {
    printf("FAILED\n\t\tCould not open object file: %s\n", filename);
    return false;
  }

  //first it reads how many lines of vertices, etc., there are in the .mod file
  int numLines = 0;
  char text[256];
  while(fgets(text, 128*sizeof(char), f) != NULL)
  {
    if (text[0] == 'N')
      numLines++;
  }
  fclose(f);

  if(numLines == 0)
  {
    printf("FAILED\n\t\tNo valid lines in object file: %s\n", baseFilename);
    return false;
  }

  modelLines[index] = numLines;
  vertices[index] = new Point3D[numLines];
  textureCoords[index] = new Point2D[numLines];
  normals[index] = new Point3D[numLines];

  f = fopen(filename, "r");
  for(int i = 0; fgets(text, 128*sizeof(char), f) != NULL; i++)
  {
    //scans in the values for a line.  Note: I have it going as N, N[2],
    //and N[1] because I took the y-direction as up and down, so putting
    //it in this order made the object right-side-up.
    char dummy;
    float normX, normY, normZ, texX, texY, vertX, vertY, vertZ;
    sscanf(text, "%c%f%f%f%c%c%f%f%c%c%f%f%f", &dummy, &normX, &normZ, &normY,
      &dummy, &dummy, &texX, &texY, &dummy, &dummy, &vertX, &vertZ, &vertY);
    vertices[index][i].Set(vertX, vertY, vertZ);
    textureCoords[index][i].Set(texX, texY);
    normals[index][i].Set(normX, normY, normZ);
  }
  fclose(f);

  return true;
}

void Model::IncrementFrame(double animationSpeed)
{
  //if the object is static, ob->Counter is 0, so it will always draw the same object, but if it is
  //animated, I use integer division to modify when the next object in the animation is drawn.  By
  //increasing animationSpeed, it takes more renders for ob->Counter/animationSpeed to actually increase.
  //Also, the CallList is sequential, as when it's an animated object, I call for a group of sequential lists,
  //based on how many files are loaded in the animation.
  currentFrame++;
  if(currentFrame > animationSpeed*numTotalFrames - 1)
    currentFrame = 0;
}