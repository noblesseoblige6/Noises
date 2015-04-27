#pragma once
#include<vector>

class ValueNoise1D
{
  private:
    int range;
    
  protected:
    int octarveNum;
    double persistence;
    std::vector<double> pixelVal;

    double smoothNoise(int);
    double interpolate(double);
    double lerp(double, double, double);
    double random(int);
    double noiseAt(double);
  
  public:
    ValueNoise1D();
    ~ValueNoise1D();
    void setOcterve(int);
    void setPersistence(double);
    void setRange(int);
    double get(int); 
    void generate();
};
