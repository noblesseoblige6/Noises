#include "PerlinNoise2D.h"
#include <iostream>
#include <cmath>

using std::vector;
using std::cout;
using std::endl;

PerlinNoise2D::PerlinNoise2D()
{
  square = 64;
  width = height = 128;
  interpType = LINEAR;
  noises.clear();
}

PerlinNoise2D::~PerlinNoise2D(){}

static double norm(double a, double b)
{
  return sqrt(a*a + b*b);
}

double PerlinNoise2D::noise2D(int x, int y)
{
  int n = x + y * 57;
  n = (n<<13)^n;
  int nn = ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff);
  return  1.0 - ((double)nn / 1073741824.0);   
}

double PerlinNoise2D::gradientAt(int x, int y)
{
  double dx1 = (noise(x)+1)/2, dx2 = 1.0 - dx1;
  double dy1 = (noise(y)+1)/2, dy2 = 1.0 - dy1;
  double  s, t, u, v;
  double sx, sy;

  double gx = noise(x), gy = noise(y); 
  double n = norm(gx, gy);
  s = gx/n*dx1 + gy/n*dy1;

  gx = noise(x+1); 
  n = norm(gx, gy);
  t = gx/n*dx2 + gy/n*dy1;

  gy = noise(y+1); 
  n = norm(gx, gy);
  v = gx/n*dx1 + gy/n*dy2;

  gx = noise(x); 
  n = norm(gx, gy); 
  u = gx/n*dx2 + gy/n*dy2;

  sx = sCurve(dx1);
  sy = sCurve(dy1);
  return interpolate(interpolate(s, t, sx), interpolate(u, v, sx), sy);
}

void PerlinNoise2D::setRange(int w, int h)
{
  width = w;
  height = h;
}

void PerlinNoise2D::generate()
{
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
  }
}
