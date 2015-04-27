#include "PerlinNoise1D.h"

#include <iostream>
#include <cmath>

using std::vector;
using std::cout;
using std::endl;

PerlinNoise1D::PerlinNoise1D()
{
  octarveNum = 8;
  persistence = 0.65;
  range = 1024;
  pixelVal.clear();
}
PerlinNoise1D::~PerlinNoise1D(){}
void PerlinNoise1D::setOcterve(int val)
{
  octarveNum = val;
}

void PerlinNoise1D::setPersistence(double val)
{
  persistence = val;
}

void PerlinNoise1D::setRange(int val)
{
  range = val;
}

double PerlinNoise1D::random(int x)
{
  x = (x<<13) ^ x;
  int xx = ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff);
  return  1.0 - ((double)xx / 1073741824.0);    
}

double PerlinNoise1D::sCurve(double t)
{
  return t*t*(3 - 2 * t);
}

double PerlinNoise1D::lerp(double a, double b, double t)
{
  return a*(1.0-t)+b*t;
}

double PerlinNoise1D::interpolate(double x)
{
  double d1 = x - (int)x, d2 = 1.0 - d1;
  double sx = sCurve(d1);
  double u, v;
  u = (random((int)x)+1)/2 * d1;
  v = (random((int)x+1)+1)/2 * d2;

  return lerp(u, v, sx);
}

double PerlinNoise1D::noiseAt(double t){
  double total = 0.0; 
  double freq = 1.0, amp = 1.0;
  for(int i = 0; i < octarveNum; i++){
    // total += interpolate(t/(double)range*freq) * amp;
    total += interpolate(t*freq) * amp;

    freq *= 2.0;
    amp *= persistence;
  }
  return total;
}

double PerlinNoise1D::get(int idx)
{
return pixelVal[idx];
}

void PerlinNoise1D::generate()
{
  pixelVal.clear();
  for(int i = 0; i < range; i++){
    pixelVal.push_back(noiseAt(i));
  }
}
