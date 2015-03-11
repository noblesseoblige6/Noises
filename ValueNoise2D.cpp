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
  interpType = LINEAR;
  noises.clear();
}

ValueNoise2D::~ValueNoise2D(){}

double ValueNoise2D::noise(int x, int y)
{
  int n = x + y * 57;
  n = (n<<13)^n;
  int nn = ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff);
  return  1.0 - ((double)nn / 1073741824.0);   
}


double ValueNoise2D::smoothNoise(int x, int y)
{
  double side, corner, center;
  center = noise(x, y)/4;
  side = (noise(x+1, y)+noise(x-1, y)+noise(x, y+1)+noise(x, y-1))/8;
  corner = (noise(x+1, y+1)+noise(x-1, y+1)+noise(x+1, y-1)+noise(x-1, y-1))/16;
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

  switch(interpType){
    case LINEAR:
      w1 = linearInterpolate(v1, v2, xFract);
      w2 = linearInterpolate(v3, v4, xFract);
      return linearInterpolate(w1, w2, yFract);
      break;
    case COSINE:
      w1 = cosineInterpolate(v1, v2, xFract);
      w2 = cosineInterpolate(v3, v4, xFract);
      return cosineInterpolate(w1, w2, yFract);
      break;
    default:
      break;
  }
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
  noises.clear();
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      noises.push_back(noiseAt(i/(double)width, j/(double)height));
    }
  }
}

void ValueNoise2D::printData()
{
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      cout<<i<<" "<<j<<" "<<noises[i*width+j]<<endl;
    }
  }
}
