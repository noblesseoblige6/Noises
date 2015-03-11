#pragma once
#include "ValueNoise1D.h"

class ValueNoise2D : public ValueNoise1D
{
  private:
    int width;
    int height;

  protected:
    double smoothNoise(int, int);
    double interpolate(double, double);
    double noise(int, int);
    double noiseAt(double, double);
  
  public:
    ValueNoise2D();
    ~ValueNoise2D();
    void setRange(int, int);
    void generate();
    void printData();
};
