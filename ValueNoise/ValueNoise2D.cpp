#include "ValueNoise2D.h"
#include <iostream>
#include <cmath>

using std::vector;
using std::cout;
using std::endl;

ValueNoise2D::ValueNoise2D()
{
  octarveNum = 8;
  persistence = 0.65;
  width = height = 256;
  pixelVal.clear();
}

ValueNoise2D::~ValueNoise2D(){}

double ValueNoise2D::random(int x, int y)
{
  int n = x + y * 57;
  n = (n<<13)^n;
  int nn = ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff);
  return  1.0 - ((double)nn / 1073741824.0);   
}


double ValueNoise2D::smoothNoise(int x, int y)
{
  double side, corner, center;
  center = random(x, y)/4;
  side = (random(x+1, y)+random(x-1, y)+random(x, y+1)+random(x, y-1))/8;
  corner = (random(x+1, y+1)+random(x-1, y+1)+random(x+1, y-1)+random(x-1, y-1))/16;
  return side+corner+center;
}

double ValueNoise2D::interpolate(double x, double y)
{
  int xInt = floor(x);
  int yInt = floor(y);
  double xFract = x - xInt;
  double yFract = y - yInt;
  double v1, v2, v3, v4;
  double w1, w2;

  v1 = smoothNoise(xInt, yInt);
  v2 = smoothNoise(xInt+1, yInt);
  v3 = smoothNoise(xInt, yInt+1);
  v4 = smoothNoise(xInt+1, yInt+1);
  return lerp(lerp(v1, v2, xFract), lerp(v3, v4, xFract), yFract);
}

double ValueNoise2D::noiseAt(double x, double y)
{
  double total = 0.0; 
  double freq = 1.0, amp = 1.0;
  for(int i = 0; i < octarveNum; i++){
    total += interpolate(x*freq, y*freq) * amp;
    freq *= 2.0;
    amp  *= persistence;
  }
  return total;
}

void ValueNoise2D::setRange(int w, int h)
{
  width = w;
  height = h;
}

void ValueNoise2D::generate()
{
  pixelVal.clear();
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      pixelVal.push_back(noiseAt(i/(double)width, j/(double)height));
    }
  }
}
