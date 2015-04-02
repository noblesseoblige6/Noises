#include "PerlinNoise1D.h"

#include <iostream>
#include <cmath>

using std::vector;
using std::cout;
using std::endl;

PerlinNoise1D::PerlinNoise1D()
{
  interval = 25; 
  octarveNum = 8;
  persistence = 0.65;
  range = 1024;
  interpType = LINEAR;
  noises.clear();
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

void PerlinNoise1D::setInterval(int val)
{
  interval = val;
}

void PerlinNoise1D::setInterpType(INTERP_TYPE type)
{
  interpType = type;
}

double PerlinNoise1D::noise(int x)
{
  x = (x<<13) ^ x;
  int xx = ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff);
  return  1.0 - ((double)xx / 1073741824.0);    
}

double PerlinNoise1D::sCurve(double t)
{
  return t*t*(3 - 2 * t);
}

double PerlinNoise1D::linearInterpolate(double a, double b, double t)
{
  return a*(1.0-t)+b*t;
}

double PerlinNoise1D::cosineInterpolate(double a, double b, double t)
{
  double f = (1.0-cos(t*M_PI))*0.5;
  return a*(1.0-f)+b*t;
}

double PerlinNoise1D::interpolate(double x)
{
  double d1 = x - (int)x, d2 = 1.0 - d1;
  double sx = sCurve(d1);
  double u, v;
  u = noise((int)x) * d1;
  v = noise((int)x+1) * d2;

  if(interpType == LINEAR){return linearInterpolate(u, v, sx);}
  if(interpType == COSINE){return cosineInterpolate(u, v, sx);}
}

double PerlinNoise1D::gradientAt(double t){
  double total = 0.0; 
  double freq = 1.0, amp = 1.0;
  for(int i = 0; i < octarveNum; i++){
    // total += interpolate(t/(double)range*freq) * amp;
    double offset = noise((int)t);
    total += interpolate((t+offset)*freq) * amp;

    freq *= 2.0;
    amp *= persistence;
  }
  return total;
}

double PerlinNoise1D::get(int n)
{
  if(n < 0 || n >= noises.size()){return 0.0;}
  return noises[n];
}

void PerlinNoise1D::generate()
{
  noises.clear();
  for(int i = 0; i < range; i++){
    noises.push_back(gradientAt(i));
  }
}

void PerlinNoise1D::printData()
{
  for(int i = 0; i < noises.size(); i++){
    cout<<i<<" "<<noises[i]<<endl;
  }
}
