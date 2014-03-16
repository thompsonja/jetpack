#ifndef MODEL_H
#define MODEL_H

#include "Point.h"

class Model
{
public:
  enum AnimationType
  {
    MODEL_STATIC,
    MODEL_ANIMATED
  };
  Model(AnimationType type, char *baseFilename);
  ~Model();
  bool IsValid(){return valid;}
  const Point3D& GetPosition(){return currentPosition;}
  void SetStartPosition(Point2D startingPosition){currentPosition.SetX(startingPosition.GetX()); currentPosition.SetZ(startingPosition.GetY());}
  int GetCurrentFrame() {return currentFrame;}
  void IncrementFrame(double animationSpeed);
  int GetLinesAtFrame(int frameNum){return frameNum > numTotalFrames ? -1 : modelLines[frameNum];}
  const Point3D &GetVertex(int frameNum, int lineNum){return vertices[frameNum][lineNum];}
  const Point2D &GetTexture(int frameNum, int lineNum){return textureCoords[frameNum][lineNum];}
  const Point3D &GetNormal(int frameNum, int lineNum){return normals[frameNum][lineNum];}
  int GetNumFrames(){return numTotalFrames;}
private:
  Point3D **vertices;
  Point2D **textureCoords;
  Point3D **normals;
  AnimationType type;
  char *baseFilename;
  Point3D currentPosition;
  double minHeight;
  int numTotalFrames;
  int currentFrame;
  int *modelLines;
  bool valid;

  bool LoadFrame(int index, char *fileName);
};

#endif