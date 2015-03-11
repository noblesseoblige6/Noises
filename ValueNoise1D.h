#pragma once
#include<vector>

enum INTERP_TYPE{
LINEAR = 0,
COSINE = 1
};

class ValueNoise1D
{
  private:
    int range;
    
  protected:
    int octarveNum;
    double persistence;
    int interpType;
    std::vector<double> noises;

    double smoothNoise(int);
    double interpolate(double);
    double linearInterpolate(double, double, double);
    double cosineInterpolate(double, double, double);
    double noise(int);
    double noiseAt(double);
  
  public:
    ValueNoise1D();
    ~ValueNoise1D();
    void setOcterve(int);
    void setPersistence(double);
    void setRange(int);
    void setInterpType(INTERP_TYPE);
    double get(int); 
    void generate();
    void printData();
};
