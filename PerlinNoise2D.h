#pragma once

class PerlinNoise2D 
{
  private:
    int width;
    int height;

  protected:
    double sCurve(int, int);
    double lerp(double, double);
    double noise(int, int);
    double gradientAt(double, double);
  
  public:
    PerlinNoise2D();
    ~PerlinNoise2D();
    void setRange(int, int);
    void generate();
};
