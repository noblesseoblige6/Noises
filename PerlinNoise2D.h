#pragma once
#include "PerlinNoise1D.h"
#include <cmath>

class vec2{
  public:
    double x;
    double y;
    double length;
    vec2():x(0.0),y(0.0),length(0.0){};
    ~vec2(){};
    vec2(double x, double y):x(x),y(y),length(sqrt(x*x+y*y)){};
    void norm(){length = sqrt(x*x + y*y);};
    vec2 normalized(){
      norm();
      return vec2(x/length, y/length);
    };
};

class PerlinNoise2D : public PerlinNoise1D 
{
  private:
    int square;
    int width;
    int height;

  protected:
    double noise2D(int, int);
    double gradientAt(double, double);
    double interpolate(double, double);

  public:
    PerlinNoise2D();
    ~PerlinNoise2D();
    void setRange(int, int);
    void generate();
    void printData();
};
