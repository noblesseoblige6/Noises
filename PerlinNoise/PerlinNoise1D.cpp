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
  int xInt = (int)x;
  double d1 = x - xInt, d2 = d1 -1.0;
  double sx = sCurve(d1);
  double u, v;

  int grad1 = permutations[xInt & 255] & 7;
  int grad2 = permutations[(xInt+1) & 255] & 7;
  u = gradients[grad1] * d1;
  v = gradients[grad2] * d2;

  return lerp(u, v, sx);
}

double PerlinNoise1D::noiseAt(double t){
  double total = 0.0; 
  double freq = 1.0/range, amp = 1.0;
  for(int i = 0; i < octarveNum; i++){
    total += interpolate(t*freq) * amp;
    freq *= 2.0;
    amp *= persistence;
  }
  return total;
}

double PerlinNoise1D::get(int idx)
{
  if(idx < 0 || idx >= pixelVal.size()){return 0.0;}
  return pixelVal[idx];
}

void PerlinNoise1D::generate()
{
  //@comment set random vector and normailize it
  for (int i=0; i<8; i++){
    gradients.push_back(random(i));
  }
  //set up the random numbers table
  for(int i=0; i<256; i++){
    permutations.push_back((int)(((random(i)+1.0)/2.0)*255));
  }
  pixelVal.clear();
  for(int i = 0; i < range; i++){
    pixelVal.push_back(noiseAt(i));
  }
}
