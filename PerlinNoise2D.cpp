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
  persistence = 0.625;
  interpType = LINEAR;
  noises.clear();
}

PerlinNoise2D::~PerlinNoise2D(){}

double dot(vec2 a, vec2 b)
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
  double fracX = x - (int)x, fracY = y - (int)y;
  vec2 d1(fracX, fracY), d2(fracX-1, fracY);
  vec2 d3(fracX, fracY-1), d4(fracX-1, fracY-1);

  vec2 g1(noise2D(x, y), noise2D(y, x)), g2(noise2D(x+1, y), noise2D(y, x));
  vec2 g3(noise2D(x, y), noise2D(y+1, x)), g4(noise2D(x+1, y), noise2D(y+1, x));
  vec2 c(sCurve(d1.x), sCurve(d1.y));
  vec2 w; 
  double s, t, u, v;
  //@commnet normalize random vectors
  g1 = g1.normalized(); g2 = g2.normalized();
  g3 = g3.normalized(); g4 = g4.normalized();

  s = dot(g1, d1);
  t = dot(g2, d2);
  u = dot(g3, d3);
  v = dot(g4, d4);

  //@comment Take average to Normalize the value of dot products
  s *= 0.5; t *= 0.5; 
  u *= 0.5; v *= 0.5; 


  switch(interpType){
    case LINEAR:
      w.x = linearInterpolate(s, t, c.x); 
      w.y = linearInterpolate(u, v, c.x);
      return linearInterpolate(w.x, w.y, c.y);
      break;
    case COSINE:
      w.x = cosineInterpolate(s, t, c.x); 
      w.y = cosineInterpolate(u, v, c.x);
      return cosineInterpolate(w.x, w.y, c.y);
      break;
    default:
      break;
  }
}

double PerlinNoise2D::gradientAt(double x, double y){
  double total = 0.0; 
  double freq = 1.0/height, amp = 1.0;
  for(int i = 0; i < octarveNum; i++){
    total += interpolate((x)*freq, (y)*freq)*amp;
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
      cout<<j<<" "<<noises[i*width+j]<<endl;
    }
    cout<<endl;
  }
}
