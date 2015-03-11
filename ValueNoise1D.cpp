#include "ValueNoise1D.h"

#include <iostream>
#include <cmath>

using std::vector;
using std::cout;
using std::endl;

ValueNoise1D::ValueNoise1D()
{
  octarveNum = 8;
  persistence = 0.5;
  range = 1024;
  interpType = LINEAR;
  noises.clear();
}
ValueNoise1D::~ValueNoise1D(){}
void ValueNoise1D::setOcterve(int val)
{
  octarveNum = val;
}

void ValueNoise1D::setPersistence(double val)
{
  persistence = val;
}

void ValueNoise1D::setRange(int val)
{
  range = val;
}

void ValueNoise1D::setInterpType(INTERP_TYPE type)
{
  interpType = type;
}

double ValueNoise1D::noise(int x)
{
  x = (x<<13) ^ x;
  int xx = ((x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff);
  return  1.0 - ((double)xx / 1073741824.0);    
}

double ValueNoise1D::smoothNoise(int x)
{
  return noise(x)/2 +noise(x-1)/4 + noise(x+1)/4;
}

double ValueNoise1D::linearInterpolate(double a, double b, double t)
{
  return a*(1.0-t)+b*t;
}

double ValueNoise1D::cosineInterpolate(double a, double b, double t)
{
  double f = (1.0-cos(t*M_PI))*0.5;
  return a*(1.0-f)+b*t;
}
double ValueNoise1D::interpolate(double x)
{
  int xInt = floor(x);
  double xFract = x - xInt;
  double v1, v2;
  v1 = smoothNoise(xInt);
  v2 = smoothNoise(xInt+1);
  if(interpType == LINEAR){return linearInterpolate(v1, v2, xFract);}
  if(interpType == COSINE){return cosineInterpolate(v1, v2, xFract);}
}

double ValueNoise1D::noiseAt(double t)
{
  double total = 0.0; 
  double freq, amp;
  for(int i = 0; i < octarveNum; i++){
    freq = pow(2, i);
    amp = pow(persistence, i);
    total += interpolate(t*freq) *  amp;
  }
  return total;
}
double ValueNoise1D::get(int n)
{
  if(n < 0 || n >= noises.size()){return 0.0;}
  return noises[n];
}
void ValueNoise1D::generate()
{
  noises.clear();
  for(int i = 0; i < range; i++){
    noises.push_back(noiseAt(i/(double)range));
  }
}

void ValueNoise1D::printData()
{
  for(int i = 0; i < noises.size(); i++){
    cout<<i<<" "<<noises[i]<<endl;
  }
}
