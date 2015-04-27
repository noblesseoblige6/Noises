#pragma once
#include "Vec2.h"
#include "PerlinNoise1D.h"
#include <vector>

class PerlinNoise2D : public PerlinNoise1D 
{
  private:
    int square;
    int width;
    int height;
    int permutations[256];
    std::vector<vec2> gradients;
  protected:
    double noise2D(int, int);
    double gradientAt(int, int);
    double interpolate(double, double);

  public:
    PerlinNoise2D();
    ~PerlinNoise2D();
    void setRange(int, int);
    void generate();
    void printData();
};
