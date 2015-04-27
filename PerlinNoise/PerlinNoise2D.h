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
    std::vector<int> permutations;
    std::vector<vec2> gradients;
  protected:
    double noiseAt(int, int);
    double interpolate(double, double);

  public:
    PerlinNoise2D();
    ~PerlinNoise2D();
    void setRange(int, int);
    void generate();
};
