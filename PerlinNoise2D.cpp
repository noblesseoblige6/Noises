#include "PerlinNoise2D.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <time.h>

using std::vector;
using std::cout;
using std::endl;
PerlinNoise2D::PerlinNoise2D()
{
  width = height = 256;
  octarveNum = 8; 
  persistence = 0.65;
  interpType = LINEAR;
  noises.clear();
  gradients.clear();
}

PerlinNoise2D::~PerlinNoise2D(){}

double dot(vec2& a, vec2& b)
{
  return a.x*b.x + a.y*b.y;
}

double PerlinNoise2D::noise2D(int x, int y)
{
  int n = x + y * 57;
  n = (n<<13)^n;
  int nn = ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff);
  return  1.0 - ((double)nn / 1073741824.0);   
}

double PerlinNoise2D::interpolate(double x, double y)
{
  int xInt = (int)x, yInt = (int)y;
  double fracX = x - (double)xInt, fracY = y - (double)yInt;
  double s, t, u, v;

  vec2 d1(fracX, fracY), d2(fracX-1.0, fracY);
  vec2 d3(fracX, fracY-1.0), d4(fracX-1.0, fracY-1.0);
  vec2 c(sCurve(fracX), sCurve(fracY));

  int grad11 = permutations[(xInt + permutations[yInt & 255]) & 255] & 7;
  int grad12 = permutations[(xInt + 1 + permutations[yInt & 255]) & 255] & 7;
  int grad21 = permutations[(xInt + permutations[(yInt + 1) & 255]) & 255] & 7;
  int grad22 = permutations[(xInt + 1 + permutations[(yInt + 1) & 255]) & 255] & 7;

  s = dot(gradients[grad11], d1);
  t = dot(gradients[grad12], d2);
  u = dot(gradients[grad21], d3);
  v = dot(gradients[grad22], d4);

  //@comment Take average to Normalize the value of dot products
  // s *= 0.5; t *= 0.5; 
  // u *= 0.5; v *= 0.5; 

  //@comment lerp alon x, then lerp along y
  return linearInterpolate(linearInterpolate(s, t, c.x), linearInterpolate(u, v, c.x), c.y);
}

double PerlinNoise2D::gradientAt(int x, int y)
{
  double total = 0.0; 
  double freq = 1.0/height, amp = 1.0;

  for(int i = 0; i < octarveNum; i++){
    total += interpolate((double)x*freq, (double)y*freq)*amp;
    freq *= 2.0;
    amp *= persistence;
  }
  return total;
}

void PerlinNoise2D::setRange(int w, int h)
{
  width = w;
  height = h;
}

void PerlinNoise2D::generate()
{
  //@comment set random vector and normailize it
  for (int i=0; i<8; i++){
    gradients.push_back(vec2(noise(i), noise(i+1)));
    gradients[i] = gradients[i].normalized();
  }
  //set up the random numbers table
  for(int i=0; i<256; i++){
    permutations[i] = (int)(((noise(i)+1.0)/2.0)*255);
  }
  noises.clear();
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      noises.push_back(gradientAt(i, j));
    }
  }
}

void PerlinNoise2D::printData()
{
  for(int i = 0; i < width; i++){
    for(int j = 0; j < height; j++){
      cout<<i<<" "<<j<<" "<<noises[i*width+j]<<endl;
    }
    cout<<endl;
  }
}
