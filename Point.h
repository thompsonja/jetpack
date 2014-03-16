#ifndef POINT_H
#define POINT_H

class Point2D
{
public:
  Point2D() : x(0), y(0) {}
  Point2D(double x, double y) : x(x), y(y) {}
  double GetX() const{return x;}
  double GetY() const{return y;}
  void SetX(double val){x = val;}
  void SetY(double val){y = val;}
  void Set(double valX, double valY){SetX(valX); SetY(valY);}
private:
  double x;
  double y;
};

class Point3D : public Point2D
{
public:
  Point3D() : Point2D(), z(0) {}
  Point3D(double x, double y, double z) : Point2D(x, y) , z(z) {}
  double GetZ() const{return z;}
  void SetZ(double val){z = val;}
  void Set(double valX, double valY, double valZ){Point2D::Set(valX, valY); SetZ(valZ);}
private:
  double z;
};

#endif