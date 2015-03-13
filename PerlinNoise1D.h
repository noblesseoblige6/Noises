#pragma once
#include<vector>

enum INTERP_TYPE{
LINEAR = 0,
COSINE = 1
};

class PerlinNoise1D
{
  private:
    int range;
    
  protected:
    double interval;
    int interpType;
    std::vector<double> noises;

    double sCurve(double);
    double interpolate(double);
    double linearInterpolate(double, double, double);
    double cosineInterpolate(double, double, double);
    double noise(int);
    double gradientAt(double, int);
  
  public:
    PerlinNoise1D();
    ~PerlinNoise1D();
    void setRange(int);
    void setInterpType(INTERP_TYPE);
    double get(int); 
    void generate();
    void printData();
};
