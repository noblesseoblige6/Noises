#pragma once
#include "PerlinNoise1D.h"

class PerlinNoise2D : public PerlinNoise1D 
{
  private:
    int square;
    int width;
    int height;

  protected:
    double noise2D(int, int);
    double gradientAt(int, int);
 
  public:
    PerlinNoise2D();
    ~PerlinNoise2D();
    void setRange(int, int);
    void generate();
    void printData();
};
